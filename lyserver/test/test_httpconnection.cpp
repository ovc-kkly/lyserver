#include <iostream>
#include "http_server.h"
#include "http_connection.h"
static lyserver::Logger::ptr g_logger = LY_LOG_ROOT();
void run() {
    lyserver::Address::ptr addr = lyserver::Address::LookupAnyIPAddress("39.100.72.123:80");
    if(!addr) {
        LY_LOG_INFO(g_logger) << "get addr error";
        return;
    }

    lyserver::Socket::ptr sock = lyserver::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    if(!rt) {
        LY_LOG_INFO(g_logger) << "connect " << *addr << " failed";
        return;
    }

    lyserver::http::HttpConnection::ptr conn(new lyserver::http::HttpConnection(sock));
    lyserver::http::HttpRequest::ptr req(new lyserver::http::HttpRequest);
    req->setPath("/blog/");
    req->setHeader("host", "www.sylar.top");
    LY_LOG_INFO(g_logger) << "req:" << std::endl
        << *req;

    conn->sendRequest(req);
    auto rsp = conn->recvResponse();

    if(!rsp) {
        LY_LOG_INFO(g_logger) << "recv response error";
        return;
    }
    LY_LOG_INFO(g_logger) << "rsp:" << std::endl
        << *rsp;

    std::ofstream ofs("rsp.dat");
    ofs << *rsp;

    LY_LOG_INFO(g_logger) << "=========================";
    // std::map<std::string, std::string> headers;
    // headers["host"] = "www.sylar.top";
    auto r = lyserver::http::HttpConnection::DoGet("https://www.baidu.com/", 300);
    LY_LOG_INFO(g_logger) << "result=" << r->result
        << " error=" << r->error
        << " rsp=" << (r->response ? r->response->toString() : "");

    LY_LOG_INFO(g_logger) << "=========================";
    // test_pool();
}
int main()
{
    run();
    return 0;
}