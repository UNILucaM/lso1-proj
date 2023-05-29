#include <sys/socket.h>
#include "serverinfo.h"
#include "errorhandling.h"
#include "log.h"

serverinfo* create_server(int port, int listenqueuesize){
	serverinfo* serverinfo = malloc(sizeof(serverinfo));
	if (serverinfo == NULL) fatal("Cannot allocate serverinfo.");
	serverinfo->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverinfo->server_fd < 0) fatal("Cannot create socket.");
	int reuse = 1;
	if (setsockopt(serverinfo->server_fd,
		SOL_SOCKET,
		SO_REUSEADDR,
		(const char*) &reuse,
		sizeof(reuse)) < 0)
		fatal ("Cannot set SO_REUSEADDR option.");
	#ifdef SO_REUSEPORT
	if (setsockopt(serverinfo->server_fd,
		SOL_SOCKET,
		SO_REUSEPORT,
		(const char*) &reuse,
		sizeof(reuse)) < 0)
		fatal("Cannot set SO_REUSEPORT option.");
	#endif
	serverinfo->server_address.sin_family = AF_INET;
	serverinfo->server_address.sin_family = htons(port);
	serverinfo->server_address.sin_addr.s_addr = INADDR_ANY;
	int bindresult = bind(server, 
		(struct sockaddr*) &(serverinfo->server_address), 
		sizeof(serverinfo->server_address);
	if (bindresult < 0) fatal("Could not bind.");
	listen(serverinfo->server_fd, listenqueuesize);
	mlog("SERVER", "Created server successfully.");
}
