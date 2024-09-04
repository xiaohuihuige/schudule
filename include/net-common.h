#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#define NET_SUCCESS 0
#define NET_FAIL    (-1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <stddef.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "log.h"
#include "net-bitstream.h"
#include "kernel-list.h"
#include "net-timestamp.h"
#include "cJSON.h"
#include "net-tcpsocket.h"
#include "ring-buffer.h"

#define net_free(obj) {if (obj) {free(obj);obj = NULL;}}
#define gettid() syscall(__NR_gettid)

#ifdef __cplusplus
}
#endif
#endif // !__COMMON_H__
