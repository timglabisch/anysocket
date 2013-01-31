#ifndef ANYSOCK_TCPSERV
#define ANYSOCK_TCPSERV

#include<netinet/in.h> // sockaddr_in
#include<sys/epoll.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

/**
* Arguments for listen to a Port
*/
typedef struct {
    int port;
} anysock_tcpserv_args;

/**
* A Port Binding
*/
typedef struct {
    int port;
    int fd;
    struct sockaddr_in sockaddr;
    int connectionsCount;

    /* private */
    int ePollFd;
    struct epoll_event ePollEvent;

} anysock_tcpserv_binding;

union {
    int CANT_CREATE;
    int CANT_REUSE;
    int CANT_BIND;
    int CANT_LISTEN;
} tcpserv_listen_errors;

anysock_tcpserv_binding* tcpserv_listen(anysock_tcpserv_args args);
void tcpserv_handelConnections(anysock_tcpserv_binding* binding);

#endif
