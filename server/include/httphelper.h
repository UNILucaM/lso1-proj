#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include "bstnode.h"

#ifndef HTTPHELPER_H
#define HTTPHELPER_H

//Versione di HTTP del server
#define HTTPVER "HTTP/1.1"
//End-Of-Line standard di HTTP
#define HTTPEOL "\r\n"
//Dimensione massima dei nomi degli header
#define HTTPMAXHEADERNAMELENGTH 40

/*Flag che identificano i vari metodi.
Sono usate per permettere ad ogni route di descrivere
i metodi da loro accettati attraverso una mask da 8 bit,
uno per ogni metodo (eccetto HEAD, che usa lo stesso di GET).*/

#define GET_FLAG 0x1
#define POST_FLAG 0x2
#define PUT_FLAG 0x4
#define HEAD_FLAG 0x1

//Usati per header_set_connection
#define CLOSE 0
#define KEEP_ALIVE 1
#define BAD_ARGS -2
#define HEADER_NOT_FOUND -1

/*La specifica HTTP 1.1 richiede (purtroppo) che siano supportati 
tutti e tre questi formati per le date.
Una particolare accortezza che dobbiamo adottare 
è come trattiamo il secondo formato:
se l'anno è più di 50 anni nel futuro, 
deve essere considerato un anno del secolo scorso.
(esempio: se siamo nel 2023, gli anni tra 74-99
vengono trattati come 1974-1999).*
Stringhe d'esempio (la prima rispetta il primo formato e così via):
"Fri, 31 Dec 1999 23:59:59 GMT"
"Friday, 31-Dec-75 23:59:59 GMT"
"Fri Dec 31 23:59:59 1999"
*/
#define HTTP_DATE_FORMAT_1 "%a, %d %b %Y %T %Z"
#define HTTP_DATE_FORMAT_2 "%a, %d-%b-%y %T %Z"
#define HTTP_DATE_FORMAT_3 "%a %b %d %T %Y"

typedef enum {
	UNSUPPORTED = -1,
	GET,
	POST,
	PUT,
	HEAD
}supportedmethod;

/*Usata per definire una tavola di conversione per ottenere,
per ogni enum supportedmethod, la stringa corrispondente
e la flag corrispondente (o anche l'enum corrispondente dalla
stringa corrispondente.)*/
typedef struct{
	supportedmethod value;
	char *str;
	int8_t flagValue;
}supportedmethodconversionvalues;

extern const supportedmethodconversionvalues supportmethodconversiontable[];

//Ordinati per numero dello statuscode.
typedef enum{
	CONTINUE, //100
	OK, //200 
	NOT_MODIFIED, //304
	BAD_REQUEST, //400
	UNAUTHORIZED, //401
	NOT_FOUND, //404
	METHOD_NOT_ALLOWED, //405
	REQUEST_TIMEOUT, //408
	PRECONDITION_FAILED, //412
	INTERNAL_SERVER_ERROR, //500
	NOT_IMPLEMENTED, //501
	SERVICE_UNAVAILABLE, //503
	UNDEFINED //PLACEHOLDER
}responsecode;

/*Usato per tenere traccia dello stato della richiesta.
Fondamentale per il processamento di essa.*/
typedef enum{
	PARSING_REQUEST_LINE,
	PARSING_HEADERS,
	PARSING_TRAILERS,
	PARSING_CHUNKED_BODY,
	OBTAINING_BODY,
	RESPONDING,
	DONE
}requeststatus;

/*Definisce le informazioni su una route.
Il request_handler corrisponde alla funzione che 
deve essere eseguita quando viene richiesta questa risorsa.*/
typedef struct routeinfo{
	void *(*request_handler)(void*);
	bool requiresBody;
	int8_t acceptedMethodsMask;
}routeinfo;

//Contiene le stringhe corrispondenti responsecode.
extern const char *responsecodestrings[]; 
#endif

/*Ottieni il valore dell'header. 
Il bool, se specificato, viene usato 
per comunicare se la funziona ha finito la memoria.*/
char *get_header_field_value(char*, char*, bool*);
int convert_string_to_supportedmethod_enum(char*);
char *find_newline(char*, int);
bool isValidArgStr(char*);
const char *get_response_code_string(responsecode);
int8_t get_flag_value_for_method(supportedmethod);
/*Costruisce la stringa che rappresenta la risposta HTTP
identificata dai parametri forniti.*/
char *form_response(responsecode, char*, char*, int*);
int write_response(int, responsecode, char*, char*, bool);
bstnode *mkroute(char*, void*(*)(void*), bool, int8_t);
void free_bstroute(bstnode*);
//NOTA: L'implementazione fornita distrugge la stringa originale.
bstnode *mkargbst(char*);
void free_bstargs(bstnode*);
/*Cerca l'header Connection nel BST specificato.
Se lo trova, ritorna un valore che rappresenta il suo valore.
Altrimenti, HEADER_NOT_FOUND. Inoltre, se è specificata una stringa,
appende l'header corretto ad essa.*/
int header_set_connection(bstnode*, char*);
/*Se la stringa è in una dei 3 formati specificati dalle define
HTTP_DATE_FORMAT_X, restituisce il tempo UTC.
Altrimenti NULL. Il risultato deve essere deallocato.*/
time_t *get_http_time_from_str(char*);
/*Restituisce una stringa rappresentante l'istante corrente
secondo il formato HTTP_DATE_FORMAT_1. La stringa deve essere deallocata.*/
char *get_http_time();
/*Crea un header con valori che di solito vengono inseriti in ogni risposta,
come Connection e Content-Length.
Il valore di ritorno riflette l'header Connection 
(se è vero bisogna chiudere la connessione).*/
bool create_basic_header(char*, bstnode*, int);
