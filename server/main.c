#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/un.h>

#include "errorhandling.h"
#include "log.h"
#include "serverinfo.h"
#include "bstnode.h"
#include "threadroutines.h"
#include "httphelper.h"
#include "config.h"

#define QUEUE_LENGTH 5
#define PORT 5200

server *server = NULL;
bstnode *routeroot = NULL;
char *requestbuf[BUFSIZE];

bstnode *init_routes(){
	return init_bst(
		mkroute("/register", &thread_register_routine, true),
		mkroute("/login", &thread_login_routine, true),
		mkroute("/products", &thread_products_routine, false),
		mkroute("/products/purchase", &thread_products_purchase_routine, true)),
		NULL);
}

int main(){
	server = create_server(PORT, QUEUE_LENGTH);
	routeroot = init_routes();
	pthread_t tid;
	handleconnectioninput *hci;
	int threadCreateRetVal;
	while(1){
		log("SERVER", "Waiting for new connection...");
		int fd = accept(server->server_fd, NULL, NULL);
		if (fd < 0) fatal("Cannot open new connection.");
		log("SERVER", "New connection established!");
		hci = malloc(sizeof(handleconnectioninput);
		hci->fd = fd;
		hci->routeroot = routeroot;
		threadCreateRetVal = pthread_create(&tid, NULL, &thread_handle_connection_routine, (void*) hci);
		if (threadCreateRetVal != 0){
			log("SERVER", strerror(createRetValue));
			close(fd);
			free(hci);
		} else pthread_detach(tid);
	}
}

