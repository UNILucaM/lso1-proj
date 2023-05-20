#include <sys/socket.h>
#include "serverinfo.h"
#include "errorhandling.h"
#include "log.h"

struct serverinfo* create_server(int port, int listenqueuesize){
	struct serverinfo* serverinfo = malloc(struct(serverinfo));
	if (serverinfo == NULL) fatal("Cannot allocate serverinfo.");
	serverinfo->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverinfo->server_fd < 0) fatal("Cannot create socket.");
	serverinfo->server_address.sin_family = AF_INET;
	serverinfo->server_address.sin_family = htons(port);
	serverinfo->server_address.sin_addr.s_addr = INADDR_ANY;
	int bindresult = bind(server, 
		(struct sockaddr*) &(serverinfo->server_address), 
		sizeof(serverinfo->server_address);
	if (bindresult < 0) fatal("Could not bind.");
	listen(serverinfo->server_fd, listenqueuesize);
	log("SERVER", "Created server successfully.");
}
