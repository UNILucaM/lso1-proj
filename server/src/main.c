#define _XOPEN_SOURCE
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
#include <sys/time.h>
#include <limits.h>

#include "errorhandling.h"
#include "mlog.h"
#include "serverinfo.h"
#include "bstnode.h"
#include "threadroutines.h"
#include "httphelper.h"
#include "config.h"


#define SERVERCONFIGFILENAME "config.cfg"
#define READ_TIMEOUT_S 60
#define READ_TIMEOUT_MS 0
#define WRITE_TIMEOUT_S 60
#define WRITE_TIMEOUT_MS 0
#define QUEUE_LENGTH 100
#define PORT 8080

serverinfo *server = NULL;
bstnode *routeroot = NULL;
serverconfig *serverConfig = NULL;

bstnode *init_routes(){
	return init_bst(
		mkroute("/register", &thread_register_routine, true, POST_FLAG),
		mkroute("/login", &thread_login_routine, true, POST_FLAG),
		mkroute("/products", &thread_products_routine, false, GET_FLAG),
		mkroute("/products/purchase", &thread_products_purchase_routine, true, POST_FLAG),
		mkroute("/images", &thread_images_routine, false, GET_FLAG),
		NULL);
}

int main(int argc, char **argv){
	char *fileName = NULL;
	int c;
        while ((c = getopt(argc, argv, "f:")) != -1){
        	switch (c){
        		//filename opt
        		case 'f':
				if (optarg != NULL){
					fileName = malloc(sizeof(char)*(strlen(optarg)+1));
					if (fileName != NULL) strcpy(fileName, optarg);
				}
        			break;
        	}	
        }	
	serverConfig = load_serverconfig_from_file
		(((fileName == NULL) ? SERVERCONFIGFILENAME : fileName));
	if (serverConfig == NULL){
		if (fileName != NULL) free(fileName);
		fatal("Unexpected: cannot load config");
	}
	if (serverConfig->dbName == NULL ||
		serverConfig->dbUsername == NULL ||
		serverConfig->dbPassword == NULL ||
		serverConfig->dbAddr == NULL)
			fatal("Could not read one or more necessary config parameters.");
	if (fileName != NULL) free(fileName);
	int port = (serverConfig->port == PORT_UNSPECIFIED) ? PORT : serverConfig->port;
	server = create_server(port, QUEUE_LENGTH);
	if (server == NULL){
		free_serverconfig(serverConfig);
		fatal("Unexpected: server is null");
	}
	routeroot = init_routes();
	if (routeroot == NULL) fatal("Unexpected: routeroot is null");
	pthread_t tid;
	handleconnectioninput *hci;	
	int threadCreateRetVal;
	struct timeval timeoutW;
	struct timeval timeoutR;
	int connectionNum = 1;
	bool failedToSetOptions = false;
	timeoutR.tv_sec = READ_TIMEOUT_S;
	timeoutW.tv_sec = WRITE_TIMEOUT_S;
	timeoutR.tv_usec = READ_TIMEOUT_MS;
	timeoutW.tv_usec = WRITE_TIMEOUT_MS;
	while(1){
		mlog("SERVER", "Waiting for new connection...", 0);
		int fd = accept(server->server_fd, NULL, NULL);
		if (fd < 0) fatal("Cannot open new connection.");
		mlog("SERVER", "New connection established!", 0);
		hci = malloc(sizeof(handleconnectioninput));
		if (hci == NULL){
			close(fd);
			mlog("SERVER", "Cannot allocate handleconnectioninput.", 0);
			continue;
		}
		hci->fd = fd;
		hci->routeroot = routeroot;
		hci->serverConfig = serverConfig;
		hci->connectionNum = connectionNum;
		if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeoutR, sizeof(struct timeval)) != -1 &&
			setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeoutW, sizeof(struct timeval)) != -1){
			threadCreateRetVal = pthread_create(&tid, NULL, &thread_handle_connection_routine, (void*) hci);
		} else failedToSetOptions = true;
		if (failedToSetOptions || threadCreateRetVal != 0) {
			char *errStr = (failedToSetOptions) ?
				"Failed to set socket options." : strerror(threadCreateRetVal);
			mlog("SERVER", errStr, 0);
			mlog("SERVER", strerror(errno), 0);
			close(fd);
			free(hci);	
		} else {
			pthread_detach(tid);
			connectionNum++;
			if (connectionNum == INT_MAX) connectionNum = 1;
		}
		failedToSetOptions = false;
	}
}
