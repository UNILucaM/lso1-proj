#include "httphelper.h"
#include "bstnode.h"
#include "linkedlist.h"
#include "config.h"

#include <stdbool.h>
#include <pthread.h>

#define REGISTERSTATEMENT "INSERT IN Account VALUES($1, $2);"
typedef struct handleconnectioninput{
	int fd;
	bstnode *routeroot;
	serverconfig *serverConfig;
} handleconnectioninput;

typedef struct sharedthreadvariables{
	int fd;
	linkedlistnode *activeThreads;
	pthread_mutex_t fdMutex;
	pthread_mutex_t activeThreadsMutex;
} sharedthreadvariables;

typedef struct handlerequestinput{
	char *body;
	supportedmethod method;
	int contentLength;
	bstnode *arguments;
	bstnode *headers;
	pthread_t *tid;
	sharedthreadvariables *stv;
	serverconfig *serverConfig;
} handlerequestinput;

typedef struct handle100continueinput{
	pthread_t *tid;
	sharedthreadvariables *stv;
} handle100continueinput;

void *thread_handle_connection_routine(void*);
void *thread_register_routine(void*);
void *thread_login_routine(void*);
void *thread_products_routine(void*);
void *thread_products_purchase_routine(void*);
void *thread_send_100_continue(void*);

void free_handlerequestinput(handlerequestinput*);
void free_handle100continueinput(handle100continueinput*);
void free_bstargs(bstnode*);
void free_bstheaders(bstnode*);
void free_sharedthreadvariables(sharedthreadvariables*);

bool add_activethread(sharedthreadvariables*, pthread_t*);
void remove_activethread(sharedthreadvariables*, pthread_t*);

bool start_thread(void *(*)(void*), void*, sharedthreadvariables*, pthread_t*);
void end_self_thread(handlerequestinput*, sharedthreadvariables*, pthread_t*);
void end_self_thread_100continue(handle100continueinput*, sharedthreadvariables*, pthread_t*);
