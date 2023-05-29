#include "threadroutines.h"
#include "config.h"
#include "mlog.h"
#include "httphelper.h"
#include "bstnode.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#define METHODBUFSIZE 32
#define PATHBUFSIZE 4096



void *thread_send_100_continue(void* arg){
	write_response(*((int*)arg), CONTINUE, NULL, NULL);
}

void *thread_handle_connection_routine(void* inputptr){
	int fd = ((*handleconnectioninput)inputptr)->fd;
	int bytecount = 0;
    int bytesJustRead = 0;
	int bodyStartOffset;
	int messageEndOffset;
    int toRead = BUFSIZE;
	int realUsedSize = BUFSIZE;
	int contentLength = 0;
	int totalChunkedDataSize = 0;
	int chunkedDataSize;
	int endLinePtrOffset;
	int writeReturnVal;
	
	char *arguments;
	char *buf = malloc(sizeof(char)*toRead);
	char *nextreadlocation = buf;
	char *methodbuf = malloc(sizeof(char)*METHODBUFSIZE);
	char *pathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *tmppathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *headernamebuf = malloc(sizeof(char)*HTTPMAXHEADERNAMELENGTH);
	char *valuebuf = malloc(sizeof(char)*BUFSIZE);
	char *nextStartForMemmove;
	char *startLinePtr = buf;
	char *endLinePtr = NULL;
	char *pathptr = NULL;
	char *token = NULL;
	char *arguments = NULL;
	char *body = NULL;
	char *response = NULL;
	
	bool isFirstLine = true;
	bool requiresBody = false;
	bool isBufFull = false;
	bool isRequestLineParsed = false;
	bool isContentLengthParsed = false;
	bool isHostParsed = false;
	bool hasBody = false;
	bool isTryingToAcquireBody = false;
	bool isDataChunkEncoded = false;
	bool shouldSend100Continue = false;
	bool shouldCloseConnection = false,
	bool *isHeaderFuncOutOfMemory = malloc(sizeof(bool));
	
	supportedmethod method;
	bstnode *routeroot = ((*handleconnectioninput)inputptr)->routeroot;
	bstnode *requestedroute = NULL;
	bstnode *bstargroot = NULL;
	responsecode errCode = UNDEFINED;
	requeststatus requeststatus = PARSING_REQUEST_LINE;
	pthread_t *tid100continue = NULL;
	
	free(inputptr);
	
	if (buf == NULL || pathbuf == NULL || methodbuf == NULL || tmppathbuf == NULL 
	|| headernamebuf == NULL || valuebuf == NULL || isHeaderFuncOutOfMemory == NULL){
		mlog("SERVER-CONN", 
			"Could not allocate memory for request.");
		return;
	}
	memset(0, buf, BUFSIZE);
	while(requestStatus != DONE || shouldCloseConnection == false){
		if(requestStatus == DONE){
			int bytesReadFromNextMessage = (buf + byteCount - 1) - messageEndOffset;
			if (realUsedSize != BUFSIZE){
				int reallocSize = (bytesReadFromNextMessage < BUFSIZE) ? BUFSIZE : bytesReadFromNextMessage;
				buf = realloc(buf, reallocSize);
				if (buf == NULL){
					mlog("SERVER-CONN", 
						"Could not reallocate memory for request.");
					errCode = SERVICE_UNAVAILABLE;
					requestStatus = RESPONDING;
				}
				else {
					errCode = UNDEFINED;
					requestStatus = PARSING_REQUEST_LINE;
				}
				realUsedSize = reallocSize;
			}
			
			if (buf != NULL){
				//BUGBUGBUG
				memmove(buf, buf+messageEndOffset+1, bytesReadFromNextMessage);
				memset(buf+bytesReadFromNextMessage, 0, realUsedSize - bytesReadFromNextMessage);
				totalChunkedDataSize = 0;
				toRead = realUsedSize;
				startLinePtr = buf;
				endLinePtr = NULL;
				nextreadlocation = buf + bytesReadFromNextMessage;
				byteCount = bytesReadFromNextMessage;
				requiresBody = false;
				isRequestLineParsed = false;
				isContentLengthParsed = false;
				isHostParsed = false;
				hasBody = false;
				isDataChunkEncoded = false;
				shouldSend100Continue = false;
				shouldCloseConnection = false,
			}
			
		}
		if (requestStatus == OBTAINING_BODY){
			if ((buf + messageEndOffset) >= (buf+realUsedSize)){
				realUsedSize = bodyEndOffset+1;
				int endLinePtrOffset = endLinePtr - buf;
				buf = realloc(buf, realUsedSize);
				if (buf == NULL){
					mlog("SERVER-CONN", 
						"Could not reallocate memory for request.");
					errCode = SERVICE_UNAVAILABLE;
					requestStatus = RESPONDING;
				}
				else {
					nextreadlocation = buf + byteCount;
					endLinePtr = buf + endLinePtrOffset;
					toRead = realUsedSize - byteCount;
				}
			} else requestStatus = RESPONDING;
		}
		if (requestStatus != RESPONDING){
			if (nextReadLocation >= (buf + realUsedSize)){
				realUsedSize += BUFSIZE/2;
				int endLinePtrOffset = endLinePtr - buf;
				buf = realloc(buf, realUsedSize);
				nextreadlocation = buf + byteCount;
				endLinePtr = buf + endLinePtrOffset;
				toRead = realUsedSize - byteCount;
				if (buf == NULL){
					mlog("SERVER-CONN", 
						"Could not reallocate memory for request.");
					errCode = SERVICE_UNAVAILABLE;
				}
			}
			bytesJustRead = read(fd, nextreadlocation, toRead);
			if (bytesJustRead == 0) {
				mlog("SERVER-CONN", 
				"Connection closed unexpectedly.");
			}
			byteCount += bytesJustRead;
			nextreadlocation += bytesJustRead;
			if ((buf + byteCount - 1) >= (buf + bodyEndOffset) 
					&& requeststatus == OBTAINING_BODY){
				requeststatus = RESPONDING;
				break;
			}
		}
		while(1){
			if (requestStatus == OBTAINING_BODY) break;
			if (endLinePtr != NULL) startLinePtr = endLinePtr+1;
			if (errCode != UNDEFINED) requestStatus = RESPONDING;
			else {
				endLinePtr = find_newline(startLinePtr, 
					(nextReadLocation) - startLinePtr);
				//DEBUGDEBUGDEBUG TODO
				printf("%.*s\n", (int) strcspn(startLinePtr, "\r"), startLinePtr);
				if (endLinePtr == NULL) break;  	
			}
			switch(requeststatus){
				case PARSING_REQUEST_LINE:
					if (sscanf(startLinePtr, "%s %s ", methodbuf, pathbuf) != 2) {
						errCode = BAD_REQUEST; 
						break;
					}
					requeststatus = PARSING_HEADERS;
					method = conver_string_to_supportedmethod_enum(methodbuf);
					if (method == UNSUPPORTED) {errCode = NOT_IMPLEMENTED; break;}
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
						if (!isValidArgStr(arguments)) {errCode = BAD_REQUEST; break;}
					*requestedroute = search(input.routeroot, token);
					if (requestedroute  == NULL) {errCode = NOT_FOUND; break;}
					else{
						routeinfo* routeinfo = (routeinfo*) requestedroute->value;
						if (routeinfo->requiresBody) requiresBody = true;	
					}
					break;
				case PARSING_TRAILERS:
				case PARSING_HEADERS:
					if (*startLinePtr == '\r' && *(startLinePtr+1) == '\n'){
						//L'header (o il trailer) è terminato
						//Se non abbiamo trovato l'header host, la richiesta non è valida secondo HTTP 1.1
						if (!isHostParsed) {errCode = BAD_REQUEST; break;}
						if (!requiresBody) {
							requeststatus = RESPONDING; 
							messageEndOffset = (startLinePtr+1); 
							break;
						}
						if (shouldSend100Continue){
							int resultCreate100Continue = pthread_create(&tid100continue, NULL,
								&thread_send_100_continue, (void*) &fd);
							if (resultCreate100Continue != 0){
								mlog("SERVER-CONN", strerror(resultCreate100Continue));
							}
						}
						//Abbiamo letto i trailer, abbiamo finito
						if (requeststatus == PARSING_TRAILERS) {
							requeststatus = RESPONDING; 
							messageEndOffset = (startLinePtr+1); 
							break;
						}
						bodyStartOffset = (startLinePtr+2) - buf;
						if (isDataChunkEncoded){
							requeststatus = PARSING_CHUNKED_BODY;
							break;
						}
						if (!isContentLengthParsed) {errCode = BAD_REQUEST; break;}
						messageEndOffset = (startLinePtr+1+contentLength) - buf;
						bodyStartOffset = (startLinePtr+2) - buf;
						requeststatus = OBTAINING_BODY;
						break;
					}						
					if (sscanf(startLinePtr, "%s: %s\r", headernamebuf, valuebuf) != 2)
						{errCode = BAD_REQUEST; break;}
					if (strcmp(headernamebuf, "Host") == 0){
						isHostParsed = true;
					} else if (strcmp(headernamebuf, "Content-Length") == 0){
						isContentLengthParsed = true;
						contentLength = atoi(valuebuf);
					} else if (strcmp(headernamebuf, "Transfer-Encoding") == 0{
						isDataChunkEncoded = (strcmp(valuebuf, "chunked") == 0);
					} else if (strcmp(headernamebuf, "Expect") == 0){
						if (strcmp(valuebuf, "100-continue") == 0) shouldSend100Continue = true;//TODO: send 100
					} else if (strcmp(headernamebuf, "Connection") == 0){
						if (strcmp(valuebuf, "close") == 0) shouldCloseConnection = true;
					}
					break;
				case OBTAINING_BODY:
					break;
				case PARSING_CHUNKED_BODY:
					while(1){
						if (*startLinePtr == 0) break;
						if (totalChunkedDataSize == 0) nextStartForMemmove = startLinePtr;
						else nextStartForMemmove += chunkedDataSize;
						sscanf(startLinePtr, "%x", &chunkedDataSize);
						//Fine del trasferimento, ora si cerca di leggere i trailer
						if (chunkedDataSize == 0) {
							hasBody = true; 
							contentLenght = totalChunkedDataSize;
							chunkedtransferstatus = PARSING_TRAILERS; 
							break;
						}
						totalChunkedDataSize += chunkedDataSize;
						startLinePtr = strchr(startLinePtr, '\r');
						if (startLinePtr == NULL || (*(++startLinePtr) != '\n')) {errCode = BAD_REQUEST; break;}
						endLinePtr = (++startLinePtr, '\r');
						if (endLinePtr == NULL || (*(++endLinePtr) != '\n')) {errCode = BAD_REQUEST; break;}
						memmove(nextStartFormMemmove, startLinePtr, chunkedDataSize);
						startLinePtr = 1 + endLinePtr;
					}
					break;
				case RESPONDING:
					if (errCode == UNDEFINED)){
						if (requiresBody){
							body = malloc(sizeof(char)*contentLength);
							if (body == NULL) {
								mlog("SERVER-CONN", 
									"Could not reallocate memory for request.");
								errCode = SERVICE_UNAVAILABLE;				
							} else {
								strncpy(body, buf+bodyStartOffset, contentLength);
								handlerequestinput *input = NULL;
								input = malloc(sizeof(handlerequestinput));
								bstargroot = malloc(sizeof(bstnode));
								if (input == NULL || bstargroot == NULL) {
									free(input);
									free(bstargroot);
									mlog("SERVER-CONN", 
											"Could not reallocate memory for request.");
									errCode = SERVICE_UNAVAILABLE;
								}
								else {
									if (arguments == NULL) input->arguments = NULL;
									else{
										bstargroot = mkargbst(arguments);
										if (bstargroot == NULL) errCode = SERVICE_UNAVAILABLE;
									}
								}
							}
						}
					}
					if (errCode != UNDEFINED){
						mlog("SERVER-CONN", strcat("Sending error response with code ", get_response_code_string(errCode));
						write_response(fd, errCode, "Connection: close\r\n\r\n", NULL, true);
					}
					else {
						//DEBUGDEBUGDEBUG TODO
						exit(0);
						input->fd = fd;
						input->body = body;
						input->contentLenght = contentLenght;
						input->arguments = bstargroot;
						pthread_t tid;
						if (tid100continue != NULL)
						{
							pthread_join(*tid100continue, NULL);
						}
						void *(request_handler)(void*) = ((routeinfo*)requesteredroute->value)->request_handler;
						int createRetValue = pthread_create(&tid, NULL, request_handler, (void*) input);
						if (createRetValue != 0){
							mlog("SERVER-CONN", strerror(createRetValue));
							close(fd);
						} else pthread_detach(tid);
					}
					requestStatus = DONE;
					break;
			}
		}
		
	}
	
	free(buf);
	free(methodbuf);
	free(pathbuf);
	free(tmppathbuf);
	free(isHeaderFuncOutOfMemory);
	free(valuebuf);
	free(headernamebuf);

}
