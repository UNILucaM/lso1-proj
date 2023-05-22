#include "threadroutines.h"
#include "config.h"
#include "log.h"
#include "httphelper.h"
#include <routing.>
#include <string.h>

#define METHODBUFSIZE 32
#define PATHBUFSIZE 4096

void *thread_handle_connection_routine(void* inputptr){
	handleconnectioninput input = *((handleconnectioninput*)inputptr);
	int fd = input.fd;
	int bytecount = 0;
        int tmpbytecount = 0;
        int toRead = BUFSIZE;
	int realUsedSize = BUFSIZE;
        bool isRequestLineParsed = false;
        bool isContentLengthParsed = false;
	char *buf = malloc(sizeof(char)*toRead);
	char *bufptr = buf;
	char *methodbuf = malloc(sizeof(char)*METHODBUFSIZE);
	char *pathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *tmppathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *pathptr;
	char *token;
	char  *tmpCRLFPtr = NULL;
	char *contentLengthHeaderVal = NULL;
	bool *isHeaderFuncOutOfMemory;
	char *errCode = NULL;
	char *arguments = NULL;
	bool isBufFull = false;
	int bodyStartPos;
	supportedmethod method;
	route *requestedroute;
	if (buf == NULL || pathbuf == NULL || methodbuf == NULL){
		log("SERVER-CONN", 
			"Could not allocate memory for request.");
		//TODO: RETURN 5xx AND DEALLOC
		return;
	}
	while (1){
		if (isBufFull){
			memset(0, buf, BUFSIZE);
			byteCount = 0;
			bufptr = buf;
		}
		tmpbytecount = read(fd, bufptr, toRead);
		if (tmpbytecount == 0) return; //TODO: RETURN 4xx AND DEALLOC
		byteCount += tmpbytecount;
		bufptr += tmpbytecount;
		if (bufptr == (buf + realUsedSize)){
			isBufFull = true;
			//Se il buf è stato reallocato ed è pieno, allora abbiamo il body
			if (realUsedSize != BUFSIZE){		
				hasBody = true;
				break;
			}
		}
		if (!isRequestLineParsed){
			sscanf(buf, "%s %s ", methodbuf, pathbuf);
			if (methodbuf == NULL || pathbuf == NULL) continue;
			isRequestLineParsed = true;
			method = conver_string_to_supportedmethod_enum(methodbuf);
			if (method == UNSUPPORTED) return; 
			//TODO: RETURN 4xx AND DEALLOC
			
			//Se il path è assoluto (es: http://server.com/products)
			if (pathbuf[0] == 'h'){
				int i = 0;
				pathptr = pathbuf;
				//Avanza oltre i primi 3 / e poi torna indietro di 1, esattamente sul terzo /
				for (i; i < 3; i++) 
					pathptr = (1 + strchr(pathptr, '/'));
				pathptr--;
			}
			else pathptr = pathbuf;
			strcpy(tmppathbuf, pathptr);
			token = strtok(tmppathbuf, "?");
			if (strlen(token) != strlen(pathptr)) 
				arguments = strtok(NULL, "?");
			route *requestedroute = search(input.routeroot, token);
			if (requestedroute  == NULL) return; //TODO: RETURN 4xx AND DEALLOC
			if (requestedroute->info->requiresBody) requiresBody = true;	
		}
		if (isRequestLineParsed && !requiresBody) break;
		if (contentLengthHeaderVal == NULL) contentLengthHeaderVal = get_header_field_value
			(buf, "Content-Length", isHeaderFuncOutOfMemory);
		if (contentLengthHeaderVal == NULL){
			if (*isHeaderFuncOutOfMemory){
				log("SERVER-CONN", 
				"Couldn't allocate memory in function get_header_field_value");
				//TODO: RETURN 5xx AND DEALLOC
			} else continue;	
		}  else {
			tmpCRLFPtr = buf;
			while ((tmpCRLFPtr = strchr(tmpCRLFPtr, '\r\n')) != NULL){
				if ( (tmpCRLFPtr++) == '\r\n'){
				       	void* bodyEndAddress = 
			        		 tmpCRLFPtr+1+atoi(contentLengthHeaderVal);
					bodyStartPos = (tmpCRLFPtr+1) - buf;
			        	if (bodyEndAddress >= (buf+BUFSIZE)){
						realUsedSize = bodyEndAddress - buf;
						toRead = realUsedSize;
			        		buf = realloc(buf, realUsedSize);
						if (buf == NULL) return; //TODO:
						isBufFull = false;
						break;
			        	} else {hasBody = true; break;}
			        }
			
			} if (hasBody) break;
			
		}
	}
	//TODO: dealloc and detect errcode, handle request	        	
}
