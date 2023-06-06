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
#include "mlog.h"
#include "serverinfo.h"
#include "bstnode.h"
#include "threadroutines.h"
#include "httphelper.h"
#include "config.h"

#define SERVERCONFIGFILENAME "config.cfg"
#define QUEUE_LENGTH 100
#define PORT 8080

serverinfo *server = NULL;
bstnode *routeroot = NULL;
serverconfig *serverConfig = NULL;

bstnode *init_routes(){
	return init_bst(
		mkroute("/register", &thread_register_routine, true, PUT_FLAG),
		mkroute("/login", &thread_login_routine, true, POST_FLAG),
		mkroute("/products", &thread_products_routine, false, GET_FLAG),
		mkroute("/products/purchase", &thread_products_purchase_routine, true, POST_FLAG),
		NULL);
}

int main(int argc, char **argv){
	serverConfig = load_serverconfig_from_file
		(((fileName == NULL) ? SERVERCONFIGFILENAME : fileName));
	if (serverConfig == NULL){
		fatal("Unexpected: cannot load config");
	}
	if (serverConfig->dbName == NULL ||
		serverConfig->dbUsername == NULL ||
		serverConfig->dbPassword == NULL)
			fatal("Could not read one or more necessary config parameters.");
	server = create_server(PORT, QUEUE_LENGTH);
	if (server == NULL){
		free_serverconfig(serverConfig);
		fatal("Unexpected: server is null");
	}
	routeroot = init_routes();
	if (routeroot == NULL) fatal("Unexpected: routeroot is null");
	char *fileName = NULL;
	pthread_t tid;
	handleconnectioninput *hci;
	int c;
	while ((c = getopt(argc, argv, "f::")) != -1){
		switch (c){
			//filename opt
			case 'f':
				fileName = strdup(optarg);
				break;
		}	
	}	
	int threadCreateRetVal;
	while(1){
		mlog("SERVER", "Waiting for new connection...");
		int fd = accept(server->server_fd, NULL, NULL);
		if (fd < 0) fatal("Cannot open new connection.");
		mlog("SERVER", "New connection established!");
		hci = malloc(sizeof(handleconnectioninput));
		if (hci == NULL){
			close(fd);
			mlog("SERVER", "Cannot allocate handleconnectioninput.");
			continue;
		}
		hci->fd = fd;
		hci->routeroot = routeroot;
		hci->serverConfig = serverConfig;
		threadCreateRetVal = pthread_create(&tid, NULL, &thread_handle_connection_routine, (void*) hci);
		if (threadCreateRetVal != 0) {
			mlog("SERVER", strerror(threadCreateRetVal));
			close(fd);
			free(hci);	
		} else pthread_detach(tid);
	}
}
