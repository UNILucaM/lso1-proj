#include <sys/socket.h>
#include <stdlib.h>

#include "serverinfo.h"
#include "errorhandling.h"
#include "mlog.h"

serverinfo *create_server(int port, int listenqueuesize){
	serverinfo *si = malloc(sizeof(serverinfo));
	if (si == NULL) fatal("Cannot allocate serverinfo.");
	si->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (si->server_fd < 0) fatal("Cannot create socket.");
	int reuse = 1;
	if (setsockopt(si->server_fd,
		SOL_SOCKET,
		SO_REUSEADDR,
		(const char*) &reuse,
		sizeof(reuse)) < 0)
		fatal ("Cannot set SO_REUSEADDR option.");
	#ifdef SO_REUSEPORT
	if (setsockopt(si->server_fd,
		SOL_SOCKET,
		SO_REUSEPORT,
		(const char*) &reuse,
		sizeof(reuse)) < 0)
		fatal("Cannot set SO_REUSEPORT option.");
	#endif
	si->server_address.sin_family = AF_INET;
	si->server_address.sin_port = htons(port);
	si->server_address.sin_addr.s_addr = INADDR_ANY;
	int bindresult = bind(si->server_fd, 
		(struct sockaddr*) (&(si->server_address)), 
		sizeof((si->server_address)));
	if (bindresult < 0) fatal("Could not bind.");
	listen(si->server_fd, listenqueuesize);
	mlog("SERVER", "Created server successfully.", 0);
	return si;
}
