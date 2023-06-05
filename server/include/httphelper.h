#include <stdbool.h>
#include <stdint.h>

#include "bstnode.h"

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

#ifndef HTTPHELPER_H
#define HTTPHELPER_H
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

typedef enum{
	CONTINUE,
	OK,
	BAD_REQUEST,
	NOT_FOUND,
	METHOD_NOT_ALLOWED,
	NOT_IMPLEMENTED,
	INTERNAL_SERVER_ERROR,
	SERVICE_UNAVAILABLE,
	UNDEFINED
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

extern const char *responsecodestrings[]; 
#endif

char *get_header_field_value(char*, char*, bool*);
int convert_string_to_supportedmethod_enum(char*);
char *find_newline(char*, int);
bool isValidArgStr(char*);
const char *get_response_code_string(responsecode);
int8_t get_flag_value_for_method(supportedmethod);
char *form_response(responsecode, char*, char*, int*);
int write_response(int, responsecode, char*, char*, bool);
bstnode *mkroute(char*, void*(*)(void*), bool, int8_t);
void free_bstroute(bstnode*);
//NOTA: L'implementazione fornita distrugge la stringa originale.
bstnode *mkargbst(char*);
