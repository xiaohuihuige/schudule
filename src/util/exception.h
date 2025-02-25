#ifndef __NET_EXCEPTION_H__
#define __NET_EXCEPTION_H__

#include "net-common.h"
#include "exception.h"

typedef void (*handle)(int);

void net_exception(handle func);

static inline void net_exception(handle func)
{
    signal(SIGSEGV, func);
    signal(SIGINT,  func);
    signal(SIGQUIT, func);
    signal(SIGTERM, func);
    signal(SIGPIPE, SIG_IGN);
}

#endif // !_NET_EXCEPTION_H__
