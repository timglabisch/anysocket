#ifndef ANYSOCK_CLIENT
#define ANYSOCK_CLIENT

#include<sys/epoll.h>


typedef struct {
    struct epoll_event epoll;
} anysock_client;

#endif
