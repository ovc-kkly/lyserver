#include "Tcpserver.h"

namespace lyserver
{
    static lyserver::ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout = lyserver::Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2), "tcp server read timeout");

    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    TcpServer::TcpServer() : m_name("lyserver/1.0.0"), m_isStop(true), m_recvTimeout(g_tcp_server_read_timeout->getValue()), server_Timer("Yes"), client_Timer("No")
    {
        // LoadConfig();

        main_reactor.reset(new MainReactor(server_Timer));
        sub_reactor.reset(new SubReactor);
        connptr.reset(new Connection(sub_reactor));
        acceptr.reset(new Acceptor(main_reactor, sub_reactor, connptr, client_Timer));
        pool.reset(new ThreadPool(4, 10));
        m_sigHandle.reset(new SigHandle);
    }
    TcpServer::~TcpServer()
    {
    }
    void TcpServer::LoadConfig(const std::string &path)
    {
        YAML::Node ListenYaml = YAML::LoadFile(path);
        lyserver::Config::LoadFromYaml(ListenYaml); // 把配置参数加载进了静态变量
        
    }
    void TcpServer::init(call_back_ cb)
    {
        if(m_type == "tcp"){
            
        }
        else if(m_type == "http"){
            connptr->setHttp_handle_cb(cb);
        }
        main_reactor->init(pool);
        m_sigHandle->SetSignalHandler(SIGINT, handleSignal);
    }
    bool TcpServer::bind(lyserver::Address::ptr addr, bool ssl, serverType type)
    {
        std::map<serverType, Address::ptr> addrs;
        std::map<serverType, Address::ptr> fails;
        // acceptr->addAddress(addr);
        // acceptr->start(type);
        addrs.insert(std::pair<serverType, Address::ptr>(type, addr));
        return bind(addrs, fails, ssl);
        return true;
    }
    bool TcpServer::bind(lyserver::Address::ptr addr, bool ssl)
    {
        // std::vector<Address::ptr> addrs;
        // std::vector<Address::ptr> fails;
        // addrs.push_back(addr);
        // return bind(addrs, fails, ssl);
        return true;
    }
    bool TcpServer::bind(const std::map<serverType, Address::ptr> &addrs, std::map<serverType, Address::ptr> &fails, bool ssl)
    {
        for (auto &addr : addrs)
        {
            acceptr->addAddress(addr.second);
            if (!acceptr->start(addr.first, m_recvTimeout))
            {
                LY_LOG_ERROR(g_logger) << "bind fail errno="
                                       << errno << " errstr=" << strerror(errno)
                                       << " addr=[" << addr.second->toString() << "]";
                fails.insert(pair<serverType, Address::ptr>(addr.first, addr.second));
                continue;
            }
            // m_socks.push_back();
        }
        if (!fails.empty())
        {
            return false;
        }
        return true;
    }

    /**
     * @brief 启动服务
     * @pre 需要bind成功后执行
     */
    bool TcpServer::start()
    {
        if (!m_isStop)
        {
            return true;
        }
        m_isStop = false;

        // CALL_BACK startaccept = My_events::make_callback(&TcpServer::startAccept, this);
        // pool->addTask(Task(startaccept));
        startAccept();
        return true;
    }

    /**
     * @brief 停止服务
     */
    void TcpServer::stop()
    {
        m_isStop = true;
        auto self = shared_from_this();
        this->acceptr->stop();
    }

    int TcpServer::handleClient(Socket::ptr client)
    {
        return 0;
    }
    void TcpServer::startAccept()
    {
        while (!m_isStop)
        {
            CALL_BACK subfun = My_events::make_callback(&SubReactor::subEventloop, sub_reactor.get(), pool);
            pool->addTask(Task(subfun));
            CALL_BACK mainfun = My_events::make_callback(&MainReactor::Eventloop, main_reactor.get(), pool);
            // pool->addTask(Task(mainfun));
            mainfun();
        }
    }

    void TcpServer::setConf(const TcpServerConf &v)
    {
        m_conf.reset(new TcpServerConf(v));
    }

}