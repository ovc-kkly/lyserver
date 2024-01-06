#include "reactor.h"
using namespace lyserver;
int main(int argc, const char *argv[])
{
    Epoll_reactor::ptr e(new Epoll_reactor);
    e->initlistensocket();
    e->server_run();
    return 0;
}