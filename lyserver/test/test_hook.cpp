#include "hook.h"
#include "iomanager.h"
#include "log.h"

lyserver::Logger::ptr g_logger = LY_LOG_ROOT();
void test_sleep()
{
    lyserver::IOManager iom(1);
    iom.schedule([](){
        sleep(2);
        LY_LOG_INFO(g_logger) << "sleep 2"; 
    });

    iom.schedule([](){
        sleep(3);
        LY_LOG_INFO(g_logger) << "sleep 3"; 
    });
    LY_LOG_INFO(g_logger) << "test_sleep";
}
void test_sock()
{
}
int main()
{
    LY_LOG_INFO(g_logger) << "start";
    lyserver::IOManager iom;
    iom.schedule(test_sleep);
    return 0;
}