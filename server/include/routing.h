#include <stdbool.h>
/* Il nostro algoritmo di routing per le richieste HTTP
è basato sulla struttura dati ABR.
I dettagli dell'implementazione sono disponibili in routing.c */


typedef struct routeinfo{
	bool requiresBody;
} routeinfo;

/*La struttura, oltre alla sua chiave e 
a riferimenti ad altri nodi conserva
la funzione da eseguire relativa al path.
e se richiede parametri aggiuntivi (vedi routeinfo)*/
typedef struct route{
	char* path;
	void (*request_handler)(void*);
	routeinfo *info;
	route* left;
	route* right;
}route;


route* create_route(char*, void (*)(void*), routeinfo*);
route* add_route(route*, route*);
route* search(route*, char*);
