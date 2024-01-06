#include "ThreadPool.h"

namespace lyserver
{

    TaskQueue::TaskQueue()
    {
        pthread_mutex_init(&m_mutex, NULL); // 初始化互斥锁
    }
    TaskQueue::~TaskQueue()
    {
        pthread_mutex_destroy(&m_mutex); // 释放互斥锁资源
    }
    // 添加任务

    void TaskQueue::addTask(Task task)
    {
        pthread_mutex_lock(&m_mutex);
        m_queue.push(task);
        pthread_mutex_unlock(&m_mutex);
    }
    void TaskQueue::addTask(callback func)
    {
        pthread_mutex_lock(&m_mutex);
        m_queue.push(Task(func));
        pthread_mutex_unlock(&m_mutex);
    }
    // 取出一个任务
    Task TaskQueue::takeTask()
    {
        Task t;
        pthread_mutex_lock(&m_mutex);
        if (!m_queue.empty())
        {
            t = m_queue.front(); // 取出第一个任务
            m_queue.pop();       // 删除第一个元素
        }
        pthread_mutex_unlock(&m_mutex);
        return t;
    }

    ThreadPool::ThreadPool(int min, int max)
    {

        do
        {
            // 实例化任务队列
            m_taskQ = new TaskQueue;
            if (m_taskQ == nullptr)
            {
                cout << "new m_taskQ fail..." << endl;
                break;
            }
            // 初始化线程池
            m_minNum = min;
            m_maxNum = max;
            m_busyNum = 0;
            m_aliveNum = min;
            m_exitNum = 0;

            m_threadIDs = new pthread_t[max]; // 实例化一个数组(每个元素都是pthread_t类型的，有max个元素)线程ID,返回一个指向数组首元素的一个指针
            if (m_threadIDs == nullptr)
            {
                cout << "new threadIDs fail..." << endl;
                break;
            }
            memset(m_threadIDs, 0, sizeof(pthread_t) * max); // 初始化线程ID的数组元素

            // 初始化互斥锁,条件变量
            if (pthread_mutex_init(&m_lock, NULL) != 0 ||
                pthread_cond_init(&m_notEmpty, NULL) != 0)
            {
                cout << "init mutex or condition fail..." << endl;
                break;
            }
            m_shutdown = false;
            /////////////////// 创建线程 //////////////////
            // 创建管理者线程, 1个
            pthread_create(&m_managerID, NULL, manager, this);
            // 根据最小线程个数, 创建线程
            for (int i = 0; i < min; ++i)
            {
                pthread_create(&m_threadIDs[i], NULL, worker, this);
                cout << "创建子线程, ID: " << to_string(m_threadIDs[i]) << endl;
            }
        } while (0);
    }

    ThreadPool::~ThreadPool()
    {
        m_shutdown = true;
        // 销毁管理者线程
        pthread_join(m_managerID, NULL);
        // 唤醒所有消费者线程
        for (int i = 0; i < m_aliveNum; ++i)
        {
            pthread_cond_signal(&m_notEmpty);
        }

        if (m_taskQ)
            delete m_taskQ;
        if (m_threadIDs)
            delete[] m_threadIDs;
        pthread_mutex_destroy(&m_lock);
        pthread_cond_destroy(&m_notEmpty);
    }

    void *ThreadPool::worker(void *arg)
    {
        ThreadPool *pool = static_cast<ThreadPool *>(arg); // 把万能参数进行强制类型转换，传进来的参数arg是ThreadPool的指针对象

        // 一直不停的工作
        while (true)
        {
            // 访问任务队列(共享资源)加锁
            pthread_mutex_lock(&pool->m_lock);
            // 判断任务队列是否为空, 如果为空工作线程阻塞
            while (pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown)
            {
                //  阻塞线程,被唤醒的地方只有两个，一个是有新任务来了，另一个是要销毁多余的线程
                pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock); // 先阻塞，解锁，当被唤醒时，解除阻塞并重新上锁。

                // 解除阻塞之后, 判断是否要销毁线程
                if (pool->m_exitNum > 0)
                {
                    pool->m_exitNum--;
                    if (pool->m_aliveNum > pool->m_minNum)
                    {
                        pool->m_aliveNum--;
                        pthread_mutex_unlock(&pool->m_lock);
                        pool->threadExit();
                    }
                }
            }
            // 判断线程池是否被关闭了
            if (pool->m_shutdown)
            {
                pthread_mutex_unlock(&pool->m_lock);
                pool->threadExit();
            }
            { // 准备执行任务了
              // 从任务队列中取出一个任务
                Task task = pool->m_taskQ->takeTask();
                // 工作的线程+1
                pool->m_busyNum++;
                // 线程池解锁
                pthread_mutex_unlock(&pool->m_lock);
                // 执行任务
                task.function();

                // if (task.arg != nullptr)
                // {
                //     delete task.arg;
                //     task.arg = nullptr;
                // }

                // 任务处理结束
                // cout << "thread " << to_string(pthread_self()) << " end working...";
                pthread_mutex_lock(&pool->m_lock);

                pool->m_busyNum--;
                pthread_mutex_unlock(&pool->m_lock);
            }
        }

