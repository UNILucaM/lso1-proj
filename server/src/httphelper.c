#define _XOPEN_SOURCE
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>   
#include <stdio.h>
#include <time.h>
#include <mlog.h>

#include "httphelper.h"
#include "bstnode.h"
#include "strutil.h"

const supportedmethodconversionvalues supportedmethodconversiontable[] = {
	{GET, "GET", GET_FLAG},
	{POST, "POST", POST_FLAG},
	{PUT, "PUT", PUT_FLAG},
	{HEAD, "HEAD", HEAD_FLAG}
};

const char *responsecodestrings[] =
	{"100 Continue", 
	"200 OK",
	"304 Not Modified",
	"400 Bad Request",
	"401 Unauthorized",
	"404 Not Found",
	"405 Method Not Allowed",
	"408 Request Timeout",
	"412 Precondition Failed",
	"500 Internal Server Error", 
	"501 Not Implemented", 
	"503 Service Unavailable", 
	""};

char *get_header_field_value(char *input, char *headerFieldName, bool *isOutOfMemory){
	char *str = strstr(input, headerFieldName);
	if (str == NULL) return NULL;
	char *copy = malloc(sizeof(char)*(strlen(str)+1));
	if (copy == NULL){
		*isOutOfMemory = true;
		return NULL;
	}
	strcpy(copy, str);
	strtok(copy, " ");
	char *token = strtok(copy, "\r\n");
	char *returntoken = malloc(sizeof(char)*(strlen(token)+1));
	if (returntoken == NULL){
		*isOutOfMemory = true;
		return NULL;
	}
	strcpy(returntoken, token);
	free(copy);
	return returntoken;	
}

