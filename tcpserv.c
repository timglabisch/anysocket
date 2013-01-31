#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <error.h>

#include "tcpserv.h"
#include <sys/epoll.h>
#include <sys/types.h>


anysock_tcpserv_binding* tcpserv_listen(anysock_tcpserv_args args) {
    int resource_socket;

    resource_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    if(resource_socket < 0) {
     printf("%s", "cant create socket");
     exit(0);
    }

    int optval = 1;
    if(setsockopt(resource_socket, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) != 0) {
     printf("%s", "could not set reuse socket");
     exit(0);
    }

    struct sockaddr_in resource_socketaddr;

    memset(&resource_socketaddr, 0, sizeof(resource_socketaddr));

    resource_socketaddr.sin_family = AF_INET;
    resource_socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    resource_socketaddr.sin_port = htons((unsigned short)args.port);

    if(bind(resource_socket, (struct sockaddr*) &resource_socketaddr, sizeof(resource_socketaddr)) != 0) {
     printf("%s", "unable to bind socket");
     exit(0);
    }

    if(listen(resource_socket, 1000) != 0) {
     printf("%s", "unable to listen to socket");
     exit(0);
    }



    anysock_tcpserv_binding* res = (anysock_tcpserv_binding*) malloc(sizeof(anysock_tcpserv_binding));
    memset(res, 0, sizeof(anysock_tcpserv_binding));

    res->port = args.port;
    res->sockaddr = resource_socketaddr;
    res->fd = resource_socket;

    return res;
}

/**
* make sure that there is a valid epoll
**/
void tcpserv_initEpollIfNecessary(anysock_tcpserv_binding* binding) {
    if(binding->ePollFd)
        return;

    //if(binding->ePollEvent == NULL)
    //    return;

	int connectionPollFd = epoll_create1(0);
	struct epoll_event connectionPollEvent;

	connectionPollEvent.data.fd = binding->fd;
	connectionPollEvent.events = EPOLLIN | EPOLLET;

	if(epoll_ctl(connectionPollFd, EPOLL_CTL_ADD, binding->fd, &connectionPollEvent) != 0) {
		printf("cant register epoll for new connection\n");
		exit(0);
	}

	binding->ePollEvent = connectionPollEvent;
	binding->ePollFd = connectionPollFd;
}

int tcpserv_getEPollFd(anysock_tcpserv_binding* binding) {
    tcpserv_initEpollIfNecessary(binding);
    return binding->ePollFd;
}

struct epoll_event* tcpserv_getEPollEvent(anysock_tcpserv_binding* binding) {
    tcpserv_initEpollIfNecessary(binding);
    return &binding->ePollEvent;
}

/* TODO: later should return client , ...*/
void tcpserv_handelNewConnection(anysock_tcpserv_binding* binding, struct epoll_event* epollEvent) {
    printf("got connection %d \n", epollEvent->data.fd);

    socklen_t sockaddrLen = (socklen_t)sizeof(binding->sockaddr);
    int clientSocketFd = accept4(binding->fd, (struct sockaddr*) &(binding->sockaddr), &sockaddrLen, SOCK_NONBLOCK);
    if(clientSocketFd < 0) {
        printf("%s\n","cant accept client");
        close(epollEvent->data.fd);
        return;
    }


    printf("client %d accepted\n", clientSocketFd);

    struct epoll_event clientPollEvent;
    clientPollEvent.data.fd  = clientSocketFd;
    clientPollEvent.events = EPOLLIN | EPOLLET | EPOLLERR | EPOLLHUP | EPOLLRDHUP;

    if(epoll_ctl(tcpserv_getEPollFd(binding), EPOLL_CTL_ADD, clientPollEvent.data.fd, &clientPollEvent) != 0) {
        printf("%s\n","cant register events for new client");
        close(epollEvent->data.fd);
        return;
    }

    printf("new client %d registered epoll events ...\n", epollEvent->data.fd);
}

/* TODO: later should return a msg bracket, ...*/
void tcpserv_handelNewMessageCrumb(anysock_tcpserv_binding* binding, struct epoll_event* epollEvent) {
    if((epollEvent->events & EPOLLERR)  || (epollEvent->events & EPOLLHUP) || (epollEvent->events & EPOLLRDHUP)) {
        printf("client %d disconnected\n", epollEvent->data.fd);
        close(epollEvent->data.fd);
        return;
    }

    printf("%s\n", "client send me something ...");

    printf("package start\n");

    while(1) {
        int buffer_size = 512;
        char buf[buffer_size];

        int readedDataLength = read(epollEvent->data.fd, buf, sizeof(buf));
        if(readedDataLength < 0) {
            break;
        } else {
            write(1, buf, readedDataLength);
        }
    }

    printf("package end\n");
}

/**
* Handels all connections
* Blocks!
*/
void tcpserv_handelConnections(anysock_tcpserv_binding* binding) {

    int max_events = 10;
	struct epoll_event socketEvents[max_events];

	while(1) {

		int epollEvents = epoll_wait(tcpserv_getEPollFd(binding), socketEvents, max_events, -1);

		if(epollEvents == -1) {
			printf("%s\n", "epoll wait crashed");
			continue;
		}

		printf("got %d new epoll events\n", epollEvents);

		int i = 0;
		for(; epollEvents > i; i++) {

			if(socketEvents[i].data.fd == binding->fd) {
				// it's a new client ...
                tcpserv_handelNewConnection(binding, &socketEvents[i]);
			} else {
			    // a client sends a msg ...
                tcpserv_handelNewMessageCrumb(binding, &socketEvents[i]);
			}
		}
	}
}

