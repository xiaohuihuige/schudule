#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif 

#define NET_SUCCESS 0
#define NET_FAIL    (-1)

#include <ifaddrs.h>
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
#include <stdatomic.h>
#include "log.h"
#include "bitstream.h"
#include "kernel_list.h"
#include "timestamp.h"
#include "cJSON.h"
#include "net-tcpsocket.h"
#include "circular_buffer.h"

#define REVC_MTU 8192

#define net_free(obj) {if (obj) {free(obj);obj = NULL;}}
#define gettid() syscall(__NR_gettid)

#define MALLOC(type, size) ({ \
    type *ptr = (type *)malloc(size * sizeof(type)); \
    if (!ptr) { \
        fprintf(stderr, "Memory allocation failed for %zu bytes\n", size * sizeof(type)); \
        exit(EXIT_FAILURE); \
    } \
    ptr; \
})

#define FREE(ptr) do { \
    if (ptr) { \
        free(ptr); \
        ptr = NULL; \
    } \
} while (0)

// 定义互斥锁结构体
typedef struct {
    pthread_cond_t cond;
} Cond;

// 初始化条件信号
#define COND_INIT(m) do { \
    if (pthread_cond_init(&(m)->cond, NULL) != 0) { \
        perror("Cond initialization failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 销毁条件信号
#define COND_DESTROY(m) do { \
    if (pthread_cond_destroy(&(m)->cond) != 0) { \
        perror("Cond destruction failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 发送信号
#define COND_SIGNAL(m) do { \
    if (pthread_cond_signal(&(m)->cond) != 0) { \
        perror("Cond signal failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 等待阻塞条件
#define COND_WAIT(m, l) do { \
    if (pthread_cond_wait(&(m)->cond, &(l)->mutex) != 0) { \
        perror("Cond signal failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 发送全部信号
#define COND_BROADCAST(m) do { \
    if (pthread_cond_broadcast((&(m)->cond)) != 0) { \
        perror("Cond broadcast failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 定义互斥锁结构体
typedef struct {
    pthread_mutex_t mutex;
} Mutex;

// 初始化互斥锁
#define MUTEX_INIT(m) do { \
    if (pthread_mutex_init(&(m)->mutex, NULL) != 0) { \
        perror("Mutex initialization failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 销毁互斥锁
#define MUTEX_DESTROY(m) do { \
    if (pthread_mutex_destroy(&(m)->mutex) != 0) { \
        perror("Mutex destruction failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 加锁
#define MUTEX_LOCK(m) do { \
    if (pthread_mutex_lock(&(m)->mutex) != 0) { \
        perror("Mutex lock failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

// 解锁
#define MUTEX_UNLOCK(m) do { \
    if (pthread_mutex_unlock(&(m)->mutex) != 0) { \
        perror("Mutex unlock failed"); \
        exit(EXIT_FAILURE); \
    } \
} while (0)


#define CREATE_THREAD(thread, function, arg, attr) \
    do { \
        if (pthread_create(&(thread), (attr), (function), (arg)) != 0) { \
            perror("Failed to create thread"); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

#define JOIN_THREAD(thread) \
    do { \
        if (pthread_join((thread), NULL) != 0) { \
            perror("Failed to join thread"); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

#define CALLOC(num, type) \
    ({ \
        type* ptr = (type*)calloc((num), sizeof(type)); \
        if (ptr == NULL) { \
            fprintf(stderr, "Memory allocation failed!\n"); \
            exit(EXIT_FAILURE); \
        } \
        ptr; \
    })

#ifdef __cplusplus
}
#endif
#endif // !__COMMON_H__
