#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"

namespace lyserver {

static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;//其实是为每个线程创建线程本地数据(也叫线程局部数据)，线程局部数据专属于线程。在线程内是共享，线程间独立。本质上，就相当于线程域的全局静态变量。
static thread_local Fiber* t_scheduler_fiber = nullptr;//主协程

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name):m_name(name) {
    LY_ASSERT(threads > 0);

    if(use_caller) {
        lyserver::Fiber::GetThis();//分配一个主协程
        --threads;

        LY_ASSERT(GetThis() == nullptr);
        t_scheduler = this;//设置协程调度器

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));//创建协程调度器的一个主协程，指定了它的执行函数
        lyserver::Thread::SetName(m_name);
        
        t_scheduler_fiber = m_rootFiber.get();
        m_rootThread = lyserver::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    }
    m_threadCount = threads;//初始化线程数量
}

Scheduler::~Scheduler() {
    LY_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);
    if(!m_stopping) {
        return;
    }
    m_stopping = false;
    LY_ASSERT(m_threads.empty());

    m_threads.resize(m_threadCount);
    //创建线程，并添加到线程池和ID数组中
    for(size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }
    lock.unlock();//要解锁

    // if(m_rootFiber) {
    // //    m_rootFiber->swapIn();
    //    m_rootFiber->call();
    //    LY_LOG_INFO(g_logger) << "call out " << m_rootFiber->getState();
    // }
}

void Scheduler::stop() {
    m_autoStop = true;
    if(m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT)) {
        LY_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if(stopping()) {
            return;
        }
    }

    //bool exit_on_this_fiber = false;
    if(m_rootThread != -1) {
        LY_ASSERT(GetThis() == this);
    } else {
        LY_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();//唤醒动作，唤醒线程
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {

        if(!stopping()) {
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
    //if(exit_on_this_fiber) {
    //}
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    LY_LOG_DEBUG(g_logger) << m_name << " run";
    set_hook_enable(true);
    setThis();
    if(lyserver::GetThreadId() != m_rootThread) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while(true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            MutexType::Lock lock(m_mutex);//加锁，防止多线程竞争
            auto it = m_fibers.begin();
            while(it != m_fibers.end()) {
                if(it->thread != -1 && it->thread != lyserver::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                LY_ASSERT(it->fiber || it->cb);
                if(it->fiber && it->fiber->getState() == Fiber::EXEC) {//这个协程正在处理，继续找下一个协程
                    ++it;
                    continue;
                }

                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }

        if(tickle_me) {//通知其他协程
            tickle();
        }

        if(ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)) {
            ft.fiber->swapIn();
            --m_activeThreadCount;
            //执行完后，判断一下状态
            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        } else if(ft.cb) {//是函数
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);//用ft的cb来初始化cb_fiber协程
                // LY_LOG_INFO(g_logger)<< "--"<<cb_fiber->getId();
            } else {
                cb_fiber.reset(new Fiber(ft.cb));//否则重新创建
                // LY_LOG_INFO(g_logger)<< "--||"<<cb_fiber->getId();
            }
            ft.reset();//用完了就reset重置数据
            cb_fiber->swapIn();//切换协程来执行
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEPT
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);//协程执行完任务，置空
                // LY_LOG_INFO(g_logger)<< "--=="<<cb_fiber->getId();
            } else {//if(cb_fiber->getState() != Fiber::TERM) {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {//这里表示没任务了，直接去执行idle协程
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                LY_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}

void Scheduler::tickle() {
    LY_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping
        && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    LY_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        lyserver::Fiber::YieldToHold();
    }
}

void Scheduler::switchTo(int thread) {
    LY_ASSERT(Scheduler::GetThis() != nullptr);
    if(Scheduler::GetThis() == this) {
        if(thread == -1 || thread == lyserver::GetThreadId()) {
            return;
        }
    }
    schedule(Fiber::GetThis(), thread);
    Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleThreadCount
       << " stopping=" << m_stopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
    m_caller = Scheduler::GetThis();
    if(target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher() {
    if(m_caller) {
        m_caller->switchTo();
    }
}

}
