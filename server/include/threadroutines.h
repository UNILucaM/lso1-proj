#include "httphelper.h"
#include "bstnode.h"
#include "linkedlist.h"
#include "config.h"

#include <stdbool.h>
#include <pthread.h>

//Dimensione standard del buffer dove vengono lette le richieste.
#define BUFSIZE 4096
//Dimensione del buffer dei messaggi d'errore da loggare.
#define ERRBUFSIZE 512
//Dimensione del buffer dove viene letto il metodo della richiesta.
#define METHODBUFSIZE 32
//Dimensione del buffer dove viene letto il path (la route) della richiesta.
#define PATHBUFSIZE 4096
//Dimensione per buffer dove viene letto il valore di un header.
#define VALUEBUFSIZE 512
//Dimensione del buffer dove viene letta la versione di una richiesta.
#define VERSIONBUFSIZE 9 
/*Ritardo che definisce ogni quanto al thread gestione richiesta è permesso di controllare che tutti
i suoi figli siano terminati.*/
#define ACTIVETHREADSCHECKDELAY_MS 500

/*Questo file contiene le strutture e le funzioni di base che sono il nucleo
del programma e dei suoi threads.*/

//Input del thread che gestisce le richieste di una singola connessione.
//fd = file descriptor della connessione
//routeroot = root del bstnode che contiene le route del serverConfig
//serverConfig = configurazione del server (vedere serverconfig.h)
typedef struct handleconnectioninput{
	int fd;
	bstnode *routeroot;
	serverconfig *serverConfig;
} handleconnectioninput;

/*Struct che contiene variabili condivise tra il thread che gestisce 
la connessione e tutti i suoi figli. Abbiamo il file descriptor
della connessione, una lista con tutti i tid dei thread attivi,
e i loro mutex. La presenza della lista è fondamentale, poiché
il thread della richiesta deve sapere quando poter deallocare
le risorse, chiudere il fd in scrittura (in lettura viene chiuso prima) 
e terminare, mentre i thread figli devono poter rimuoversi dalla lista. 
*/
typedef struct sharedthreadvariables{
	int fd;
	linkedlistnode *activeThreads;
	pthread_mutex_t fdMutex;
	pthread_mutex_t activeThreadsMutex;
} sharedthreadvariables;

//Ad ogni thread figlio del thread che gestisce le richieste 
//viene passata questa struct con informazioni sulla richiesta.
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

//Questa è una versione esclusiva al thread che manda il messaggio "100 Continue",
//che ha comportamento statico e quindi non necessita di altre variabili.
typedef struct handle100continueinput{
	pthread_t *tid;
	sharedthreadvariables *stv;
} handle100continueinput;

/*Questo è il thread che gestisce le richieste, effetuandone il parsing,
e reagendo ad esse come deve. Il suo compito è quindi quello di ottenere tutte le informazioni
sulla richiesta e poi creare un thread figlio in base alla route richiesta dal client, o, in caso di errore,
rispondere con un messaggio di errore.
Per informazioni migliori sul parsing delle richieste, è consigliato leggere il codice sorgente.
Per comodità, da ora in poi in questo file ci riferiremo a questo thread come il thread "padre".*/
void *thread_handle_connection_routine(void*);

//Aggiunge/rimuove un thread alla lista dei thread attivi.
bool add_activethread(sharedthreadvariables*, pthread_t*);
void remove_activethread(sharedthreadvariables*, pthread_t*);

/*Avvia un nuovo thread. I primi due argomenti sono la funzione che gestisce la richiesta
e l'input, che deve è void* perché non è detto che il thread accetti handlerequestinput.
Il nuovo thread viene inoltre aggiunto alla lista dei thread attivi.*/
bool start_thread(void *(*)(void*), void*, sharedthreadvariables*, pthread_t*);
//Funzioni di autoterminazione del thread, che includono la rimozione dalla lista dei thread attivi.
void end_self_thread(handlerequestinput*, sharedthreadvariables*, pthread_t*);
void end_self_thread_100continue(handle100continueinput*, sharedthreadvariables*, pthread_t*);

/*Cerca di scrivere la risposta sul fd della connessione.
Parametri: tag da usare nei log, sharedthreadvariables, codice risposta,
header, body, shouldCloseFileDescriptor, lunghezza body.
Nessun thread figlio setta a 1 shouldCloseFileDescriptor,
in quanto essa è responsabilità del thread padre.*/
void attempt_response(char*, sharedthreadvariables*, responsecode, char*, char*, bool, int);
/*Come attempt_response, ma con alcuni parametri rimossi, 
poiché le nostre risposte di errore dal thread padre includono solo un responsecode
e l'header di chiusura connessione.
Poiché questa funzione chiude il fd, questa funzione viene chiamata solo dal padre.*/
void attempt_error_response(char*, sharedthreadvariables*, responsecode);

/*Questi sono i thread che vengono creati dal thread che gestisce la richiesta
in base alla route richiesta dal client,
ognuno ovviamente legato ad una specifica funzione.
Generalmente, i thread hanno questa struttura:
1. Il thread controlla il suo input.
2. Il thread svolge le sue funzioni, usando le informazioni
fornitegli dalla struttura handlerequestinput.
3. Il thread libera le sue risorse e chiama attempt_response (o attempt_error_response)
4. Il thread chiama end_self_thread (o end_self_thread_100continue, nel caso di thread_send100_continue)*/
void *thread_register_routine(void*);
void *thread_login_routine(void*);
void *thread_products_routine(void*);
void *thread_products_purchase_routine(void*);
void *thread_images_routine(void*);
void *thread_send_100_continue(void*);

void free_handlerequestinput(handlerequestinput*);
void free_handle100continueinput(handle100continueinput*);
void free_bstheaders(bstnode*);
void free_sharedthreadvariables(sharedthreadvariables*);

