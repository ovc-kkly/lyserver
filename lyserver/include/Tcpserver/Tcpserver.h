#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <memory>
#include "reactor.h"
#include "noncopyable.h"
#include "config.h"
#include "sig_handle.h"
namespace lyserver
{
    struct TcpServerConf
    {
        typedef std::shared_ptr<TcpServerConf> ptr;

        std::vector<std::string> address;
        int keepalive = 0;
        int timeout = 1000 * 2 * 60;
        int ssl = 0;
        std::string id;
        /// 服务器类型，http, ws, rock
        std::string type = "http";
        std::string name;
        std::string cert_file;
        std::string key_file;
        std::string accept_worker;
        std::string io_worker;
        std::string process_worker;
        std::map<std::string, std::string> args;
        std::string server_Timer;
        std::string client_Timer;

        bool isValid() const
        {
            return !address.empty();
        }

        bool operator==(const TcpServerConf &oth) const
        {
            return address == oth.address && keepalive == oth.keepalive && timeout == oth.timeout && name == oth.name && ssl == oth.ssl && cert_file == oth.cert_file && key_file == oth.key_file && accept_worker == oth.accept_worker && io_worker == oth.io_worker && process_worker == oth.process_worker && args == oth.args && id == oth.id && type == oth.type && server_Timer == oth.server_Timer && client_Timer == oth.client_Timer;
        }
    };

    template <>
    class LexicalCast<std::string, TcpServerConf>
    {
    public:
        TcpServerConf operator()(const std::string &v)
        {
            YAML::Node node = YAML::Load(v);
            TcpServerConf conf;
            conf.id = node["id"].as<std::string>(conf.id);
            conf.type = node["type"].as<std::string>(conf.type);
            conf.keepalive = node["keepalive"].as<int>(conf.keepalive);
            conf.timeout = node["timeout"].as<int>(conf.timeout);
            conf.name = node["name"].as<std::string>(conf.name);
            conf.ssl = node["ssl"].as<int>(conf.ssl);
            conf.cert_file = node["cert_file"].as<std::string>(conf.cert_file);
            conf.key_file = node["key_file"].as<std::string>(conf.key_file);
            conf.accept_worker = node["accept_worker"].as<std::string>();
            conf.io_worker = node["io_worker"].as<std::string>();
            conf.process_worker = node["process_worker"].as<std::string>();
            conf.args = LexicalCast<std::string, std::map<std::string, std::string>>()(node["args"].as<std::string>(""));
            if (node["address"].IsDefined())
            {
                for (size_t i = 0; i < node["address"].size(); ++i)
                {
                    conf.address.push_back(node["address"][i].as<std::string>());
                }
            }

            conf.server_Timer = node["server_Timer"].as<std::string>();
            conf.client_Timer = node["client_Timer"].as<std::string>();
            return conf;
        }
    };

    template <>
    class LexicalCast<TcpServerConf, std::string>
    {
    public:
        std::string operator()(const TcpServerConf &conf)
        {
            YAML::Node node;
            node["id"] = conf.id;
            node["type"] = conf.type;
            node["name"] = conf.name;
            node["keepalive"] = conf.keepalive;
            node["timeout"] = conf.timeout;
            node["ssl"] = conf.ssl;
            node["cert_file"] = conf.cert_file;
            node["key_file"] = conf.key_file;
            node["accept_worker"] = conf.accept_worker;
            node["io_worker"] = conf.io_worker;
            node["process_worker"] = conf.process_worker;
            node["args"] = YAML::Load(LexicalCast<std::map<std::string, std::string>, std::string>()(conf.args));
            for (auto &i : conf.address)
            {
                node["address"].push_back(i);
            }
            node["server_Timer"] = conf.server_Timer;
            node["client_Timer"] = conf.client_Timer;
            std::stringstream ss;
            ss << node;
            return ss.str();
        }
    };

    class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable
    {
    public:
        typedef std::shared_ptr<TcpServer> ptr;
        TcpServer();
        /**
         * @brief 析构函数
         */
        virtual ~TcpServer();

        virtual void init(call_back_ cb=nullptr);
        /**
         * @brief 绑定地址
         * @return 返回是否绑定成功
         */
        virtual bool bind(lyserver::Address::ptr addr, bool ssl = false);
        virtual bool bind(lyserver::Address::ptr addr, bool ssl = false, serverType type = serverType::TCP);
        /**
         * @brief 绑定地址数组
         * @param[in] addrs 需要绑定的地址数组
         * @param[out] fails 绑定失败的地址
         * @return 是否绑定成功
         */
        virtual bool bind(const std::map<serverType, Address::ptr> &addrs, std::map<serverType, Address::ptr> &fails, bool ssl = false);
        /**
         * @brief 启动服务
         * @pre 需要bind成功后执行
         */
        virtual bool start();

        /**
         * @brief 停止服务
         */
        virtual void stop();

        /**
         * @brief 返回服务器名称
         */
        std::string getName() const { return m_name; }
        /**
         * @brief 返回读取超时时间(毫秒)
         */
        uint64_t getRecvTimeout() const { return m_recvTimeout; }
        /**
         * @brief 设置读取超时时间(毫秒)
         */
        void setRecvTimeout(uint64_t v) { m_recvTimeout = v; }
        /**
         * @brief 设置服务器名称
         */
        virtual void setName(const std::string &v) { m_name = v; }

        TcpServerConf::ptr getConf() const { return m_conf; }
        void setConf(TcpServerConf::ptr v) { m_conf = v; }
        void setConf(const TcpServerConf &v);

        /**
         * @brief 是否停止
         */
        bool isStop() const { return m_isStop; }

    protected:
        void LoadConfig(const std::string &path = "/home/ly/lyserver_master/configuration/yaml/Listen.yaml");
        /**
         * @brief 处理新连接的Socket类
         */
        virtual int handleClient(Socket::ptr client);

        /**
         * @brief 开始接受连接
         */
        virtual void startAccept();

    protected:
        /// 监听Socket数组
        std::vector<Socket::ptr> m_socks;

        MainReactor::ptr main_reactor;
        SubReactor::ptr sub_reactor;
        Acceptor::ptr acceptr;
        Connection::ptr connptr;
        SigHandle::ptr m_sigHandle;

        ThreadPool::ptr pool;
        /// 服务器名称
        std::string m_name;
        /// 服务器类型
        std::string m_type = "tcp";
        /// 服务是否停止
        bool m_isStop;
        /// 接收超时时间(毫秒)
        uint64_t m_recvTimeout;

        std::string server_Timer;
        std::string client_Timer;

        bool m_ssl = false;
        TcpServerConf::ptr m_conf;
    };
}
#endif