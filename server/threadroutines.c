#include "threadroutines.h"
#include "config.h"
#include "log.h"
#include "httphelper.h"
#include <routing.>
#include <string.h>

#define METHODBUFSIZE 32
#define PATHBUFSIZE 4096

typedef struct handlerequestinput{
	int fd;
	route* routeroot;
}handleconnectioninput;

typedef struct handlerequestinput{
	char* body;
	char* arguments;
}
void *thread_handle_connection_routine(void* inputptr){
	handleconnectioninput input = *((handleconnectioninput*)inputptr);
	int fd = input.fd;
	int bytecount = 0;
        int tmpbytecount = 0;
        int toRead = BUFSIZE;
        bool isRequestLineParsed = false;
        bool isContentLengthParsed = false;
	char *buf = malloc(sizeof(char)*toRead);
	char *methodbuf = malloc(sizeof(char)*METHODBUFSIZE);
	char *pathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *tmppathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *pathptr;
	char *token;
	supportedmethod method;
	if (buf == NULL || pathbuf == NULL || methodbuf == NULL){
		log("SERVER-CONN", 
			"Could not allocate memory for request.");
		//TODO: RETURN 5xx AND DEALLOC
		return;
	}
	while (1){
		tmpbytecount = read(fd, buf, toRead);
		if (tmpbytecount == 0) return; //TODO: RETURN 4xx AND DEALLOC
		byteCount += tmpbytecount;
		else if (!isRequestLineParsed){
			sscanf((buf, "%s %s ", methodbuf, pathbuf);
			if (methodbuf == NULL || pathbuf == NULL) continue;
			isRequestLineParsed = true;
			method = conver_string_to_supportedmethod_enum(methodbuf);
			if (method == UNSUPPORTED) return; 
			//TODO: RETURN 4xx AND DEALLOC
			
			//Se il path è assoluto (es: http://server.com/products)
			if (pathbuf[0] == 'h'){
				int i = 0;
				pathptr = pathbuf;
				for (i; i < 3; i++) 
					pathptr = (1 + strchr(pathptr, '/'));
				pathptr--;
			}
			else pathptr = pathbuf;
			strcpy(tmppathbuf, pathptr);
			token = strtok(tmppathbuf, "?");
			if (token == NULL) token = pathptr;
			route *resultroute = search(input.routeroot, token);
			if (resultroute == NULL) return; //TODO: RETURN 4xx AND DEALLOC
			
		}
	}	        	
}
