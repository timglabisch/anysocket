#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<stdio.h>

#include "tcpserv.h"

int main() {
    anysock_tcpserv_args args;
    args.port = 12347;

	anysock_tcpserv_binding* binding = tcpserv_listen(args);
	tcpserv_handelConnections(binding);
}
