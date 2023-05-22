/* Il nostro algoritmo di routing per le richieste HTTP
è basato sulla struttura dati ABR.
I dettagli dell'implementazione sono disponibili in routing.c */

typedef struct route{
	char* path;
	void (*request_handler)(void*);
	route* left;
	route* right;
}route;

route* create_route(char*, void (*)(void*));
route* add_route(route*, route*);
route* search(route*, char*);
