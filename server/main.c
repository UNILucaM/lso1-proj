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
#include "routing.h"
#include "threadroutines.h"
#include "httphelper.h"
#include "config.h"

#define QUEUE_LENGTH 5
#define PORT 5200

server* server = NULL;
route* routeroot = NULL;
char* requestbuf[BUFSIZE];

void *thread_server_child_write_routine(void *arg){
        int server_child_fd = *((int*)arg);
       	char* msg = "Hey client.\n\x04";
       	int sizeofmsg = strlen(msg);
       	int n = 0;
       	int byteswritten = 0;
       	while (byteswritten < sizeofmsg){
       		if ((n = write(server_child_fd, msg, sizeofmsg)) < 0) fatal("Error writing");
       		byteswritten += n;
        }
       	pthread_exit(NULL);
}

route* init_routes(){
	route* root = create_route("/register", &thread_register_routine);
	route* tmp = create_route("/login", &thread_login_routine);
	if (root == NULL || tmp == NULL) return NULL;
	root = add_route(root, tmp);
	tmp = create_route("/products", &thread_products_routine);
	if (tmp == NULL) return NULL;
	root = add_route(root, tmp);
	tmp = create_route("/products/purchase", &thread_products_purchase_routine);
	if (tmp == NULL) return NULL;
	root = add_route(root, tmp);
	return root;
}

void main_loop(){
	
	
	
	
	
		

}

int main(){
	server = create_server(PORT, QUEUE_LENGTH);
	routeroot = init_routes();
	int server_child_fd;
	int pid;
	int tid;
        
        
	
	
	
	
	while(1){
		tid = 0;
		bytecount = 0;
		tmpbytecount = 0;
		isMethodParsed = false;
                isPathParsed = false;
                isContentLengthParsed = false;
		toRead = BUFSIZE;
		log("SERVER", "Waiting for new connection...");
		int fd = accept(server->server_fd, NULL, NULL);
		if (fd < 0) fatal("Cannot open new connection.");
		log("SERVER", "New connection established!");
		while (1){
			tmpbytecount = read(fd, requestbuf, BUFSIZE);
			if (tmpbytecount == 0) //TODO: RETURN 4xx
			else if (!isMethodParsed		
				

		}          			
	}
}

