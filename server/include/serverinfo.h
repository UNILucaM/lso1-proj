#include <netinet/ip.h>


typedef struct serverinfo{
	struct sockaddr_in server_address;
	int server_fd;
} serverinfo;


serverinfo *create_server(int, int);
