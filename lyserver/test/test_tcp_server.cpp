#include "Tcpserver.h"
#include "iomanager.h"
#include "log.h"

lyserver::Logger::ptr g_logger = LY_LOG_ROOT();

void run() {
    auto addr = lyserver::Address::LookupAny("0.0.0.0:8033");
    //auto addr2 = lyserver::UnixAddress::ptr(new lyserver::UnixAddress("/tmp/unix_addr"));
    std::vector<lyserver::Address::ptr> addrs;
    addrs.push_back(addr);
    //addrs.push_back(addr2);

    lyserver::TcpServer::ptr tcp_server(new lyserver::TcpServer);
    std::vector<lyserver::Address::ptr> fails;
    while(!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
    
}
int main(int argc, char** argv) {
    lyserver::IOManager iom(2);
    iom.schedule(run);
    return 0;
}