        return nullptr;
    }

    // 管理者线程任务函数
    void *ThreadPool::manager(void *arg)
    {
        ThreadPool *pool = static_cast<ThreadPool *>(arg);
        // 如果线程池没有关闭, 就一直检测
        while (!pool->m_shutdown)
        {
            // 每隔1s检测一次
            sleep(1);
            // 取出线程池中的任务数和线程数量
            //  取出工作的线程池数量
            pthread_mutex_lock(&pool->m_lock);
            int queueSize = pool->m_taskQ->taskNumber();
            int liveNum = pool->m_aliveNum;
            int busyNum = pool->m_busyNum;
            pthread_mutex_unlock(&pool->m_lock);

            // 创建线程

            // 当前任务个数>(存活的线程数-忙的线程数)即空闲的线程数 && 存活的线程数<最大线程个数
            if (queueSize > (liveNum - busyNum) && liveNum < pool->m_maxNum)
            {
                // 线程池加锁
                pthread_mutex_lock(&pool->m_lock);
                int num = 0;
                // 直接创建两个线程，NUMBER=2
                for (int i = 0; i < pool->m_maxNum && num < NUMBER && pool->m_aliveNum < pool->m_maxNum; ++i)
                {
                    if (pool->m_threadIDs[i] == 0) // 从线程数组中找到没有用的元素
                    {
                        pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                        num++;
                        pool->m_aliveNum++;
                    }
                }
                pthread_mutex_unlock(&pool->m_lock);
            }

            // 销毁多余的线程
            // 忙线程+3 < 存活的线程数目 && 存活的线程数 > 最小线程数量
            if (busyNum + 3 < liveNum && liveNum > pool->m_minNum)
            {
                pthread_mutex_lock(&pool->m_lock);
                pool->m_exitNum = NUMBER; // 销毁线程是直接设置成员变量为NUMBER=2,就是先销毁2个线程
                pthread_mutex_unlock(&pool->m_lock);
                for (int i = 0; i < NUMBER; ++i)
                {
                    pthread_cond_signal(&pool->m_notEmpty); // 通知阻塞的线程进行自杀
                }
            }
        }
        return nullptr;
    }

    void ThreadPool::addTask(Task task)
    {
        if (m_shutdown)
        {
            return;
        }
        // 添加任务，不需要加锁，任务队列中有锁
        m_taskQ->addTask(task);
        // 唤醒工作的线程
        pthread_cond_signal(&m_notEmpty);
    }

    int ThreadPool::getAliveNumber()
    {
        pthread_mutex_lock(&m_lock);
        int threadNum = m_aliveNum;
        pthread_mutex_unlock(&m_lock);
        return threadNum;
    }

    int ThreadPool::getBusyNumber()
    {
        pthread_mutex_lock(&m_lock);
        int busyNum = m_busyNum;
        pthread_mutex_unlock(&m_lock);
        return busyNum;
    }

    // 线程退出

    void ThreadPool::threadExit()
    {
        pthread_t tid = pthread_self();
        for (int i = 0; i < m_maxNum; ++i)
        {
            if (m_threadIDs[i] == tid)
            {
                m_threadIDs[i] = 0;
                cout << "threadExit() function: thread "
                     << to_string(tid) << " exiting..." << endl;
                break;
            }
        }
        pthread_exit(NULL);
    }
}