int convert_string_to_supportedmethod_enum(char *str){
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

const int8_t get_flag_value_for_method(supportedmethod method){
	return supportedmethodconversiontable[method].flagValue; 
}

char *find_newline(char *ptr, int n){
	int i = 0;
	for (i; i+1 < n; i++){
		if (ptr[i] == '\r' && ptr[i+1] == '\n') return ptr+i+2;
	}
	return NULL;
}

const char *get_response_code_string(responsecode code){
	return responsecodestrings[code];
}

bstnode *mkroute(char* path, void *(*request_handler)(void*),
	bool requiresBody, int8_t acceptedMethodsMask){
	routeinfo *ri = malloc(sizeof(routeinfo));
	if (ri == NULL) return NULL;
    	ri->request_handler = request_handler;
	ri->requiresBody = requiresBody;
	ri->acceptedMethodsMask = acceptedMethodsMask;
	bstnode *route = create_bstnode(path, (void*) ri);
	return route;
}

void free_bstroute(bstnode *root){
	if (root == NULL) return;
	free_bstroute(root->left);
	free_bstroute(root->right);
	void *val = root->value;
	routeinfo *routeinfo = val;
	if (routeinfo != NULL) free(routeinfo);
	free(root);
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
	if (args == NULL) return false;
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
	char *tokenName;
	char *tokenValue;
	char *argName;
	char *argValue;
	bstnode *root = NULL;
	bstnode *bstnodePtr;
	while((tokenName = strtok(ptr, "=&")) != NULL){
		tokenValue = strtok(NULL, "=&");
		argName = malloc(sizeof(char)*(1+strlen(tokenName)));
		argValue = malloc(sizeof(char)*(1+strlen(tokenValue)));
		if (argName == NULL || argValue == NULL){
			if (argName != NULL) free(argName);
			if (argValue != NULL) free(argValue);
			return NULL;
		}
		strcpy(argName, tokenName);
		strcpy(argValue, tokenValue);
		bstnodePtr = create_bstnode(argName, argValue);
		if (bstnodePtr == NULL) return NULL;
		root = add_bstnode(root, bstnodePtr);
		if (root == NULL){
			free(bstnodePtr);
			return NULL;
		}
		//Fai in modo che le prossime chiamate del while
		//a strtok abbiano come primo parametro NULL
		ptr = NULL;
	}
	return root;
}

void free_bstargs(bstnode *bstargsroot){
	if (bstargsroot != NULL){
		free_bstargs(bstargsroot->left);
		free_bstargs(bstargsroot->right);
		if (bstargsroot->key != NULL)
			free(bstargsroot->key);
		if (bstargsroot->value != NULL)
			free(bstargsroot->value);
		free(bstargsroot);
	}
}

int write_response(int fd, responsecode responsecode, 
	char *header, char *body, bool shouldCloseFileDescriptor, int unterminatedBodySize){
	int tlen, len;
	char *response = form_response(responsecode, header, body, &tlen, unterminatedBodySize);
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

int header_set_connection(bstnode *headerroot, char *header){
	if (headerroot == NULL) return BAD_ARGS;
	bstnode *connectionHeader = search(headerroot, "Connection");
	if (connectionHeader != NULL){
		if (strcasecmp((char*)(connectionHeader->value), "keep-alive") == 0){
			if (header != NULL) strcat(header, "Connection: keep-alive\r\n");
			return KEEP_ALIVE;
		}
		else {
			if (header != NULL) strcat(header, "Connection: close\r\n");
			return CLOSE;
		}
	} else return HEADER_NOT_FOUND;
}

char *form_response(responsecode responsecode, 
	char *header, char *body, int *ilen, int unterminatedBodySize){
	char *response;
	char *ptr;
	const char *responsecodestring = get_response_code_string(responsecode);
	char c;
	int len = strlen(HTTPVER) + 2  + strlen(responsecodestring) 
		+ strlen(HTTPEOL) 
		+ ((header == NULL) ? 0 : strlen(header));
	if (body != NULL) {
		len = (unterminatedBodySize == 0) ? 
		len + strlen(body) : len + unterminatedBodySize;
	}
	response = malloc(sizeof(char)*len);
	if (response == NULL) return NULL;
	strcpy(response, HTTPVER);
	ptr = response;
	ptr = chainstrcat(ptr, " ");
	ptr = chainstrcat(ptr, (char*) responsecodestring);
	ptr = chainstrcat(ptr, HTTPEOL);
	if (header != NULL) ptr = chainstrcat(ptr, header);
	if (body != NULL) {
		if (unterminatedBodySize == 0) ptr = chainstrcat(ptr, body);
		else for(int i = 0; i < unterminatedBodySize; i++) *ptr++ = body[i];
	}
	if (ilen != NULL) *ilen = len - 1;
	return response;
}

/*Ricordiamo, come detto in httphelper.h, che se una stringa che rappresenta un anno
è più di 50 anni nel futuro, deve essere trattata come un anno del secolo scorso.
C'è un particolare conflitto quindi con strptime, che invece tratta la stringa
del formato %Y come nel secolo scorso se rappresenta un numero da 69 a 99.
Questo conflitto viene risolto con un check nella funzione.*/
time_t *get_http_time_from_str(char* str){
    if (str == NULL) return NULL;
    time_t *mtime = malloc(sizeof(time_t));
    if (mtime == NULL) return NULL;
    char *fPtr = HTTP_DATE_FORMAT_1;
    int format = 1;
    struct tm mtm;
    while ((strptime(str, fPtr, &mtm)) == NULL && format < 4){
        format++;
        if (format == 2) fPtr = HTTP_DATE_FORMAT_2;
        else if (format == 3) fPtr = HTTP_DATE_FORMAT_3;
    }
    if (format == 4) return NULL;
    else {
        if(format == 2){
            time_t currentTime = time(NULL);
            struct tm *tmCurrentTime = gmtime(&currentTime);
            /*Controlla che il valore non sia stato "erroneamente" convertito
            da strptime.*/
            if (mtm.tm_year < 100){
                if ((mtm.tm_year - (tmCurrentTime->tm_year - 100)) < 51) mtm.tm_year += 100;
            }
        }
		*mtime = mktime(&mtm);
		return mtime;
    }
}

char *get_http_time(){
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = gmtime(&currentTime);
    char *str = malloc(sizeof(char)*32);
    strftime(str, 32, HTTP_DATE_FORMAT_1, tmCurrentTime);
    return str;
}

bool create_basic_header(char *header, bstnode *headerRoot, int contentLength){
	if (header == NULL) return false;
	header[0] = '\0';
	bool shouldClose;
	int connectionHeader = NOT_FOUND;
	if (headerRoot != NULL) connectionHeader = header_set_connection(headerRoot, header);
	shouldClose = (connectionHeader == CLOSE);
	//Content length
	char tmp[32];
	//L'int massimo ha 10 cifre. 10+1
	char tmpN[11];
	sprintf(tmpN, "%d", contentLength);
	char *ptr;
	tmp[0] = '\0';
	ptr = chainstrcat(tmp , "Content-Length: ");
	ptr = chainstrcat(tmp, tmpN);
	ptr = chainstrcat(tmp, HTTPEOL);
	ptr = chainstrcat(header, tmp);
	char *time = get_http_time();
	ptr = chainstrcat(header, "Date: ");
	ptr = chainstrcat(header, time);
	ptr = chainstrcat(header, HTTPEOL);
	free(time);
	return shouldClose;
}

