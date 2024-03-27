#include "Tcpserver.h"
#include "config.h"

using namespace lyserver;
static lyserver::Logger::ptr g_logger = LY_LOG_NAME("root1");
static lyserver::Logger::ptr system_logger = LY_LOG_NAME("system");


int main(int argc, const char *argv[])
{

    
    LY_LOG_ERROR(g_logger) << "main";
    return 0;
}