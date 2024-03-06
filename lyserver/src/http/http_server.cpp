#include "http_server.h"
#include "log.h"
// #include "lyserver/http/servlets/config_servlet.h"
// #include "lyserver/http/servlets/status_servlet.h"

namespace lyserver
{
    namespace http
    {

        static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");

        HttpServer::HttpServer(bool keepalive) : TcpServer(), m_isKeepalive(keepalive)
        {
            m_dispatch.reset(new ServletDispatch);

            m_type = "http";
            // m_dispatch->addServlet("/_/status", Servlet::ptr(new StatusServlet));
            // m_dispatch->addServlet("/_/config", Servlet::ptr(new ConfigServlet));
        }

        void HttpServer::setName(const std::string &v)
        {
            TcpServer::setName(v);
            m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
        }

        int HttpServer::handleClient(Socket::ptr client)
        {
            // LY_LOG_DEBUG(g_logger) << "handleClient " << *client;
            HttpSession::ptr session(new HttpSession(client, false));
            int ret = -2;
            do
            {
                ret = -2;
                auto req = session->recvRequest(ret);
                if (ret > 0 && req)
                {
                    HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
                    rsp->setHeader("Server", getName());
                    m_dispatch->handle(req, rsp, session);
                    session->sendResponse(rsp);

                    if (!m_isKeepalive || req->isClose())
                    {
                        std::cout<< "断开连接:"<<client->getSocket() << " m_isKeepalive: "<<m_isKeepalive << " m_close: "<<req->isClose() << std::endl;
                        return 0;
                    }
                }
                else if (ret == 0)
                {

                }
                else if (ret == -1 && req == nullptr)
                {
                    // if(errno == EAGAIN){
                    //     break;
                    // }
                    // if (!req)
                    // {
                    //     LY_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                    //                            << errno << " errstr=" << strerror(errno)
                    //                            << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
                    //     break;
                    // }
                    break;
                }

            } while (true);
            return ret;
        }
        call_back_ HttpServer::make_cb()
        {
            auto cb = [this](Socket::ptr client)->int
            {
                return this->handleClient(client);
            };
            return cb;
        }

    }
}
