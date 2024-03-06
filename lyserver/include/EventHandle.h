#ifndef EVENTHANDLE_H
#define EVENTHANDLE_H
#include <memory>
#include <functional>
#include <unordered_map>
#include <stdlib.h>
#include <sys/epoll.h>
#include <json/json.h>
#include "EventType.h"
#include "My_events.h"
#include "user_db_handle.h"
#include "r_s_Msg.h"


#include "fdmanager.h"
#include "epollmanager.h"
#include "fcntl.h"
#include "ftp.h"
#include "myutil.h"
#include "AStar.h"
// #include "RF.h"
#include "personalization.h"
using namespace Random_Forest;
namespace lyserver
{
    // extern My_events ;
    class EventManager;
    using Event_Callback = std::function<int(const Json::Value &, My_events *)>;
    class EventHandle : public Db_handle
    {
        friend class EventManager;
    public:
        // using Db_handle::Db_handle;
        typedef std::shared_ptr<EventHandle> ptr;
        EventHandle();
        ~EventHandle();
        static string image;
        size_t length;

        /**
         * @brief 注册事件回调函数
         *
         */
        void register_Event_callback();
        /**
         * @brief 给客户端响应消息
         *
         * @param ev 事件类型
         * @param myev 客户端文件描述符所对应的类对象
         * @return int
         */
        int respond_client(EventType &ev, My_events* myev);
        int respond_client_(EventType &ev, My_events* myev);
        /**
         * @brief
         *
         * @param obj
         * @param myev
         * @return int
         */
        int ID_CallBack(const Json::Value &obj, My_events* myev);
        /**
         * @brief 登录回调函数
         *
         * @param obj 客户端发来的消息，是json格式
         * @param myev 客户端文件描述符所对应的类对象
         * @return int
         */
        int Log_on_CallBack(const Json::Value &obj, My_events* myev);
        /**
         * @brief 注册回调函数
         *
         * @param obj 客户端发来的消息，是json格式
         * @param myev 客户端文件描述符所对应的类对象
         * @return int
         */
        int Sign_up_CallBack(const Json::Value &obj, My_events* myev);

        int Transmit_CallBack(const Json::Value &obj, My_events* myev);
        /**
         * @brief 注销回调函数
         *
         * @param obj 客户端发来的消息，是json格式
         * @param myev 客户端文件描述符所对应的类对象
         * @return int
         */
        int Close_account_CallBack(const Json::Value &obj, My_events* myev);
        /**
         * @brief Set the client ID object
         *
         * @param myev
         * @param er_ptr
         * @param sp
         * @param buf
         * @return int
         */
        int set_client_ID(const Json::Value &obj, My_events* myev);

        /**
         * @brief 退出登录
         *
         * @param obj
         * @param myev
         * @param er_ptr
         * @return int
         */
        int Log_out_CallBack(const Json::Value &obj, My_events* myev);

        int RF_execute_CallBack(const Json::Value &obj, My_events* myev);
        /**
         * @brief
         *
         */
        void notify_u(const string &userID);
        int recognize_CallBack(const Json::Value &obj, My_events* myev);
        int imageLength_CallBack(const Json::Value &obj, My_events* myev);
        /**
         * @brief ftp的回调函数
         * 
         * @param obj 
         * @param myev 
         * @return int 
         */
        int ftp_CallBack(const Json::Value &obj, My_events* myev);
        /**
         * @brief 根据need从buff获取字符串
         * 
         * @param buff 
         * @param need 
         * @return string 
         */
        string get_info(const string &buff, const string &need);
        /**
         * @brief 发送消息
         * 
         * @param myev 
         * @param fdmagr 
         * @return int 
         */
        int send_function(My_events *myev, Fdmanager::ptr fdmagr);
        bool sendjson(int fd, string &buff);
        bool send_str(int fd, string &buff);

        void dete(my_ocr::ocr_ptr &ptr, string &image, const int &fd);
        std::unordered_map<EventType, Event_Callback> callback_map;
        int detection_data_CallBack(const Json::Value &obj, My_events* myev);

        void run_RF();
        /**
         * @brief 接受客户端发来的用户数据，制作数据集
         * 
         * @param obj 
         * @param myev 
         * @return int 
         */
        int recv_dataset_one(const Json::Value &obj, My_events* myev);
        /**
         * @brief 返回请求
         * 
         * @param obj 
         * @param myev 
         * @return int 
         */
        int return_CallBack(const Json::Value &obj, My_events* myev);
    private:
        /* data */
        // user_db_handle *db_handle;
        static EventHandle* evHandle;
        user_db_handle *user_;
        Fdmanager::ptr fdmagr;
        Epollmanager::ptr epollmagr;
        FTP::ptr ftp;
        RF::ptr rf;

        PersonaLization::ptr pl;
    };
    // 定义一个删除器，释放查询结果
    struct Deleter_resource
    {
        void operator()(user_db_handle *ptr) const
        {
            // 不执行任何操作
            ptr->delete_var();
        }
    };
    class EventManager
    {
    public:
        EventHandle::ptr getEvmgr_ptr()
        {
            // 获取单例实例的全局访问点
            // 使用双检锁（Double-Checked Locking）确保线程安全
            if (EventHandle::evHandle == nullptr)
            {
                RWMutex::WriteLock lock(mutex);
                if (EventHandle::evHandle == nullptr)
                {
                    EventHandle::evHandle = new EventHandle;
                }
            }
            return EventHandle::ptr(EventHandle::evHandle);
        }

    private:
        RWMutex mutex;
    };
    typedef Singleton<EventManager> EvMgr;
}
#endif
