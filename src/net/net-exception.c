#include "net-common.h"
#include "net-exception.h"

void net_exception(handle func)
{
    signal(SIGSEGV, func);
    signal(SIGINT,  func);
    signal(SIGQUIT, func);
    signal(SIGTERM, func);
    signal(SIGPIPE, SIG_IGN);
}
