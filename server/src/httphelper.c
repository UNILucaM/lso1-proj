#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>   

#include "httphelper.h"
#include "bstnode.h"
#include "strutil.h"

const supportedmethodstrpair supportedmethodconversiontable[] = {
	{GET, "GET"},
	{POST, "POST"},
	{PUT, "PUT"}
};

const char* responsecodestrings[] =
	{"100 Continue", 
	"200 OK", 
	"400 Bad Request", 
	"404 Not Found", 
	"501 Not Implemented", 
	"503 Service Unavailable", 
	""};

char *get_header_field_value(char *input, char *headerFieldName, bool *isOutOfMemory){
	
	char *str = strstr(input, headerFieldName);
	if (str == NULL) return NULL;
	char *copy = malloc(strlen(str)+1);
	if (copy == NULL){
		*isOutOfMemory = true;
		return NULL;
	}
	strcpy(copy, str);
	strtok(copy, " ");
	char* token = strtok(copy, "\r\n");
	char* returntoken = malloc(strlen(token)+1);
	if (returntoken == NULL){
		*isOutOfMemory = true;
		return NULL;
	}
	strcpy(returntoken, token);
	free(copy);
	return returntoken;	
}

int convert_string_to_supportedmethod_enum(char* str){
	int i;
	int n = sizeof(supportedmethodconversiontable) / 
		sizeof(supportedmethodconversiontable[0]);
	for (i = 0; i < n; i++){
		if (strcmp(supportedmethodconversiontable[i].str,
			str) == 0)
			return supportedmethodconversiontable[i].value;
	}
	return UNSUPPORTED;
}

char *find_newline(char *ptr, int n){
	int i = 0;
	for (i; i+1 < n; i++){
		if (ptr[i] == '\r' && ptr[i+1] == '\n') return ptr+2;
	}
	return NULL;
}

const char *get_response_code_string(responsecode code){
	return responsecodestrings[code];
}

bstnode *mkroute(char* path, void *(*request_handler)(void*), bool requiresBody){
    routeinfo *routeinfo = malloc(sizeof(routeinfo));
	if (routeinfo == NULL) return NULL;
    routeinfo->request_handler = request_handler;
    routeinfo->requiresBody = requiresBody;
    bstnode *route = create_bstnode(path, (void*) routeinfo);
    return route;
}

//Algoritmo:
//1. Trova la prossima occorenza di '=' nella stringa
//	1a. Se esiste, la stringa deve non terminare, e:
//		1a.a Se non esiste una prossima occorennza di '&', non deve esistere nemmeno
//		una prossima occorrenza di '='
//		1a.b Se invece esiste, l'occorenza deve venire DOPO '=', non essere il carattere
//		direttamente successivo a '=' e deve esistere un'altra occorrenza di '='
//	1b. Se non esiste, la stringa è valida a patto che non esista nemmeno 
//	un'altra occorrenza di '&'
//2. Se non esiste, la stringa è valida se non esiste nemmeno
//una prossima occorrenza di '&' (altrimenti è invalida).
bool isValidArgStr(char *args){
	char *ptr;
	char *ampPtr;
	while(1){
		ptr = args;
		if ((ptr = strchr(ptr, '=')) != NULL)
		{
			ampPtr = strchr(args, '&');
			if (ampPtr == NULL){
			     if ((strchr(ptr+1, '=')) != NULL) return false;
			     else return true;
			}
			else if (ampPtr != NULL){
			     if (ampPtr < ptr || (ampPtr == ptr+1) 
			        || (strchr(ampPtr, '=') == NULL)) return false;
			}
		} else if ((ampPtr = strchr(args, '&')) != NULL) return false;
		else return true;
		args = ampPtr+1;
	}
}

bstnode *mkargbst(char *args){
	char *ptr = args;
	char *token;
	bstnode *root = NULL;
	bstnode *bstnodePtr;
	while((token = strtok(ptr, "=&")) != NULL){
		bstnodePtr = create_bstnode(token, strtok(NULL, "=&"));
		if (bstnodePtr == NULL) return NULL;
		root = add_bstnode(root, bstnodePtr);
		if (root == NULL) return NULL;
		//Fai in modo che le prossime chiamate del while
		//a strtok abbiano come primo parametro NULL
		ptr = NULL;
	}
	return root;
}

int write_response(int fd, responsecode responsecode, 
	char *header, char *body, bool shouldCloseFileDescriptor){
	int tlen, len;
	char *response = form_response(responsecode, header, body, &tlen);
	len = tlen;
	int writtenBytes = 0, totalWrittenBytes = 0;
	while (len > 0){
		writtenBytes = write(fd, response + totalWrittenBytes, len);
		if (writtenBytes < 0) return writtenBytes;
		len -= writtenBytes;
		totalWrittenBytes += totalWrittenBytes;
	}
	if (shouldCloseFileDescriptor) close(fd);
	return tlen;
}

//L'output non è terminato con '\0'
char *form_response(responsecode responsecode, 
	char *header, char *body, int *ilen){
	char *response;
	char *ptr;
	const char *responsecodestring = get_response_code_string(responsecode);
	char c;
	int len = strlen(HTTPVER) + 1 + strlen(responsecodestring) 
		+ strlen(HTTPEOL) 
		+ (header == NULL) ? 0 : strlen(header)
		+ (body == NULL) ? 0 : strlen(body);
	response = malloc(sizeof(char)*len);
	if (response == NULL) return NULL;
	strcpy(response, HTTPVER);
	ptr = response;
	ptr = chainstrcat(ptr, " ");
	ptr = chainstrcat(ptr, (char*) responsecodestring);
	ptr = chainstrcat(ptr, HTTPEOL);
	ptr = chainstrcat(ptr, header);
	//Evita di copiare '\0'
	while((c = *body)){
		*ptr++ = c;
		body++;
	}
	if (ilen != NULL) *ilen = len;
	return response;
}

