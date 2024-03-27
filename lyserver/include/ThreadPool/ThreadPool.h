#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <iostream>
#include <queue>
#include <pthread.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <memory>
#include <functional>
#include <tuple>
#include "thread.h"
#include "log.h"
using namespace std;

namespace lyserver
{
    
    // using callback = void (*)(void *arg); // 定义一个函数指针类型,等价于typedef void (*callback)(void* arg)；
    using callback = std::function<void()>;
    struct Task
    {
        /*任务结构体*/

    public:
        Task()
        {
            function = nullptr;
            // arg = nullptr;
        }
        Task(callback f)
        {
            function = f;
        }
        callback function; // 任务结构体的一个包装器（包装函数），完成需要的任务
    };
    class TaskQueue
    {
        /*封装了一个任务队列类*/
    public:
        TaskQueue();
        ~TaskQueue();
        // 添加任务
        void addTask(Task task);
        void addTask(callback func);
        // 取出一个任务
        Task takeTask();
        // 获取当前任务的个数
        inline size_t taskNumber()
        {
            return m_queue.size();
        }

    private:
        RWMutex m_rwmutex; // 读写锁
        std::queue<Task> m_queue; // 任务队列容器,队列里面的每一个元素都是Task类型的
    };
    class ThreadPool
    {
    public:
        typedef std::shared_ptr<ThreadPool> ptr;
        ThreadPool(int min, int max);
        ~ThreadPool();

        // 给线程池添加任务
        void addTask(Task task);
        void addThread(std::function<void()>& cb, const string& name);
        bool isThreadempty(){return m_threads.empty();}
        int getThreadID(int i){return m_threads[i]->getId();}
        // void addTask(Task<T><SockInfo> task);
        //  获取忙线程的个数
        int getBusyNumber();
        // 获取活着的线程个数
        int getAliveNumber();

    private:
        // 工作的线程的任务函数
        static void worker(void *arg); // 静态成员函数只能访问静态变量,不能访问普通成员变量
        // 管理者线程的任务函数
        static void manager(void *arg);
        void threadExit();

    private:
        TaskQueue *m_taskQ;        // 任务队列
        // pthread_mutex_t m_lock;    // 锁整个线程池的一个锁
        Mutex m_lock;
        pthread_cond_t m_notEmpty; // 条件变量
        std::vector<Thread::ptr> m_threads; // 工作线程
        // pthread_t *m_threadIDs;    // 工作的线程ID
        // pthread_t m_managerID;     // 管理者线程ID
        Thread* m_manager;

        int m_minNum;            // 最小线程数
        int m_maxNum;            // 最大线程数
        int m_busyNum;           // 忙的线程个数
        int m_aliveNum;          // 存活的线程个数
        int m_exitNum;           // 要销毁的线程个数
        bool m_shutdown = false; // 是否要销毁线程池，销毁1，不销毁0
        static const int NUMBER = 2;
    };

}
#endif
