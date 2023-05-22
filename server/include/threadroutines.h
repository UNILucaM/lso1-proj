typedef struct handleconnectioninput{
	int fd;
	route* routeroot;
} handleconnectioninput;

typedef struct handlerequestinput{
	char* body;
	int contentLength;
	char* arguments;
} handlerequestinput;

void *thread_handle_connection_routine(void*);
void *thread_register_routine(void*);
void *thread_login_routine(void*);
void *thread_products_routine(void*);
void *thread_products_purchase_routine(void*);
