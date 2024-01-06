// epoll基于非阻塞I/O事件驱动
#include "reactor.h"
namespace lyserver
{

    pthread_mutex_t Epoll_reactor::mutex_epfd = PTHREAD_MUTEX_INITIALIZER;

    Epoll_reactor::Epoll_reactor()
    {

        epollmgr.reset(Epollmagr::GetInstance()); // epoll管理对象

        fdmagr.reset(FdMgr::GetInstance()); // fd存储结构

        // MyTimer.reset(new Timerfd);
        EventH.reset(new EventHandle);
        if (fdmagr->get_block() < 1)
        {
            cout << "创建fdStor错误" << __LINE__ << endl;
            return;
        }
        fdmagr->addepfd(EPFD_, main_re);
        fdmagr->addepfd(EPFD_SUB_, sub_re);

        // 创建线程池
        // pool = new ThreadPool(4, 10);
        pool.reset(new ThreadPool(4, 10));

        int rt = pipe(m_tickleFds);
        assert(!rt);
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_tickleFds[0];

        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        assert(!rt);
        rt = epoll_ctl(EPFD_, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        assert(!rt);
    }
    Epoll_reactor::~Epoll_reactor()
    {
    }

    // 回调函数 接受连接
    void Epoll_reactor::acceptconnect(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr, ThreadPool::ptr p)
    {
        int flag = 0;
        // int i;
        int cfd;
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);
        while (1)
        {
            cfd = accept(myev->m_fd, (struct sockaddr *)&cliaddr, &len);
            if (cfd == -1) // 表示出错了
            {
                if (errno == EAGAIN && errno == EWOULDBLOCK)
                { // 暂不处理
                    cout << "没有更多的连接了" << endl;
                    break;
                }
                printf("%s:accept,%s", __func__, strerror(errno));
                break;
            }
            else if (cfd > 0) // 表示有连接
            {
                My_events *my_ev_cfd = fdmagr->lookup(cfd);
                do
                {
                    if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) // 将cfd设置为非阻塞
                    {
                        printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
                        break;
                    }
                    my_ev_cfd->init();
                    const int *epfd_x = fdmagr->find_epfd(sub_re); // 找到一个有位置的epfd
                    // auto recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, my_ev_cfd, fdmagr);
                    CALL_BACK recvfun = My_events::make_callback(Epoll_reactor::recvdata, er_ptr, my_ev_cfd, fdmagr);
                    my_ev_cfd->Myev_set(cfd, recvfun, cliaddr, epfd_x); // 把得到的客户端通信文件描述符添加到类对象中，并且设置回调函数
                    my_ev_cfd->type = 1;
                    my_ev_cfd->epfd_type = reactor_type::sub_re;
                    // 创建定时器
                    // auto activefun = std::bind(&Epoll_reactor::client_notactive, my_ev_cfd, fdmagr);
                    CALL_BACK activefun = My_events::make_callback(&Epoll_reactor::client_notactive, my_ev_cfd, fdmagr);
                    Timer::ptr my_T = er_ptr->addTimer(60000, activefun, true);
                    my_ev_cfd->T = my_T;

                    fdmagr->add_epfd_event(*epfd_x, sub_re);
                    er_ptr->epollmgr->eventadd(EPOLLIN | EPOLLET | EPOLLONESHOT, my_ev_cfd, *epfd_x); // 把客户端类对象添加到红黑树上
                } while (0);

                printf("new connect[%s:%d],[time:%ld],pos[%d]\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), my_ev_cfd->last_active, cfd);
            }
        }
    }
    void Epoll_reactor::udp_recv(int udpSocket)
    {
        // 接收数据
        char buffer[50000];
        memset(buffer, 0, sizeof(buffer));

        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        ssize_t bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (bytesRead == -1)
        {
            std::cerr << "Error: Failed to receive data." << std::endl;
            return;
        }

        // 在这里处理接收到的数据
        // 你可以将 buffer 转换为图像或进行其他处理
        std::cout << "Received data from " << inet_ntoa(clientAddr.sin_addr) << ": " << buffer << std::endl;
    }
    // 接受客户端发送的数据 可读事件回调函数
    void Epoll_reactor::recvdata(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr)
    {
        er_ptr->update_time(myev); // 更新时间
        ssize_t len;
        int event;
        CALL_BACK recvfun;
        while (1)
        {
            char *buf = NULL;
            len = r_s_Msg::readMsg(myev->m_fd, &buf);
            // char buf[1024 * 1024];
            // len = recv(myev->m_fd, buf, sizeof(buf), 0);
            if (len > 0)
            {
                // printf("%s\n", buf);
                int ret_ = er_ptr->JudgeEventType(buf, myev, er_ptr);
                if (ret_ == -2)
                {
                    printf("buf不是Json对象\n");
                }
                else if (ret_ == -1) // 表示用ID初始化成功
                {
                    printf("用户ID初始化成功\n");
                    // recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    recvfun = My_events::make_callback(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    er_ptr->epollmgr->reset_oneshot(myev, EPOLLIN, recvfun);
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
                    recvfun = My_events::make_callback(Epoll_reactor::senddata, er_ptr, myev, fdmagr);
                    printf("转发数据\n");
                }
                else if (ret_ > 0)
                { // 已经处理了
                    event = EPOLLIN;
                    // recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    recvfun = My_events::make_callback(Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    printf("已经处理了\n");
                }
            }
            else if (len == 0)
            { // 客户端主动关闭连接
                /*还有一种情况，未知ID发送的数据不满足粘包协议，读了一些数据，所以返回0*/
                fdmagr->free_client_resources(myev, "disconnection_", true);
                if (buf != NULL)
                {
                    free(buf);
                }
                else
                {
                    cout << "未释放，因为是空的" << endl;
                }
                break;
            }
            else if (len == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK) // 表示读缓冲区没有可读的数据了，叫你稍后再尝试读取
                {
                    // printf("数据读完了\n");
                    myev->m_buf_len = myev->readBuffer->size();

                    er_ptr->epollmgr->reset_oneshot(myev, event, recvfun);
                    free(buf);
                    break;
                }
                else if (errno == EINTR) // 这个错误码表明系统调用被某个信号中断，通常是因为进程接收到了一个信号，例如 SIGINT（终止进程的信号）或 SIGTERM（终止进程的信号）
                {
                    // 被信号中断，则重新尝试
                    printf("continue reading\n");
                    continue;
                }
                else if (errno == EBADF)
                { // 你应该确保在使用文件描述符之前检查它是否有效，以避免在无效的文件描述符上执行操作。如果文件描述符无效，通常需要进行适当的错误处理，例如关闭文件描述符或向用户报告错误。
                    // EBADF：sock不是有效的文件描述词,EFAULT：内存空间访问出错
                    fdmagr->free_client_resources(myev, "文件描述符无效", true);
                    free(buf);
                    break;
                }
                else if (errno == EFAULT)
                {
                    // 内存空间访问出错,你应该确保传递给系统调用的指针是有效的，并且指向合法的内存地址。如果指针无效，你应该采取适当的措施，例如检查指针是否为空（NULL）或检查指针是否越界。
                    fdmagr->free_client_resources(myev, "内存空间访问出错", true);
                    free(buf);
                    break;
                }
                else if (errno == ECONNREFUSED)
                {
                    // 表示连接尝试被目标主机拒绝，通常是由于目标服务未启动或防火墙规则等问题引起的。
                    fdmagr->free_client_resources(myev, "拒绝了", true);
                    free(buf);
                    break;
                }
                else if (errno == ECONNRESET)
                {
                    // 错误表示在数据传输期间，远程主机（即客户端）强制关闭了连接，导致连接被重置。这通常发生在以下情况下：
                    //  客户端进程意外终止或崩溃，导致其操作系统强制关闭与服务器的连接。
                    //  客户端主机出现网络故障或中断，导致连接被中断。
                    fdmagr->free_client_resources(myev, "意外中断连接", true);
                    free(buf);
                    break;
                }
                else
                {
                    printf("\n The [Client:%d] closed the connection \n", errno);
                    // recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    recvfun = My_events::make_callback(Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
                    er_ptr->epollmgr->reset_oneshot(myev, EPOLLIN, recvfun);
                }
            }
        }
    }
    void Epoll_reactor::update_time(My_events *myev)
    {
        myev->last_active = GetCurrentMS();
    }
    // 发送数据，可写事件回调函数
    void Epoll_reactor::senddata(Epoll_reactor *er_ptr, My_events *myev, Fdmanager::ptr fdmagr)
    {

        int result;
        // printf("准备发送了\n");
        // 发送信息
        result = er_ptr->send_function(myev, fdmagr);
        // myev->readBuffer->append("\n", 1);
        // result = send(myev->m_fd, myev->readBuffer->c_str(), myev->m_buf_len, 0);

        if (result >= 1) // 成功发送
        {
            // printf("发送了数据\n");
        }
        else if (result == 0) // 发送失败
        {
            cout << "发送失败-------------" << endl;
        }
        else if (result == -1)
        {
            cout << "对方还没登录--------------" << endl;
        }
        else if (result == -2)
        {
            cout << "外来连接------------------" << endl;
        }
        else if (result == -3)
        {
            cout << "缓冲区为空----------------" << endl;
        }
        // auto recvfun = std::bind(&Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
        CALL_BACK recvfun = My_events::make_callback(Epoll_reactor::recvdata, er_ptr, myev, fdmagr);
        er_ptr->epollmgr->reset_oneshot(myev, EPOLLIN, recvfun);
        myev->readBuffer->clear(); // 发送完就清除缓冲区数据
    }
    bool Epoll_reactor::stopping(uint64_t &timeout)
    {
        timeout = getNextTimer();
        return timeout == ~0ull;
    }
    void Epoll_reactor::onTimerInsertedAtFront()
    {
        int rt = write(m_tickleFds[1], "T", 1); // 唤醒epoll_wait
        assert(rt > 0);
    }
    // 初始化监听套接字
    void Epoll_reactor::initlistensocket(uint32_t address, uint16_t port)
    {

        int lfd;
        struct sockaddr_in seraddr;

        // 创建监听套接字
        lfd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 端口复用
        // 将监听套接字设置非阻塞
        fcntl(lfd, F_SETFL, O_NONBLOCK);

        bzero(&seraddr, sizeof(seraddr));
        seraddr.sin_family = AF_INET;
        seraddr.sin_addr.s_addr = htonl(address);
        seraddr.sin_port = htons(port);

        // 将监听文件描述符与初始化的IP和端口进行绑定
        bind(lfd, (struct sockaddr *)&seraddr, sizeof(seraddr));

        // 设置监听上限
        listen(lfd, 128);

        printf("Server Running:port[%d]\n", SERVER_PORT);

        // lfd 初始化
        My_events *myev_lfd = fdmagr->lookup(lfd);
        // auto acfun = std::bind(&Epoll_reactor::acceptconnect, this, myev_lfd, fdmagr, pool); // 绑定器
        const int *epfd_x = fdmagr->find_epfd(main_re); // 找到一个有位置的epfd
        CALL_BACK acfun = My_events::make_callback(Epoll_reactor::acceptconnect, this, myev_lfd, fdmagr, pool);
        myev_lfd->Myev_set(lfd, acfun, epfd_x); // 初始化监听文件描述符所对应的类对象里面的文件描述符和回调函数
        myev_lfd->type = 0;
        myev_lfd->epfd_type = reactor_type::main_re;

        // 监听事件上树
        fdmagr->add_epfd_event(*epfd_x, main_re);
        epollmgr->eventadd(EPOLLIN | EPOLLET, myev_lfd, *epfd_x);

        // 创建UDP监听
        udp_l = new UdpReceiver("Broadc", 6666);
        My_events *myev_udp = fdmagr->lookup(udp_l->get_udp_listen());
        // auto udp_fun = std::bind(&Epoll_reactor::udp_recv, myev_udp->m_fd); // 绑定器
        CALL_BACK udp_fun = My_events::make_callback(Epoll_reactor::udp_recv, myev_udp->m_fd);
        const int *epfd_u = fdmagr->find_epfd(main_re); // 找到一个有位置的epfd
        myev_udp->Myev_set(udp_l->get_udp_listen(), udp_fun, epfd_u);
        myev_udp->type = 3;
        myev_udp->epfd_type = reactor_type::main_re;
        
        fdmagr->add_epfd_event(*epfd_u, main_re);
        epollmgr->eventadd(EPOLLIN, myev_udp, *epfd_u);
    }
    // 服务器运行
    void Epoll_reactor::server_run()
    {
        mg_events_Timer.type = 2;
        // auto detectionfun = std::bind(&Epoll_reactor::detection_client, pool, fdmagr);
        CALL_BACK detectionfun = My_events::make_callback(Epoll_reactor::detection_client, pool, fdmagr); // 创建定时器，并且开始定时器
        this->addTimer(1000, detectionfun, true);

        // auto subfun = std::bind(&Epoll_reactor::subreactor, this, pool, fdmagr); // 绑定器
        CALL_BACK subfun = My_events::make_callback(Epoll_reactor::subreactor, this, pool, fdmagr);
        pool->addTask(Task(subfun));

        EventH->register_Event_callback(); // 注册回调函数
        while (1)
        {
            uint64_t next_timeout = 0;
            if (stopping(next_timeout))
            {
                std::cout << "主线程停止 exit";
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
                static const int MAX_TIMEOUT = 3000;
                if (next_timeout != ~0ull)
                {
                    next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
                }
                else
                {
                    next_timeout = MAX_TIMEOUT;
                }
                n_ready = epoll_wait(EPFD_, evs_main, MAX_EVENTS, (int)next_timeout); // 检测就绪事件
                if (n_ready < 0 && errno == EINTR)
                {
                    printf("epoll_wait error!\n");
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
                // SYLAR_LOG_DEBUG(g_logger) << "on timer cbs.size=" << cbs.size();
                for (auto &cb : cbs)
                {
                    pool->addTask(Task(cb)); // 把回调函数添加到线程池中进行运行
                }
                cbs.clear();
            }
            // 得到就绪文件描述符数组后开始遍历
            for (int i = 0; i < n_ready; ++i)
            {
                epoll_event &event = evs_main[i];
                if (event.data.fd == m_tickleFds[0])
                {
                    uint8_t dummy[256];
                    while (read(m_tickleFds[0], dummy, sizeof(dummy)) > 0)
                        ;
                    continue;
                }

                My_events *myev = (My_events *)evs_main[i].data.ptr;
                if (evs_main[i].events & EPOLLIN) // 监听读事件
                {
                    // cout << "读"<<endl;
                    if (myev->type == 0) // 连接事件
                    {
                        pool->addTask(Task(myev->call_back)); // 把回调函数添加到线程池中进行运行
                    }
                    else if (myev->type == 3)
                    {
                        std::cout << "udp" << std::endl;
                        pool->addTask(Task(myev->call_back)); // 线程池会释放info
                    }
                }
                if (evs_main[i].events & EPOLLERR)
                {
                    cout << "异常事件" << endl;
                }
            }
        }
    }
    void Epoll_reactor::subreactor(Epoll_reactor *er_ptr, ThreadPool::ptr p, Fdmanager::ptr fdmagr)
    {
        Epollmanager::ptr epollmgr = er_ptr->get_Epollmgr();
        while (1)
        {
            int n_ready = epoll_wait(EPFD_SUB(er_ptr), er_ptr->evs_sub, MAX_EVENTS, -1); // 检测就绪事件
            if (n_ready < 0)
            {
                printf("epoll_wait error!\n");
                break;
            }
            // 得到就绪文件描述符数组后开始遍历
            for (int i = 0; i < n_ready; ++i)
            {
                My_events *myev = (My_events *)(er_ptr->evs_sub[i].data.ptr);
                if (er_ptr->evs_sub[i].events & EPOLLIN && myev->m_events & EPOLLIN) // 监听读事件
                {
                    p->addTask(Task(myev->call_back)); // 把回调函数添加到线程池中进行运行
                }
                if (er_ptr->evs_sub[i].events & EPOLLOUT && myev->m_events & EPOLLOUT) // 监听写事件
                {
                    p->addTask(Task(myev->call_back)); // 把回调函数添加到线程池中进行运行
                }
                if (er_ptr->evs_sub[i].events & EPOLLERR)
                {
                    cout << "异常事件" << endl;
                }
            }
        }
    }

    // 判断事件类型
    int Epoll_reactor::JudgeEventType(const char *buff, My_events *myev, Epoll_reactor *er_ptr)
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
                Event_Callback cb = this->EventH->callback_map[ev_type]; // 得到回调函数
                ret = cb(obj, myev);
            }

            // execute_cb(cb, My_events *myev, Epoll_reactor *er_ptr);
            return ret;
        }
        return -2;
    }

    string Epoll_reactor::get_info(const string &buff, const string &need)
    {
        Value obj;
        Reader r;
        string info;
        // Json::FastWriter fwriter;
        // Json::CharReaderBuilder reader;
        r.parse(buff, obj); // 把字符串转换成Value数据
        if (obj.isObject())
        {
            if (need == "ID")
            {
                info = obj["ID"].asString();
            }
            else if (need == "m_events")
            {
                info = obj["m_events"].asString();
            }
            else if (need == "toID")
            {
                info = obj["toID"].asString();
            }
        }
        else
        {
            return "HTTP";
        }
        return info;
    }
    int Epoll_reactor::send_function(My_events *myev, Fdmanager::ptr fdmagr) // 发送接收到的数据
    {
        string str = myev->readBuffer->get_str();
        std::string toID = get_info(str, "toID"); // 获得要给哪个ID发信息
        int fd = fdmagr->ID_2_fd(toID);
        // string x = "me";
        // int fd = toID_2_fd(x, myev);
        if (fd > 0)
        {
            bool situtation = sendjson(fd, str); // 准备好数据和fd后进行发送
            if (situtation == true)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else if (fd == -1) // 还未登录
        {
            return -1;
        }
        else if (fd == 0) // 外来连接
        {
            return -2;
        }
        return -2;
    }
    bool Epoll_reactor::send_str(int fd, string &buff)
    {
        int state = send(fd, buff.c_str(), strlen(buff.c_str()), 0);
        if (state > 0)
        {
            printf("向%s,发送了%d字节\n", "me", state);
            return true;
        }
        else if (state == -1)
        {
            printf("错误码:%s", strerror(errno));
        }
        return false;
    }
    bool Epoll_reactor::sendjson(int fd, string &buff)
    {
        Value obj;
        Reader r;

        r.parse(buff, obj); // 把字符串转换成Value数据
        if (obj.isObject())
        {
            Value s = obj["toID"];
            obj["RESPONSE"] = string(EventTypeToString(EventType::TRANSMIT_RESPOND));
            string id = s.toStyledString();
            id.erase(id.find_last_not_of("\n") + 1); // 删除末尾的换行符
            obj.removeMember("REQUEST");
            obj.removeMember("toID");          // 删除toID
            string str = obj.toStyledString(); // value转string
            int state = r_s_Msg::sendMsg(fd, str.c_str(), strlen(str.c_str()));
            if (state > 0)
            {
                printf("向%s,发送了%d字节\n", id.c_str(), state);
                return true;
            }
            else if (state == -1)
            {
                printf("错误码:%s", strerror(errno));
            }
        }
        return false;
    }

    /***************************************************************************************************************
     *定时器相关
     ****************************************************************************************************************
     */
    void Epoll_reactor::detection_client(ThreadPool::ptr p, Fdmanager::ptr fdmagr)
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

    void Epoll_reactor::client_notactive(My_events *myevent, Fdmanager::ptr fdmagr)
    {
        uint16_t now = GetCurrentMS();
        uint16_t duration = now - myevent->last_active; // 当前时间与客户端连接的时间的差
        if (duration >= 60000)
        {
            Json::Value close_conn;
            close_conn["CLOSE"] = string(EventTypeToString(EventType::CLOSE_CONNECTION));
            string str = close_conn.toStyledString();

            r_s_Msg::sendMsg(myevent->m_fd, str.c_str(), strlen(str.c_str()));
            fdmagr->free_client_resources(myevent, "客户端未活跃，关闭了", true);
            myevent->T->cancel();
        }
    }
}
