#include "Tcpserver.h"
#include "log.h"
#include "iomanager.h"
#include "bytearray.h"
#include "address.h"

static lyserver::Logger::ptr g_logger = LY_LOG_ROOT();

class EchoServer : public lyserver::TcpServer {
public:
    EchoServer(int type);
    void handleClient(lyserver::Socket::ptr client);

private:
    int m_type = 0;
};

EchoServer::EchoServer(int type)
    :m_type(type) {
}

void EchoServer::handleClient(lyserver::Socket::ptr client) {
    LY_LOG_INFO(g_logger) << "handleClient " << *client;   
    lyserver::ByteArray::ptr ba(new lyserver::ByteArray);
    while(true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int rt = client->recv(&iovs[0], iovs.size());
        if(rt == 0) {
            LY_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if(rt < 0) {
            LY_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt);
        ba->setPosition(0);
        //LY_LOG_INFO(g_logger) << "recv rt=" << rt << " data=" << std::string((char*)iovs[0].iov_base, rt);
        if(m_type == 1) {//text 
            std::cout << ba->toString();// << std::endl;
        } else {
            std::cout << ba->toHexString();// << std::endl;
        }
        std::cout.flush();
    }
}

int type = 1;

void run() {
    LY_LOG_INFO(g_logger) << "server type=" << type;
    EchoServer::ptr es(new EchoServer(type));
    auto addr = lyserver::Address::LookupAny("0.0.0.0:8020");
    while(!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}

int main(int argc, char** argv) {
    lyserver::IOManager iom(1);
    
    iom.schedule(run);
    return 0;
}