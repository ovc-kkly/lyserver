#include "http_server.h"
#include "log.h"

static lyserver::Logger::ptr g_logger = LY_LOG_ROOT();

void run() {
    g_logger->setLevel(lyserver::LogLevel::INFO);

    lyserver::http::HttpServer::ptr server(new lyserver::http::HttpServer(true));
    lyserver::Address::ptr addr = lyserver::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    
    server->start();
}

int main(int argc, char** argv) {
    lyserver::IOManager iom(1, true, "main");
    iom.schedule(run);
    return 0;
}
