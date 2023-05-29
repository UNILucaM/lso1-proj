#include "httphelper.h"
#include <stdbool.h>

typedef struct handleconnectioninput{
	int fd;
	route* routeroot;
} handleconnectioninput;

typedef struct handlerequestinput{
	int fd;
	char *body;
	int contentLength;
	bstnode *arguments;
} handlerequestinput;

void *thread_handle_connection_routine(void*);
void *thread_register_routine(void*);
void *thread_login_routine(void*);
void *thread_products_routine(void*);
void *thread_products_purchase_routine(void*);
void *thread_send_100_continue(void*);