#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<stdio.h>

#include<sys/epoll.h>
#include "client.h"

void anysock_client_free(anysock_client* client) {
    free(client);
}
