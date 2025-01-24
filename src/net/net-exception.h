#ifndef __NET_EXCEPTION_H__
#define __NET_EXCEPTION_H__

typedef void (*handle)(int);

void net_exception(handle func);

#endif // !_NET_EXCEPTION_H__
