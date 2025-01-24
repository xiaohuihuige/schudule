#include "net-schedule.h"

#define EPOLL_MAX_NEVENT (4096)
#define MAX_SECONDS_IN_MSEC_LONG \
    (((LONG_MAX)-999) / 1000)

struct epoll_ctx
{
    int epfd;
    int nevents;
    struct epoll_event *events;
};

static void *epoll_init(void)
{
    int fd = epoll_create(1);
    if (fd == NET_FAIL)
    {
        ERR("epoll_create errno=%d %s", errno, strerror(errno));
        return NULL;
    }

    struct epoll_ctx *ec = (struct epoll_ctx *)calloc(1, sizeof(struct epoll_ctx));
    if (!ec)
    {
        ERR("malloc epoll_ctx failed!");
        return NULL;
    }

    ec->epfd = fd;
    ec->nevents = EPOLL_MAX_NEVENT;
    ec->events = (struct epoll_event *)calloc(EPOLL_MAX_NEVENT, sizeof(struct epoll_event));
    if (!ec->events)
    {
        ERR("malloc epoll_event failed!");
        return NULL;
    }
    return ec;
}

static void epoll_deinit(void *ctx)
{
    if (!ctx)
        return;

    struct epoll_ctx *ec = (struct epoll_ctx *)ctx;
    if (!ctx)
    {
        return;
    }
    
    net_free(ec->events);
    net_free(ec);
}

static int epoll_add(void *ctx, ev_ptr event)
{
    struct epoll_ctx *ec = (struct epoll_ctx *)ctx;
    struct epoll_event epev;
    memset(&epev, 0, sizeof(epev));

    if (event->flags & EVENT_READ)
        epev.events |= EPOLLIN;

    if (event->flags & EVENT_WRITE)
        epev.events |= EPOLLOUT;

    if (event->flags & EVENT_ERROR)
        epev.events |= EPOLLERR;

    if (0 == (event->flags & EVENT_PERSIST))
    {
        epev.events |= EPOLLONESHOT;
    }
    else
    {
        epev.events &= ~EPOLLONESHOT;
    }

    epev.events |= EPOLLET;
    epev.data.ptr = (void *)event;

    if (NET_FAIL == epoll_ctl(ec->epfd, EPOLL_CTL_ADD, event->evfd, &epev))
    {
        ERR("EPOLL_CTL_ADD failed: %d %s", errno, strerror(errno));
        return NET_FAIL;
    }
    return NET_SUCCESS;
}

static int epoll_del(void *ctx, ev_ptr event)
{
    struct epoll_ctx *ec = (struct epoll_ctx *)ctx;
    if (NET_FAIL == epoll_ctl(ec->epfd, EPOLL_CTL_DEL, event->evfd, NULL))
    {
        ERR("EPOLL_CTL_DEL failed: %d %s", errno, strerror(errno));
        return NET_FAIL;
    }
    return NET_SUCCESS;
}

static int epoll_mod(void *ctx, ev_ptr event)
{
    struct epoll_ctx *ec = (struct epoll_ctx *)ctx;
    struct epoll_event epev;
    memset(&epev, 0, sizeof(epev));

    if (event->flags & EVENT_READ)
        epev.events |= EPOLLIN;

    if (event->flags & EVENT_WRITE)
        epev.events |= EPOLLOUT;

    if (event->flags & EVENT_ERROR)
        epev.events |= EPOLLERR;

    if (0 == (event->flags & EVENT_PERSIST))
    {
        epev.events |= EPOLLONESHOT;
    }
    else
    {
        epev.events &= ~EPOLLONESHOT;
    }

    epev.events |= EPOLLET;
    epev.data.ptr = (void *)event;

    if (NET_FAIL == epoll_ctl(ec->epfd, EPOLL_CTL_MOD, event->evfd, &epev))
    {
        ERR("EPOLL_CTL_ADD failed: %d %s", errno, strerror(errno));
        return NET_FAIL;
    }
    return NET_SUCCESS;
}

static int epoll_dispatch(void *ctx, int timeout)
{
    struct epoll_ctx *epop = (struct epoll_ctx *)ctx;
    struct epoll_event *events = epop->events;

    int n = epoll_wait(epop->epfd, events, epop->nevents, timeout);
    if (NET_FAIL == n)
    {
        if (errno != EINTR)
        {
            ERR("epoll_wait failed %d: %s", errno, strerror(errno));
            return NET_FAIL;
        }
        return NET_SUCCESS;
    }

    if (0 == n)
    {
        // ERR("epoll_wait timeout");
        return 0;
    }

    for (int i = 0; i < n; i++)
    {
        int what = events[i].events;
        ev_ptr handle = (ev_ptr )events[i].data.ptr;

        if (what & (EPOLLHUP | EPOLLERR))
        {
        }
        else
        {
            if (what & EPOLLIN)
            {
                if (handle->function)
                    handle->function(handle->evfd, handle->args);
            }
            // if (what & EPOLLOUT)
            //     if (handle->function)
            //         handle->function(handle->evfd, handle->args);
            // if (what & EPOLLRDHUP)
            //     if (handle->function)
            //         handle->function(handle->evfd, handle->args);
        }
    }
    return NET_SUCCESS;
}

event_ops epollops = {
    .init = epoll_init,
    .deinit = epoll_deinit,
    .add = epoll_add,
    .del = epoll_del,
    .mod = epoll_mod,
    .dispatch = epoll_dispatch,
};
