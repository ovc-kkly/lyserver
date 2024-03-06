#include "daemon.h"
#include "log.h"
using namespace lyserver;
static lyserver::Logger::ptr g_logger = LY_LOG_NAME("business");
int main(int argc, const char *argv[])
{
    lyserver::daemon::ptr dp(new lyserver::daemon);
    if(argc  == 1){
        dp->init("./", "ly_server");
        dp->work();
    }
    else if (argc == 2)
    {
        LY_LOG_ERROR(g_logger) << "必须有可执行文件名";
    }else if(argc == 3){
        dp->init(argv[1], argv[2]);
        dp->work();
    }else if(argc  == 4){
        dp->init(argv[1], argv[2], argv[3]);
        dp->work();
    }

    return 0;
}