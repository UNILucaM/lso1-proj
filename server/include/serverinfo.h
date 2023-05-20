#include <sys/socket.h>


typedef struct serverinfo{
	struct sockaddr_in server_address;
	int server_fd;
} serverinfo;


struct serverinfo* create_server(int, int);
