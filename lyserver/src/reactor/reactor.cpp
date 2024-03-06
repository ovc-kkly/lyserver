#include "reactor/reactor.h"
namespace lyserver
{
    static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");
    Reactor::Reactor()
    {
        fdmagr = FdMgr::GetInstance()->getFdmgr_ptr(); // 单例模式
        EventH = EvMgr::GetInstance()->getEvmgr_ptr();
    }

    Acceptor::Acceptor(MainReactor::ptr reactorm, SubReactor::ptr reactors, Connection::ptr conn, std::string client_Timer) : reactor_m(reactorm), reactor_s(reactors), conn_(conn), client_Timer(client_Timer)
    {
        // 创建监听 socket 等初始化操作
    }
    void Acceptor::addAddress(Address::ptr addr)
    {
        this->addr = addr;
    }
    void Acceptor::start()
    {
        // 启动 Acceptor，开始监听
        // initlistensocket(addr);
        // 进行监听等操作
    }
    bool Acceptor::start(serverType type, uint64_t m_recvTimeout)
    {
        switch (type)
        {
        case serverType::TCP:
            return initlistensocket(addr, m_recvTimeout);
        case serverType::FTP:
            return init_FTP_listen(addr, m_recvTimeout);
        case serverType::FTPDATA:
            return inti_file_listen(addr, m_recvTimeout);
        case serverType::UDP:
            return init_UDP_listen(addr, m_recvTimeout);
        case serverType::HTTP:
            return init_http_listen(addr, m_recvTimeout);
        default:
            break;
        }
        return false;
    }
    bool Acceptor::stop()
    {
        // this->
        return true;
    }
    void Acceptor::handleEvent(My_events *myev, Fdmanager::ptr fdmagr, int listen_type, uint64_t m_recvTimeout)
    {
        if (listen_type == 2)
        {
            filedata_acceptconnect(myev, fdmagr);
        }
        else if (listen_type == 0 || listen_type == 1)
        {
            acceptconnect(myev, fdmagr, listen_type, m_recvTimeout);
        }
        else if (listen_type == 3)
        {
            httpaccept(myev, fdmagr, m_recvTimeout);
        }
        else if (listen_type == 4)
        {
            UDP_recv(myev, fdmagr, m_recvTimeout);
        }
    }
    bool Acceptor::initlistensocket(Address::ptr addr, uint64_t m_recvTimeout)
    {
        // Address::ptr addr = lyserver::IPAddress::Create(address, port); // 创建IP地址基类

        m_socket = lyserver::Socket::CreateTCP(addr);
        if (!m_socket->bind(addr))
        {
            return false;
        }

        m_socket->sockfcntl(m_socket->getSocket(), F_SETFL, O_NONBLOCK);
        m_socket->listen();
        LY_LOG_INFO(g_logger) << " Server Running:port [" << dynamic_cast<IPAddress *>(addr.get())->getPort() << "] ";

        // 从存储结构中找出一个位置返回
        My_events *myev_lfd = reactor_m->fdmagr->lookup(m_socket->getSocket());

        const int *epfd_x = reactor_m->fdmagr->find_epfd(main_re, reactor_m->epollmgr); // 找到一个有位置的epfd
        CALL_BACK acfun = My_events::make_callback(&Acceptor::handleEvent, this, myev_lfd, reactor_m->fdmagr, 0, m_recvTimeout);
        if (myev_lfd->Myev_set(m_socket, acfun, *epfd_x) == -1)
        { // 初始化监听文件描述符所对应的类对象里面的文件描述符和回调函数
            return false;
        }
        myev_lfd->type = 0;
        myev_lfd->epfd_type = reactor_type::main_re;

        // 监听事件上树
        reactor_m->fdmagr->add_epfd_event(*epfd_x, main_re);
        // 注册事件
        reactor_m->registerHandler(myev_lfd, EPOLLIN | EPOLLET);
        return true;
    }
    bool Acceptor::init_UDP_listen(Address::ptr addr, uint64_t m_recvTimeout)
    {
        // 创建UDP监听
        m_socket_udp = Socket::CreateUDP(addr);
        if (!m_socket_udp->bind(addr))
        {
            return false;
        }
        LY_LOG_INFO(g_logger) << "UDP_Server Running:port [" << dynamic_cast<IPAddress *>(addr.get())->getPort() << "]";

        My_events *myev_udp = reactor_m->fdmagr->lookup(m_socket_udp->getSocket());
        const int *epfd_u = reactor_m->fdmagr->find_epfd(main_re, reactor_m->epollmgr); // 找到一个有位置的epfd

        CALL_BACK udp_fun = My_events::make_callback(&Acceptor::handleEvent, this, myev_udp, reactor_m->fdmagr, 4, m_recvTimeout);
        if (myev_udp->Myev_set(m_socket_udp, udp_fun, *epfd_u) == -1)
        { // 初始化监听文件描述符所对应的类对象里面的文件描述符和回调函数
            return false;
        }
        myev_udp->type = 3;
        myev_udp->epfd_type = reactor_type::main_re;

        reactor_m->fdmagr->add_epfd_event(*epfd_u, main_re);
        reactor_m->registerHandler(myev_udp, EPOLLIN);
        return true;
    }
    bool Acceptor::init_FTP_listen(Address::ptr addr, uint64_t m_recvTimeout)
    {
        // Address::ptr addr = lyserver::IPAddress::Create(address, port); // 创建IP地址基类
        m_socket_FTP = lyserver::Socket::CreateTCP(addr);
        if (!m_socket_FTP->bind(addr))
        {
            return false;
        }
        m_socket_FTP->sockfcntl(m_socket_FTP->getSocket(), F_SETFL, O_NONBLOCK);
        m_socket_FTP->listen();
        LY_LOG_INFO(g_logger) << "FTP_Server Running:port [" << dynamic_cast<IPAddress *>(addr.get())->getPort() << "]";

        // lfd 初始化
        My_events *myev_lfd = reactor_m->fdmagr->lookup(m_socket_FTP->getSocket());
        const int *epfd_x = reactor_m->fdmagr->find_epfd(main_re, reactor_m->epollmgr); // 找到一个有位置的epfd
        CALL_BACK acfun = My_events::make_callback(&Acceptor::handleEvent, this, myev_lfd, reactor_m->fdmagr, 1, m_recvTimeout);
        if (myev_lfd->Myev_set(m_socket_FTP, acfun, *epfd_x) == -1)
        {
            return false;
        } // 初始化监听文件描述符所对应的类对象里面的文件描述符和回调函数
        myev_lfd->type = 0;
        myev_lfd->epfd_type = reactor_type::main_re;

        // 监听事件上树
        reactor_m->fdmagr->add_epfd_event(*epfd_x, main_re);
        // epollmgr->eventadd(EPOLLIN | EPOLLET, myev_lfd, *epfd_x);
        reactor_m->registerHandler(myev_lfd, EPOLLIN | EPOLLET);
        return true;
    }
    bool Acceptor::inti_file_listen(Address::ptr addr, uint64_t m_recvTimeout)
    {
        // Address::ptr addr = lyserver::IPAddress::Create(address, port); // 创建IP地址基类
        m_socket_FTPDATA = lyserver::Socket::CreateTCP(addr);
        if (!m_socket_FTPDATA->bind(addr))
        {
            return false;
        }
        m_socket_FTPDATA->sockfcntl(m_socket_FTPDATA->getSocket(), F_SETFL, O_NONBLOCK);
        m_socket_FTPDATA->listen();
        LY_LOG_INFO(g_logger) << "FTP_Server data Running:port [" << dynamic_cast<IPAddress *>(addr.get())->getPort() << "]";

        // lfd 初始化
        My_events *myev_lfd = reactor_m->fdmagr->lookup(m_socket_FTPDATA->getSocket());
        const int *epfd_x = reactor_m->fdmagr->find_epfd(main_re, reactor_m->epollmgr); // 找到一个有位置的epfd
        CALL_BACK acfun = My_events::make_callback(&Acceptor::handleEvent, this, myev_lfd, reactor_m->fdmagr, 2, m_recvTimeout);
        if (myev_lfd->Myev_set(m_socket_FTPDATA, acfun, *epfd_x) == -1)
        { // 初始化监听文件描述符所对应的类对象里面的文件描述符和回调函数
            return false;
        }
        myev_lfd->type = 0;
        myev_lfd->epfd_type = reactor_type::main_re;

        // 监听事件上树
        reactor_m->fdmagr->add_epfd_event(*epfd_x, main_re);
        // epollmgr->eventadd(EPOLLIN | EPOLLET, myev_lfd, *epfd_x);
        reactor_m->registerHandler(myev_lfd, EPOLLIN | EPOLLET);
        return true;
    }
    bool Acceptor::init_http_listen(Address::ptr addr, uint64_t m_recvTimeout)
    {
        m_socket_http = Socket::CreateTCP(addr);
        if (!m_socket_http->bind(addr))
        {
            return false;
        }
        m_socket_http->sockfcntl(m_socket_http->getSocket(), F_SETFL, O_NONBLOCK);
        m_socket_http->listen();
        LY_LOG_INFO(g_logger) << " HttpServer Running:port [" << dynamic_cast<IPAddress *>(addr.get())->getPort() << "] ";

        // 从存储结构中找出一个位置返回
        My_events *myev_http_lfd = reactor_m->fdmagr->lookup(m_socket_http->getSocket());
        const int *epfd_x = reactor_m->fdmagr->find_epfd(main_re, reactor_m->epollmgr); // 找到一个有位置的epfd

        CALL_BACK acfun = My_events::make_callback(&Acceptor::handleEvent, this, myev_http_lfd, reactor_m->fdmagr, 3, m_recvTimeout);
        if (myev_http_lfd->Myev_set(m_socket_http, acfun, *epfd_x) == -1)
        { // 初始化监听文件描述符所对应的类对象里面的文件描述符和回调函数
            return false;
        }
        myev_http_lfd->type = 0;
        myev_http_lfd->epfd_type = reactor_type::main_re;

        reactor_m->fdmagr->add_epfd_event(*epfd_x, main_re);
        // 监听事件上树
        // 注册事件
        reactor_m->registerHandler(myev_http_lfd, EPOLLIN | EPOLLET);
        return true;
    }
    void Acceptor::UDP_recv(My_events *myev, Fdmanager::ptr fdmagr, uint64_t m_recvTimeout)
    {
        // 接收数据
        char buffer[50000];
        memset(buffer, 0, sizeof(buffer));

        lyserver::Address::ptr from(new lyserver::IPv4Address);
        int len = myev->sockptr->recvFrom(buffer, sizeof(buffer), from);
        if (len > 0)
        {
            buffer[len] = '\0';
            LY_LOG_INFO(g_logger) << "recv: " << buffer << " from: " << *from;
            // len = myev->sockptr->sendTo(buff, len, from);
            // if (len < 0)
            // {
            //     LY_LOG_INFO(g_logger) << "send: " << buff << " to: " << *from
            //                              << " error=" << len;
            // }
        }

        // 在这里处理接收到的数据
    }
    void Acceptor::filedata_acceptconnect(My_events *myev, Fdmanager::ptr fdmagr)
    {
        while (1)
        {

            // cfd = accept(myev->m_fd, (struct sockaddr *)&cliaddr, &len);
            Socket::ptr clientsock = myev->sockptr->accept();
            if (clientsock == nullptr) // 表示出错了
            {
                break;
            }
            else if (clientsock) // 表示有连接
            {
                Socket::sockfcntl(clientsock->getSocket(), F_SETFL, O_NONBLOCK);
                int buffer_size = 1024 * 1024; // 2MB，你可以根据需要设置缓冲区大小

                // 设置发送缓冲区大小
                clientsock->setOption(SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));

                // 设置接收缓冲区大小
                clientsock->setOption(SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));

                reactor_m->filedata_event.init();
                const int *epfd_x = fdmagr->find_epfd(sub_re, reactor_s->epollmgr); // 找到一个有位置的epfd

                CALL_BACK recvfun = My_events::make_callback(&Connection::handleEvent, conn_.get(), &(reactor_m->filedata_event), fdmagr, 2, 0);
                reactor_m->filedata_event.Myev_set(clientsock, recvfun, clientsock->getRemoteAddress(), *epfd_x); // 把得到的客户端通信文件描述符添加到类对象中，并且设置回调函数
                reactor_m->filedata_event.type = 1;
                reactor_m->filedata_event.epfd_type = reactor_type::sub_re;

                fdmagr->add_epfd_event(*epfd_x, sub_re);
                // reactor_->getEpollMgr()->eventadd(EPOLLIN | EPOLLET | EPOLLONESHOT, &(reactor_->filedata_event), *epfd_x); // 把客户端类对象添加到红黑树上
                reactor_s->registerHandler(&(reactor_m->filedata_event), EPOLLIN | EPOLLET | EPOLLONESHOT);
                LY_LOG_INFO(g_logger) << "new connect"
                                      << "[" << clientsock->getRemoteAddress()->toString() << "],["
                                      << "time:" << reactor_m->filedata_event.last_active << "], pos[" << clientsock->getSocket() << "] "
                                      << "at epfd:" << epfd_x;
            }
        }
    }
    // 回调函数 接受连接
    void Acceptor::acceptconnect(My_events *myev, Fdmanager::ptr fdmagr, int listen_type, uint64_t m_recvTimeout)
    {
        while (1)
        {
            Socket::ptr clientsock = myev->sockptr->accept();
            if (clientsock == nullptr) // 表示出错了
            {
                break;
            }
            else if (clientsock) // 表示有连接
            {
                clientsock->setRecvTimeout(m_recvTimeout);
                Socket::sockfcntl(clientsock->getSocket(), F_SETFL, O_NONBLOCK);

                My_events *my_ev_cfd = fdmagr->lookup(clientsock->getSocket());
                my_ev_cfd->init();
                const int *epfd_x = fdmagr->find_epfd(sub_re, reactor_s->epollmgr); // 找到一个有位置的epfd
                if (listen_type == 0)
                {
                    CALL_BACK recvfun = My_events::make_callback(&Connection::handleEvent, conn_.get(), my_ev_cfd, fdmagr, 0, m_recvTimeout);
                    my_ev_cfd->Myev_set(clientsock, recvfun, clientsock->getRemoteAddress(), *epfd_x); // 把得到的客户端通信文件描述符添加到类对象中，并且设置回调函数
                    my_ev_cfd->type = 1;
                    my_ev_cfd->epfd_type = reactor_type::sub_re;
                    if (this->client_Timer == "Yes")
                    {
                        // 创建定时器
                        CALL_BACK activefun = My_events::make_callback(&MainReactor::client_notactive, reactor_m.get(), my_ev_cfd, fdmagr);
                        Timer::ptr my_T = reactor_m->addTimer(60000, activefun, true);
                        my_ev_cfd->T = my_T;
                    }
                }
                else
                {
                    CALL_BACK recvfun = My_events::make_callback(&Connection::FTP_recvdata, conn_.get(), my_ev_cfd, fdmagr);
                    my_ev_cfd->Myev_set(clientsock, recvfun, clientsock->getRemoteAddress(), *epfd_x); // 把得到的客户端通信文件描述符添加到类对象中，并且设置回调函数
                    my_ev_cfd->type = 1;
                    my_ev_cfd->m_file_event = &(reactor_m->filedata_event);
                    my_ev_cfd->epfd_type = reactor_type::sub_re;
                    // FTP监听，不需要定时器
                }

                int event_count = fdmagr->add_epfd_event(*epfd_x, sub_re);
                // reactor_s->getEpollMgr()->eventadd(EPOLLIN | EPOLLET | EPOLLONESHOT, my_ev_cfd, *epfd_x); // 把客户端类对象添加到红黑树上
                reactor_s->registerHandler(my_ev_cfd, EPOLLIN | EPOLLET | EPOLLONESHOT);
                LY_LOG_INFO(g_logger) << "new connect"
                                      << "[" << clientsock->getRemoteAddress()->toString() << "],["
                                      << "time:" << my_ev_cfd->last_active << "], pos[" << clientsock->getSocket() << "] "
                                      << "at epfd:" << *epfd_x << " event count :" << event_count;
            }
        }
    }
    void Acceptor::httpaccept(My_events *myev, Fdmanager::ptr fdmagr, uint64_t m_recvTimeout)
    {
        while (1)
        {
            Socket::ptr clientsock = myev->sockptr->accept();
            if (clientsock == nullptr) // 表示出错了
            {
                break;
            }
            else if (clientsock) // 表示有连接
            {
                clientsock->setRecvTimeout(m_recvTimeout);
                Socket::sockfcntl(clientsock->getSocket(), F_SETFL, O_NONBLOCK);

                My_events *my_ev_cfd = fdmagr->lookup(clientsock->getSocket());
                my_ev_cfd->init();
                const int *epfd_x = fdmagr->find_epfd(sub_re, reactor_s->epollmgr); // 找到一个有位置的epfd

                CALL_BACK recvfun = My_events::make_callback(&Connection::handleEvent, conn_.get(), my_ev_cfd, fdmagr, 3, m_recvTimeout);
                my_ev_cfd->Myev_set(clientsock, recvfun, clientsock->getRemoteAddress(), *epfd_x); // 把得到的客户端通信文件描述符添加到类对象中，并且设置回调函数
                my_ev_cfd->type = 1;
                my_ev_cfd->epfd_type = reactor_type::sub_re;

                int event_count = fdmagr->add_epfd_event(*epfd_x, sub_re);
                // reactor_s->getEpollMgr()->eventadd(EPOLLIN | EPOLLET | EPOLLONESHOT, my_ev_cfd, *epfd_x); // 把客户端类对象添加到红黑树上
                reactor_s->registerHandler(my_ev_cfd, EPOLLIN | EPOLLET | EPOLLONESHOT);
                LY_LOG_INFO(g_logger) << "new connect"
                                      << "[" << clientsock->getRemoteAddress()->toString() << "],["
                                      << "time:" << my_ev_cfd->last_active << "], pos[" << clientsock->getSocket() << "] "
                                      << "at epfd:" << *epfd_x << " event count :" << event_count;
            }
        }
    }
    Connection::Connection(SubReactor::ptr reactor) : reactor_(reactor)
    {
        // 初始化操作
    }
    void Connection::handleEvent(My_events *myev, Fdmanager::ptr fdmagr, int listen_type, uint64_t m_recvTimeout)
    {
        if (listen_type == 0)
        {
            recvdata(myev, fdmagr);
        }
        else if (listen_type == 1)
        {
            senddata(myev, fdmagr);
        }
        else if (listen_type == 2)
        {
            handle_fileevent(myev, fdmagr);
        }
        else if (listen_type == 3)
        {
            handle_http_event(myev, fdmagr);
        }
    }
    // 接受客户端发送的数据 可读事件回调函数
    void Connection::recvdata(My_events *myev, Fdmanager::ptr fdmagr)
    {
        this->update_time(myev); // 更新时间
        int len;
        int event;
        CALL_BACK recvfun;
        while (1)
        {
            char *buf = NULL;
            len = r_s_Msg::readMsg(GET_SOCK_FD(myev), &buf);
            // char *buf = (char*)malloc(sizeof(char) * 1024);
            // len = myev->sockptr->recv(buf, 1024);
            if (len > 0)
            {
                // printf("%s\n", buf);
                int ret_ = this->JudgeEventType(buf, myev);
                if (ret_ == -2)
                {
                    LY_LOG_ERROR(g_logger) << "buf不是Json对象";
                }
                else if (ret_ == -1) // 表示用ID初始化成功
                {
                    LY_LOG_INFO(g_logger) << "用户ID初始化成功";
                    recvfun = My_events::make_callback(&Connection::recvdata, this, myev, fdmagr);
                    reactor_->getEpollMgr()->reset_oneshot(myev, EPOLLIN, recvfun);
                    free(buf);
                    break;
                }
                else if (ret_ == -3) // 表示错误ID
                {
                    free(buf);
                    break;
                }
                else if (ret_ == 0) // 表示需要转发数据
                {
                    myev->readBuffer->append(buf, len); // 把接收到的数据放在客户端类的成员属性（Buffer类）
                    event = EPOLLOUT;
                    // recvfun = std::bind(&Epoll_reactor::senddata, er_ptr, myev, fdmagr);
                    recvfun = My_events::make_callback(&Connection::senddata, this, myev, fdmagr);
                    LY_LOG_INFO(g_logger) << "转发数据";
                }
                else if (ret_ > 0)
                { // 已经处理了
                    event = EPOLLIN;
                    // recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    recvfun = My_events::make_callback(&Connection::recvdata, this, myev, fdmagr);
                    LY_LOG_INFO(g_logger) << "已经处理了";
                }
            }
            else if (len == 0)
            { // 客户端主动关闭连接
                /*还有一种情况，未知ID发送的数据不满足粘包协议，读了一些数据，所以返回0*/
                fdmagr->free_client_resources(myev, "disconnection_", true, fdmagr->get_epollmgr(myev));
                if (buf != NULL)
                {
                    free(buf);
                }
                else
                {
                    LY_LOG_INFO(g_logger) << "未释放，因为是空的";
                }
                break;
            }
            else if (len == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // 表示读缓冲区没有可读的数据了，叫你稍后再尝试读取
                {
                    // printf("数据读完了\n");
                    myev->m_buf_len = myev->readBuffer->size();

                    reactor_->epollmgr->reset_oneshot(myev, event, recvfun);
                    free(buf);
                    break;
                }
                else if (errno == EINTR) // 这个错误码表明系统调用被某个信号中断，通常是因为进程接收到了一个信号，例如 SIGINT（终止进程的信号）或 SIGTERM（终止进程的信号）
                {
                    // 被信号中断，则重新尝试
                    LY_LOG_WARN(g_logger) << "continue reading";
                    continue;
                }
                else if (errno == EBADF)
                { // 你应该确保在使用文件描述符之前检查它是否有效，以避免在无效的文件描述符上执行操作。如果文件描述符无效，通常需要进行适当的错误处理，例如关闭文件描述符或向用户报告错误。
                    // EBADF：sock不是有效的文件描述词,EFAULT：内存空间访问出错
                    fdmagr->free_client_resources(myev, "文件描述符无效", true, fdmagr->get_epollmgr(myev));
                    free(buf);
                    break;
                }
                else if (errno == EFAULT)
                {
                    // 内存空间访问出错,你应该确保传递给系统调用的指针是有效的，并且指向合法的内存地址。如果指针无效，你应该采取适当的措施，例如检查指针是否为空（NULL）或检查指针是否越界。
                    fdmagr->free_client_resources(myev, "内存空间访问出错", true, fdmagr->get_epollmgr(myev));
                    free(buf);
                    break;
                }
                else if (errno == ECONNREFUSED)
                {
                    // 表示连接尝试被目标主机拒绝，通常是由于目标服务未启动或防火墙规则等问题引起的。
                    fdmagr->free_client_resources(myev, "拒绝了", true, fdmagr->get_epollmgr(myev));
                    free(buf);
                    break;
                }
                else if (errno == ECONNRESET)
                {
                    // 错误表示在数据传输期间，远程主机（即客户端）强制关闭了连接，导致连接被重置。这通常发生在以下情况下：
                    //  客户端进程意外终止或崩溃，导致其操作系统强制关闭与服务器的连接。
                    //  客户端主机出现网络故障或中断，导致连接被中断。
                    fdmagr->free_client_resources(myev, "意外中断连接", true, fdmagr->get_epollmgr(myev));
                    free(buf);
                    break;
                }
                else
                {
                    printf("\n The [Client:%d] closed the connection \n", errno);
                    // recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    recvfun = My_events::make_callback(&Connection::recvdata, this, myev, fdmagr);
                    reactor_->epollmgr->reset_oneshot(myev, EPOLLIN, recvfun);
                }
            }
        }
    }
    // 发送数据，可写事件回调函数
    void Connection::senddata(My_events *myev, Fdmanager::ptr fdmagr)
    {

        int result;
        // 发送信息
        result = reactor_->EventH->send_function(myev, fdmagr);

        if (result >= 1) // 成功发送
        {
            // printf("发送了数据\n");
        }
        else if (result == 0) // 发送失败
        {
            LY_LOG_ERROR(g_logger) << "发送失败-------------";
        }
        else if (result == -1)
        {
            LY_LOG_INFO(g_logger) << "对方还没登录--------------";
        }
        else if (result == -2)
        {
            LY_LOG_INFO(g_logger) << "外来连接------------------";
        }
        else if (result == -3)
        {
            LY_LOG_ERROR(g_logger) << "缓冲区为空------------------";
        }
        // auto recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
        CALL_BACK recvfun = My_events::make_callback(&Connection::recvdata, this, myev, fdmagr);
        reactor_->getEpollMgr()->reset_oneshot(myev, EPOLLIN, recvfun);
        myev->readBuffer->clear(); // 发送完就清除缓冲区数据
    }
    void Connection::FTP_recvdata(My_events *myev, Fdmanager::ptr fdmagr)
    {
        this->update_time(myev); // 更新时间
        int len;
        int event;
        CALL_BACK recvfun;
        while (1)
        {
            char *buf = NULL;
            len = r_s_Msg::readMsg(GET_SOCK_FD(myev), &buf);
            // len = myev->sockptr->recv(buf, 1024);
            if (len > 0)
            {
                int ret_ = this->JudgeEventType(buf, myev);
                if (ret_ > 0)
                {
                    LY_LOG_INFO(g_logger) << "FTP处理完成";
                    event = EPOLLIN;
                    // recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    recvfun = My_events::make_callback(&Connection::FTP_recvdata, this, myev, fdmagr);
                }
            }
            else if (len == 0)
            {
                fdmagr->free_client_resources(myev, "ftp_disconnection_", false, fdmagr->get_epollmgr(myev));
                if (buf != NULL)
                {
                    free(buf);
                }
                else
                {
                    LY_LOG_INFO(g_logger) << "未释放，因为是空的";
                }
                break;
            }
            else if (len < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // 表示读缓冲区没有可读的数据了，叫你稍后再尝试读取
                {
                    // printf("数据读完了\n");
                    // myev->m_buf_len = myev->readBuffer->size();

                    reactor_->getEpollMgr()->reset_oneshot(myev, event, recvfun);
                    free(buf);
                    break;
                }
                else if (errno == EINTR) // 这个错误码表明系统调用被某个信号中断，通常是因为进程接收到了一个信号，例如 SIGINT（终止进程的信号）或 SIGTERM（终止进程的信号）
                {
                    // 被信号中断，则重新尝试
                    LY_LOG_WARN(g_logger) << "continue reading";
                    continue;
                }
            }
        }
    }
    void Connection::handle_fileevent(My_events *myev, Fdmanager::ptr fdmagr)
    {
        char buf[BUFF_SIZE];
        while (1)
        {
            memset(buf, 0, BUFF_SIZE);
            int ret = recv(GET_SOCK_FD(myev), buf, BUFF_SIZE, 0);
            if (ret > 0)
            {
                FTP::client_2_server_recvFile(buf, ret);
            }
            else if (ret == 0)
            {
                fdmagr->free_client_resources(myev, "20端口断开了", false, fdmagr->get_epollmgr(myev));
                LY_LOG_INFO(g_logger) << "传输数据端断开了连接";

                break;
            }
            else if (ret == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    CALL_BACK recvfun = My_events::make_callback(&Connection::handle_fileevent, this, myev, fdmagr);
                    reactor_->getEpollMgr()->reset_oneshot(myev, reactor_type::sub_re, recvfun);
                    break;
                }
                else
                {
                    // printf("\n The [Client:%d] closed the connection \n", errno);
                    fdmagr->free_client_resources(myev, "断开l", false, fdmagr->get_epollmgr(myev));
                    break;
                }
            }
        }
    }
    /**
     * @brief HTTP处理事件
     * 
     * @param myev 
     * @param fdmagr 
     */
    void Connection::handle_http_event(My_events *myev, Fdmanager::ptr fdmagr)
    {
        int ret = Http_handle_cb(myev->sockptr);
        CALL_BACK Httpfun;
        if (ret > 0)
        {
            Httpfun = My_events::make_callback(&Connection::handle_http_event, this, myev, fdmagr);
        }
        else if (ret == 0)
        {
            fdmagr->free_client_resources(myev, "Http disconnect_", false, fdmagr->get_epollmgr(myev));
            return;
        }
        else if (ret == -1)
        {
            if (errno == EAGAIN)
            {
                Httpfun = My_events::make_callback(&Connection::handle_http_event, this, myev, fdmagr);
                reactor_->getEpollMgr()->reset_oneshot(myev, EPOLLIN, Httpfun);
            }
            else
            {
                
                LY_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                                       << errno << " errstr=" << strerror(errno)
                                       << " cliet:" << *(myev->sockptr);
                fdmagr->free_client_resources(myev, strerror(errno), false, fdmagr->get_epollmgr(myev));
            }

            return;
        }

        reactor_->getEpollMgr()->reset_oneshot(myev, EPOLLIN, Httpfun);
    }
    // 判断事件类型
    int Connection::JudgeEventType(const char *buff, My_events *myev)
    {
        Value obj;
        Reader r;
        int ret;
        string info(buff);
        r.parse(info, obj); // 把字符串转换成Value数据
        if (obj.isObject())
        {
            string ev;
            ev = obj["REQUEST"].asString();
            EventType ev_type = StringToEventType(ev);
            if (ev_type == EventType::TRANSMIT_REQUEST)
            {
                // 转发请求
                return 0;
            }
            else
            {
                Event_Callback cb = reactor_->EventH->callback_map[ev_type]; // 得到回调函数
                ret = cb(obj, myev);
            }

            // execute_cb(cb, My_events *myev, Epoll_reactor *er_ptr);
            return ret;
        }
        return -2;
    }
    void Connection::update_time(My_events *myev)
    {
        myev->last_active = GetCurrentMS();
    }

    MainReactor::MainReactor(const std::string &server_Timer)
    {
        this->server_Timer = server_Timer;
        // 初始化操作
        epollmgr.reset(new Epollmanager);

        fdmagr->addepfd(EPFD(this), main_re, *epollmgr);

        // 添加管道到epoll中
        int rt = pipe(m_tickleFds);
        LY_ASSERT(!rt);
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_tickleFds[0];

        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        LY_ASSERT(!rt);
        rt = epoll_ctl(EPFD(this), EPOLL_CTL_ADD, m_tickleFds[0], &event);
        LY_ASSERT(!rt);
    }
    bool MainReactor::stopping(uint64_t &timeout)
    {
        timeout = getNextTimer();
        return timeout == ~0ull;
    }
    void MainReactor::onTimerInsertedAtFront()
    {
        int rt = write(m_tickleFds[1], "T", 1); // 唤醒epoll_wait
        assert(rt > 0);
    }
    void MainReactor::init(ThreadPool::ptr p)
    {
        if (server_Timer == "Yes")
        {
            mg_events_Timer.type = 2;
            CALL_BACK detectionfun = My_events::make_callback(&MainReactor::detection_client, this, p, fdmagr); // 创建定时器，并且开始定时器
            this->addTimer(1000, detectionfun, true);
        }

        EventH->register_Event_callback(); // 注册回调函数
    }
    void MainReactor::Eventloop(ThreadPool::ptr p)
    {
        while (1)
        {
            uint64_t next_timeout = 0;
            if (stopping(next_timeout))
            {
                // std::cout << "主线程停止 exit";
                LY_LOG_ERROR(g_logger) << "主线程停止 exit";
                break;
            }
            /*epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout)
            epfd：epoll_create() 函数的返回值, 通过这个参数找到epoll实例
            events：传出参数, 这是一个结构体数组的地址, 里边存储了已就绪的文件描述符的信息
            maxevents：修饰第二个参数, 结构体数组的容量（元素个数）
            timeout：如果检测的epoll实例中没有已就绪的文件描述符，该函数阻塞的时长, 单位ms 毫秒
                0：函数不阻塞，不管epoll实例中有没有就绪的文件描述符，函数被调用后都直接返回
                大于0：如果epoll实例中没有已就绪的文件描述符，函数阻塞对应的毫秒数再返回
                -1：函数一直阻塞，直到epoll实例中有已就绪的文件描述符之后才解除阻塞
            函数返回值：
                成功：
                    等于0：添加了定时器，函数是阻塞被强制解除了, 没有检测到满足条件的文件描述符
                    大于0：检测到的已就绪的文件描述符的总个数
                失败：返回-1
            */
            int n_ready;
            do
            {
                static const int MAX_TIMEOUT = 2000;
                if (next_timeout != ~0ull)
                {
                    next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
                }
                else
                {
                    next_timeout = MAX_TIMEOUT;
                }
                n_ready = epoll_wait(EPFD(this), evs_main, MAX_EVENTS, (int)next_timeout); // 检测就绪事件
                if (n_ready < 0 && g_received == Signal::SIGINT_)
                {
                    LY_LOG_ERROR(g_logger) << "epoll_wait error!, " << strerror(errno);
                    g_received = Signal::SIGNOT_;
                    return;
                }
                else
                {
                    break;
                }
            } while (true);
            std::vector<std::function<void()>> cbs;
            listExpiredCb(cbs); // 获得所有超时的定时器
            if (!cbs.empty())
            {
                // LY_LOG_DEBUG(g_logger) << "on timer cbs.size=" << cbs.size();
                for (auto &cb : cbs)
                {
                    p->addTask(Task(cb)); // 把回调函数添加到线程池中进行运行
                }
                cbs.clear();
            }
            for (int i = 0; i < n_ready; i++)
            {
                eventqueue.push(evs_main[i]);
            }
            if (eventqueue.size() > 0)
            {
                handleEvents(p); // 处理事件
            }
        }
    }

    void MainReactor::detection_client(ThreadPool::ptr p, Fdmanager::ptr fdmagr)
    {
        Json::Value obj_state;
        int temp_qt_fd;

        temp_qt_fd = fdmagr->ID_2_fd("Qt");
        int alivethread = p->getAliveNumber();
        int busythread = p->getBusyNumber();
        if (temp_qt_fd > 0)
        {
            // 往对象中添加键值对
            obj_state["CLIENT_STATE"] = EventTypeToString(EventType::CLIENT_STATE);
            obj_state["Qt_state"] = fdmagr->is_exist("Qt");
            obj_state["battery_state"] = fdmagr->is_exist("battery");
            obj_state["uportrait_state"] = fdmagr->is_exist("uportrait");
            obj_state["action_state"] = fdmagr->is_exist("action");
            obj_state["xavier_state"] = fdmagr->is_exist("xavier");
            obj_state["alive_thread"] = alivethread;
            obj_state["busy_thread"] = busythread;

            string str = obj_state.toStyledString();
            r_s_Msg::sendMsg(temp_qt_fd, str.c_str(), strlen(str.c_str()));
        }
        else
        {
            // std::cout << "Qt未登录" << endl;
        }
    }

    void MainReactor::client_notactive(My_events *myevent, Fdmanager::ptr fdmagr)
    {
        uint16_t now = GetCurrentMS();
        uint16_t duration = now - myevent->last_active; // 当前时间与客户端连接的时间的差
        if (duration >= 60000)
        {
            Json::Value close_conn;
            close_conn["CLOSE"] = string(EventTypeToString(EventType::CLOSE_CONNECTION));
            string str = close_conn.toStyledString();

            r_s_Msg::sendMsg(GET_SOCK_FD(myevent), str.c_str(), strlen(str.c_str()));
            fdmagr->free_client_resources(myevent, "客户端未活跃，关闭了", true, epollmgr.get());
        }
    }
    void MainReactor::registerHandler(My_events *myev, int events)
    {
        this->epollmgr->eventadd(events, myev, this->epollmgr->get_epfd());
    }

    void MainReactor::removeHandler(My_events *myev)
    {
        this->epollmgr->eventdel(myev);
    }

    void MainReactor::handleEvents(ThreadPool::ptr p)
    {
        // pool->addTask(Task(cb));

        int n_ready = eventqueue.size();
        // 得到就绪文件描述符数组后开始遍历
        for (int i = 0; i < n_ready; ++i)
        {

            epoll_event &event = eventqueue.front();
            if (event.data.fd == m_tickleFds[0])
            {
                uint8_t dummy[256];
                eventqueue.pop();
                while (read(m_tickleFds[0], dummy, sizeof(dummy)) > 0)
                    ;
                continue;
            }

            My_events *myev = (My_events *)event.data.ptr;
            if (event.events & EPOLLIN && myev->m_events & EPOLLIN) // 监听读事件
            {
                if (myev->type == 0) // 连接事件
                {
                    p->addTask(Task(myev->call_back)); // 把回调函数添加到线程池中进行运行
                }
                else if (myev->type == 3)
                {
                    p->addTask(Task(myev->call_back)); // 线程池会释放info
                }
            }
            if (event.events & EPOLLERR)
            {
                // cout << "异常事件" << endl;
                LY_LOG_ERROR(g_logger) << "异常事件";
            }
            eventqueue.pop();
        }
    }
    SubReactor::SubReactor()
    {
        // 初始化操作
        epollmgr.reset(new Epollmanager);
        fdmagr->addepfd(EPFD(this), sub_re, *epollmgr);
    }
    void SubReactor::subEventloop(ThreadPool::ptr p)
    {
        while (1)
        {
            int n_ready;
            do
            {
                n_ready = epoll_wait(EPFD(this), this->evs_sub, MAX_EVENTS, -1); // 检测就绪事件
                if (n_ready < 0 && g_received == Signal::SIGINT_)
                {
                    LY_LOG_ERROR(g_logger) << "epoll_wait error!, " << strerror(errno);
                    g_received = Signal::SIGNOT_;
                    return;
                }
                else
                {
                    break;
                }
            } while (true);

            for (int i = 0; i < n_ready; i++)
            {
                eventqueue.push(evs_sub[i]);
            }
            if (eventqueue.size() > 0)
            {
                handleEvents(p); // 处理事件
            }
        }
    }

    void SubReactor::registerHandler(My_events *myev, int events)
    {
        this->epollmgr->eventadd(events, myev, this->epollmgr->get_epfd());
    }

    void SubReactor::removeHandler(My_events *myev)
    {
        this->epollmgr->eventdel(myev);
    }

    void SubReactor::handleEvents(ThreadPool::ptr pool)
    {

        int n_ready = eventqueue.size();
        // 得到就绪文件描述符数组后开始遍历
        for (int i = 0; i < n_ready; ++i)
        {
            epoll_event &event = eventqueue.front();
            My_events *myev = (My_events *)(event.data.ptr);
            if (event.events & EPOLLIN && myev->m_events & EPOLLIN) // 监听读事件
            {
                pool->addTask(Task(myev->call_back)); // 把回调函数添加到线程池中进行运行
            }
            if (event.events & EPOLLOUT && myev->m_events & EPOLLOUT) // 监听写事件
            {
                pool->addTask(Task(myev->call_back)); // 把回调函数添加到线程池中进行运行
            }
            if (event.events & EPOLLERR)
            {
                LY_LOG_ERROR(g_logger) << "异常事件";
            }
            eventqueue.pop();
        }
    }

}