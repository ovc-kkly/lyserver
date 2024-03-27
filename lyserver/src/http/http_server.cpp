#include "http_server.h"
#include "log.h"
// #include "lyserver/http/servlets/config_servlet.h"
// #include "lyserver/http/servlets/status_servlet.h"

namespace lyserver
{
    namespace http
    {

        static lyserver::Logger::ptr g_logger = LY_LOG_NAME("system");

        HttpServer::HttpServer(bool keepalive, lyserver::IOManager *worker, lyserver::IOManager *io_worker, lyserver::IOManager *accept_worker)
            : TcpServer(worker, io_worker, accept_worker), m_isKeepalive(keepalive)
        {
            m_dispatch.reset(new ServletDispatch);

            m_type = "http";
            // m_dispatch->addServlet("/_/status", Servlet::ptr(new StatusServlet));
            // m_dispatch->addServlet("/_/config", Servlet::ptr(new ConfigServlet));

            urls.resize(100);
            urls[0] = std::make_shared<URLServlet>("url");
            urls[0]->setURL("/home/ly/lyserver_master/html/kk.html");
            m_dispatch->addServlet("/lyserver/ly", urls[0]);

            urls[1] = std::make_shared<URLServlet>("urlschool");
            urls[1]->setURL("/home/ly/lyserver_fiber/html/school.html");
            m_dispatch->addServlet("/lyserver/ly/school", urls[1]);

            urls[2] = std::make_shared<URLServlet>("urlyy");
            urls[2]->setURL("/home/ly/lyserver_fiber/html/yy.html");
            m_dispatch->addServlet("/lyserver/ly/yy", urls[2]);

            m_dispatch->addServlet("/lyserver/xx", [](HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session){
                rsp->setBody("1234567890");
                return 0; 
            });
            m_dispatch->addGlobServlet("/lyserver/*", [](HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session){
                rsp->setBody(req->toString());
                return 0; 
            });
        }

        void HttpServer::setName(const std::string &v)
        {
            TcpServer::setName(v);
            m_dispatch->setDefault(std::make_shared<NotFoundServlet>(v));
        }

        void HttpServer::handleClient(Socket::ptr client)
        {
            // LY_LOG_DEBUG(g_logger) << "handleClient " << *client;
            HttpSession::ptr session(new HttpSession(client));
            do
            {
                auto req = session->recvRequest();
                if (!req)
                {
                    LY_LOG_DEBUG(g_logger) << "recv http request fail, errno="
                                              << errno << " errstr=" << strerror(errno)
                                              << " cliet:" << *client << " keep_alive=" << m_isKeepalive;
                    break;
                }

                HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepalive));
                rsp->setHeader("Server", getName());
                m_dispatch->handle(req, rsp, session);
                session->sendResponse(rsp);

                if (!m_isKeepalive || req->isClose())
                {
                    break;
                }
            } while (true);
            session->close();
        }

    }
}
