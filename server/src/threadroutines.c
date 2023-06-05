#include "threadroutines.h"
#include "config.h"
#include "mlog.h"
#include "httphelper.h"
#include "bstnode.h"
#include "dbconn.h"
#include "errorhandling.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <jansson.h>
#include <sys/socket.h>

#define BUFSIZE 4096
#define METHODBUFSIZE 32
#define PATHBUFSIZE 4096
#define VALUEBUFSIZE 512
#define ACTIVETHREADSCHECKDELAY_MS 500

void *thread_products_routine(void* arg){
	if (arg == NULL){
		mlog("SERVER-PRODUCTS",
			"Passed NULL to thread_products_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
	}
	handlerequestinput *hri = (handlerequestinput*)arg; 
	sharedthreadvariables *stv = hri->stv;
        if (stv == NULL){
        	mlog("SERVER-PRODUCTS",
        		"Stv is NULL. Program will be terminated.");
        	fatal("Unexpected NULL sharedThreadVariables while staring thread.");
        }
	//TODO
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_login_routine(void* arg){
	if (arg == NULL){                                         	
        	mlog("SERVER-LOGIN",
        		"Passed NULL to thread_login_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
        }
        handlerequestinput *hri = (handlerequestinput*)arg; 
	sharedthreadvariables *stv = hri->stv;
        if (stv == NULL){
        	mlog("SERVER-LOGIN",
        		"Stv is NULL. Program will be terminated.");
        	fatal("Unexpected NULL sharedThreadVariables while staring thread.");
        }
	//TODO
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_products_purchase_routine(void* arg){
	if (arg == NULL){                                         		
        	mlog("SERVER-PRODUCTS_PURCHASE",
        		"Passed NULL to thread_products_purchase_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
        }
        handlerequestinput *hri = (handlerequestinput*)arg; 
	sharedthreadvariables *stv = hri->stv;
        if (stv == NULL){
        	mlog("SERVER-PRODUCTS_PURCHASE",
        		"Stv is NULL. Program will be terminated.");
		fatal("Unexpected NULL sharedThreadVariables while staring thread.");
        }
        //TODO
        end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_register_routine(void* arg){
	if (arg == NULL){
		mlog("SERVER-REGISTER", 
			"Passed NULL to thread_register_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
	}
	handlerequestinput *hri = (handlerequestinput*)arg;
	sharedthreadvariables *stv = hri->stv;                                    	
        if (stv == NULL){
		mlog("SERVER-REGISTER",
        		"Stv is NULL. Program will be terminated.");
        	fatal("Unexpected NULL sharedThreadVariables while staring thread.");
        }
	responsecode statusCode = UNDEFINED;
	char **errBuf = malloc(sizeof(char*));
	if (errBuf != NULL) *errBuf = NULL;
	else {
		mlog("SERVER-REGISTER",
			"Could not allocate memory for errBuf. Quitting thread.");
		end_self_thread(hri, hri->stv, hri->tid);
	}
	//parsing body jansson
	const char *requestUsername = NULL;
	const char *requestPassword = NULL;
	if (hri->body != NULL){
		json_error_t jsonErr;
		json_t *root = json_loads(hri->body, 0, &jsonErr);
		if (root != NULL){
			if (json_is_object(root)){
				json_t *jsonUsername = json_object_get(root, "username");
				json_t *jsonPassword = json_object_get(root, "password");
				if (jsonUsername != NULL && jsonPassword != NULL &&
					json_is_string(jsonUsername) &&
					json_is_string(jsonPassword))
				{
					requestUsername = json_string_value(jsonUsername);
					requestPassword = json_string_value(jsonPassword);
				}
			}
			
		}
		json_decref(root); 
	}
	//database                                                                                                          
       	if (requestUsername != NULL && requestPassword != NULL){
		const char *params[] = {requestUsername, requestPassword};
		serverconfig *sc = hri->serverConfig;                                                                                      
	        PGconn *conn = get_db_conn(sc->dbName, sc->dbUsername, sc->dbPassword, errBuf);                                            
	        if (conn == NULL || ((*errBuf) != NULL)){                                                                                 
	        	if ((*errBuf) != NULL) {                                                                                          
	        		mlog("SERVER-REGISTER", *errBuf);                                                                         
	        		statusCode = SERVICE_UNAVAILABLE;                                                                                         
	        	}                                                                                                                 
	        	else {                                                                                                            
	        		mlog("SERVER-REGISTER",                                                                                   
	        			"Unexpected *errBuf NULL, but conn is NULL. Quitting thread, but this should not happen.");
				free(errBuf);        
	        		end_self_thread(hri, hri->stv, hri->tid);                                                                 
	        	}                                                                                                                 
	        }	                                                                                                                 
	        if (statusCode  == UNDEFINED){                                                                                                          
	        	PGresult *queryResult =                                                                                           
	        		PQexecParams(conn,                                                                                        
	        			REGISTERSTATEMENT,                                                                                
	        			2,                                                                                                
	        			NULL,                                                                                             
	        			params,
					NULL, NULL, 0);
			ExecStatusType execStatus = PQresultStatus(queryResult);                                                                                                                                                                                                            
	        	if (execStatus != PGRES_COMMAND_OK){
				if (execStatus == PGRES_NONFATAL_ERROR ||
					execStatus == PGRES_FATAL_ERROR)
				{statusCode = BAD_REQUEST;}
				else statusCode = INTERNAL_SERVER_ERROR; 
				mlog("SERVER-REGISTER",
					PQerrorMessage(conn));	
			} else statusCode = OK;
			PQclear(queryResult);
	        }
		PQfinish(conn);                                                                                                                      
	} else statusCode = BAD_REQUEST;
	free(errBuf);	
	
	pthread_mutex_lock(&stv->fdMutex);	
	write_response(stv->fd, statusCode, NULL, NULL, false);
	pthread_mutex_unlock(&stv->fdMutex);
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_send_100_continue(void* arg){
	if (arg == NULL){                                         		
        	mlog("SERVER-100CONTINUE",
        		"Passed NULL to thread_send_100_continue. Program will be terminated.");
        	fatal("Unexpected NULL argument while starting thread.");
        }
        handle100continueinput *h100ci = (handle100continueinput*)arg;
	sharedthreadvariables *stv = h100ci->stv;
	if (stv == NULL){
		mlog("SERVER-100CONTINUE",
			"Stv is NULL. Program will be terminated.");
		fatal("Unexpected NULL sharedThreadVariables while staring thread.");
	}
	pthread_mutex_lock(&stv->fdMutex);
	write_response(stv->fd, CONTINUE, NULL, NULL, false);
	pthread_mutex_unlock(&stv->fdMutex);
	end_self_thread_100continue(h100ci, h100ci->stv, h100ci->tid);                       
}

void *thread_handle_connection_routine(void* inputptr){
	handleconnectioninput *hci = ((handleconnectioninput*)inputptr);
	sharedthreadvariables *stv = malloc(sizeof(sharedthreadvariables));
	stv->fd = hci->fd;
	pthread_mutex_init(&(stv->activeThreadsMutex), NULL);
	pthread_mutex_init(&(stv->fdMutex), NULL);
 
	int byteCount = 0;
    	int bytesJustRead = 0;
	int toRead = BUFSIZE;
	int realUsedSize = BUFSIZE;
	int contentLength = 0;
	int totalChunkedDataSize = 0;
	int chunkedDataSize;
	int writeReturnVal;
	
	size_t bodyStartOffset;
	size_t messageEndOffset;
	size_t endLinePtrOffset;

	char *buf = malloc(sizeof(char)*toRead);
	char *nextReadLocation = buf;
	char *methodbuf = malloc(sizeof(char)*METHODBUFSIZE);
	char *pathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *tmppathbuf = malloc(sizeof(char)*PATHBUFSIZE);
	char *headernamebuf = malloc(sizeof(char)*HTTPMAXHEADERNAMELENGTH);
	char *valuebuf = malloc(sizeof(char)*VALUEBUFSIZE);
	char *nextStartForMemmove;
	char *startLinePtr = buf;
	char *endLinePtr = NULL;
	char *pathptr = NULL;
	char *token = NULL;
	char *arguments = NULL;
	char *body = NULL;
	char *response = NULL;
	
	bool requiresBody = false;
	bool isRequestLineParsed = false;
	bool isContentLengthParsed = false;
	bool isHostParsed = false;
	bool isDataChunkEncoded = false;
	bool shouldSend100Continue = false;
	bool shouldCloseConnection = false;
	bool *isHeaderFuncOutOfMemory = malloc(sizeof(bool));
	
	supportedmethod method;
	bstnode *routeroot = hci->routeroot;
	bstnode *requestedroute = NULL;
	bstnode *bstargroot = NULL;
	bstnode *headerRoot = NULL;
	responsecode errCode = UNDEFINED;
	requeststatus requestStatus = PARSING_REQUEST_LINE;
	serverconfig *serverConfig = hci->serverConfig;
	pthread_t tid100continue = 0;
	
	free(inputptr);
	
	if (buf == NULL || pathbuf == NULL || methodbuf == NULL || tmppathbuf == NULL 
	|| headernamebuf == NULL || valuebuf == NULL || isHeaderFuncOutOfMemory == NULL){
		mlog("SERVER-CONN", 
			"Could not allocate memory for request.");
		return NULL;
	}
	memset(buf, 0, BUFSIZE);
	while(requestStatus != DONE || shouldCloseConnection == false){
		if(requestStatus == DONE){
			size_t bytesReadFromNextMessage = (size_t) ((buf + byteCount - 1) - messageEndOffset);
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
				memmove(buf, buf+messageEndOffset+1, bytesReadFromNextMessage);
				memset(buf+bytesReadFromNextMessage, 0, realUsedSize - bytesReadFromNextMessage);
				totalChunkedDataSize = 0;
				toRead = realUsedSize;
				startLinePtr = buf;
				endLinePtr = NULL;
				nextReadLocation = buf + bytesReadFromNextMessage;
				byteCount = bytesReadFromNextMessage;
				requiresBody = false;
				isRequestLineParsed = false;
				isContentLengthParsed = false;
				isHostParsed = false;
				isDataChunkEncoded = false;
				shouldSend100Continue = false;
			}
			
		}
		if (requestStatus == OBTAINING_BODY){
			if ((buf + messageEndOffset) >= (buf+realUsedSize)){
				realUsedSize = messageEndOffset+1;
				int endLinePtrOffset = endLinePtr - buf;
				buf = realloc(buf, realUsedSize);
				if (buf == NULL){
					mlog("SERVER-CONN", 
						"Could not reallocate memory for request.");
					errCode = SERVICE_UNAVAILABLE;
					requestStatus = RESPONDING;
				}
				else {
					nextReadLocation = buf + byteCount;
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
				nextReadLocation = buf + byteCount;
				endLinePtr = buf + endLinePtrOffset;
				toRead = realUsedSize - byteCount;
				if (buf == NULL){
					mlog("SERVER-CONN", 
						"Could not reallocate memory for request.");
					errCode = SERVICE_UNAVAILABLE;
				}
			}
			bytesJustRead = read(stv->fd, nextReadLocation, toRead);
			if (bytesJustRead == 0) {
				mlog("SERVER-CONN", 
				"Connection closed unexpectedly.");
				shouldCloseConnection = true;
				continue;
			}
			byteCount += bytesJustRead;
			nextReadLocation += bytesJustRead;
			if ((buf + byteCount - 1) >= (buf + messageEndOffset) 
					&& requestStatus == OBTAINING_BODY){
				requestStatus = RESPONDING;
				break;
			}
		}
		while(1){
			if (requestStatus == DONE) break;
			if (requestStatus == OBTAINING_BODY) break;
			if (endLinePtr != NULL) startLinePtr = endLinePtr+1;
			if (errCode != UNDEFINED) requestStatus = RESPONDING;
			if (requestStatus != RESPONDING) {
				endLinePtr = find_newline(startLinePtr, 
					(nextReadLocation) - startLinePtr) - 1;
				if (endLinePtr == NULL){
					mlog("SERVER-CONN", "Didn't find endline. Rereading...");
					break;
				};  	
			}
			switch(requestStatus){
				case PARSING_REQUEST_LINE:
					if (sscanf(startLinePtr, "%s %s ", methodbuf, pathbuf) != 2) {
						errCode = BAD_REQUEST; 
						break;
					}
					requestStatus = PARSING_HEADERS;
					method = convert_string_to_supportedmethod_enum(methodbuf);
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
					if (token != NULL){
						if (strlen(token) != strlen(pathptr)){
							arguments = strtok(NULL, "?");                                          	
							if (!isValidArgStr(arguments)) 
								{errCode = BAD_REQUEST; break;}
						}
							
					}
					requestedroute = search(routeroot, token);
					if (requestedroute == NULL) {errCode = NOT_FOUND; break;}
					else{
						routeinfo* rinfo = (routeinfo*)(requestedroute->value);
						int8_t methodFlag = get_flag_value_for_method(method);						
                                                if (!(rinfo->acceptedMethodsMask & methodFlag))
							{errCode = METHOD_NOT_ALLOWED; break;}
						if (rinfo->requiresBody) requiresBody = true;	
					}
					break;
				case PARSING_TRAILERS:
				case PARSING_HEADERS:
					if (*startLinePtr == '\r' && *(startLinePtr+1) == '\n'){
						//L'header (o il trailer) è terminato
						//Se non abbiamo trovato l'header host, la richiesta non è valida secondo HTTP 1.1
						if ((!isHostParsed && !isDataChunkEncoded) ||
							(!isHostParsed && requestStatus == PARSING_TRAILERS)) 
							{errCode = BAD_REQUEST; break;}
						if (!requiresBody) {
							requestStatus = RESPONDING; 
							messageEndOffset = (size_t) (startLinePtr+1); 
							break;
						}
						if (shouldSend100Continue){
							pthread_t *tid100continue = malloc(sizeof(pthread_t));
							handle100continueinput *h100ci = malloc(sizeof(handle100continueinput));
							h100ci->stv = stv;
							h100ci->tid = tid100continue; 
							if (!start_thread(&thread_send_100_continue, h100ci, stv, tid100continue)){
								shouldCloseConnection = true;
								continue;
							}
						}
						//Abbiamo letto i trailer, abbiamo finito
						if (requestStatus == PARSING_TRAILERS) {
							requestStatus = RESPONDING; 
							messageEndOffset = (size_t) (startLinePtr+ 1 - buf); 
							break;
						}
						bodyStartOffset = (startLinePtr+2) - buf;
						if (isDataChunkEncoded){
							requestStatus = PARSING_CHUNKED_BODY;
							break;
						}
						if (!isContentLengthParsed) {errCode = BAD_REQUEST; break;}
						messageEndOffset = (startLinePtr+1+contentLength) - buf;
						bodyStartOffset = (startLinePtr+2) - buf;
						requestStatus = OBTAINING_BODY;
						break;
					}						
					if (sscanf(startLinePtr, "%[^:]: %s\r", headernamebuf, valuebuf) != 2)
						{errCode = BAD_REQUEST; break;}
					if (strcmp(headernamebuf, "Host") == 0){
						isHostParsed = true;
					} else if (strcmp(headernamebuf, "Content-Length") == 0){
						isContentLengthParsed = true;
						contentLength = atoi(valuebuf);
					} else if (strcmp(headernamebuf, "Transfer-Encoding") == 0){
						isDataChunkEncoded = (strcmp(valuebuf, "chunked") == 0);
					} else if (strcmp(headernamebuf, "Expect") == 0){
						if (strcmp(valuebuf, "100-continue") == 0) shouldSend100Continue = true;
					} else if (strcmp(headernamebuf, "Connection") == 0){
						if (strcmp(valuebuf, "close") == 0) shouldCloseConnection = true;
					}
					char *headername = malloc
							(sizeof(char)*(strlen(headernamebuf)+1));
					char *headervalue = malloc
							(sizeof(char)*(strlen(valuebuf)+1));
					strcpy(headername, headernamebuf);
					strcpy(headervalue, valuebuf);	
					headerRoot = add_bstnode(headerRoot, 
						create_bstnode(headername, headervalue));
					if (headerRoot == NULL ||
						 headername == NULL || headervalue == NULL) {
						mlog("SERVER-CONN", "Could not allocate memory for header.");
						errCode = SERVICE_UNAVAILABLE;
						break;
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
							contentLength = totalChunkedDataSize;
							requestStatus = PARSING_TRAILERS; 
							break;
						}
						totalChunkedDataSize += chunkedDataSize;
						startLinePtr = strchr(startLinePtr, '\r');
						if (startLinePtr == NULL || (*(++startLinePtr) != '\n')) {errCode = BAD_REQUEST; break;}
						endLinePtr = strchr(++startLinePtr, '\r');
						if (endLinePtr == NULL || (*(++endLinePtr) != '\n')) {errCode = BAD_REQUEST; break;}
						memmove(nextStartForMemmove, startLinePtr, chunkedDataSize);
						startLinePtr = 1 + endLinePtr;
					}
					break;
				case RESPONDING:
					handlerequestinput *newThreadInput;
					if (errCode == UNDEFINED){
						if (requiresBody){
							body = malloc(sizeof(char)*(contentLength+1));
							if (body == NULL) {
								mlog("SERVER-CONN", 
									"Could not reallocate memory for request.");
								errCode = SERVICE_UNAVAILABLE;				
							} else {
								strncpy(body, buf+bodyStartOffset, contentLength);
								body[contentLength] = '\0';
								newThreadInput = malloc(sizeof(handlerequestinput));
								bstargroot = malloc(sizeof(bstnode));
								if (newThreadInput == NULL || bstargroot == NULL) {
									free(newThreadInput);
									free(bstargroot);
									mlog("SERVER-CONN", 
											"Could not allocate memory for request.");
									errCode = SERVICE_UNAVAILABLE;
								}
								else {
									if (arguments == NULL) newThreadInput->arguments = NULL;
									else{
										bstargroot = mkargbst(arguments);
										if (bstargroot == NULL) errCode = SERVICE_UNAVAILABLE;
									}
								}
							}
						}
					}
					if (errCode != UNDEFINED){
						char *errLogBegin = "Sending error response with code ";
						const char *errCodeString = get_response_code_string(errCode);
						char *tmpErrMessageBuf = malloc(strlen(errLogBegin)+
							strlen(errCodeString)+1);
						strcpy(tmpErrMessageBuf, errLogBegin);
						strcat(tmpErrMessageBuf, errCodeString);
						mlog("SERVER-CONN", tmpErrMessageBuf);
						shouldCloseConnection = true;
						pthread_mutex_lock(&stv->fdMutex);
						write_response(stv->fd, errCode, "Connection: close\r\n\r\n", NULL, true);
						pthread_mutex_unlock(&stv->fdMutex);
						free(tmpErrMessageBuf);
					}
					else {
						newThreadInput->stv = stv;
						newThreadInput->body = body;
						newThreadInput->contentLength = contentLength;
						newThreadInput->arguments = bstargroot;
						newThreadInput->method = method;
						newThreadInput->headers = headerRoot;
						newThreadInput->serverConfig = serverConfig;	
						pthread_t *tid = malloc(sizeof(pthread_t));
						newThreadInput->tid = tid;
						if (tid100continue != 0)
						{
							pthread_join(tid100continue, NULL);
						}
						void *(*request_handler)(void*) = ((routeinfo*)requestedroute->value)->request_handler;
						if (!start_thread(request_handler, (void*) newThreadInput, stv, tid)){
							shouldCloseConnection = true;
						}
					}
					requestStatus = DONE;
					break;
			}
		}
		
	}
	shutdown(stv->fd, SHUT_RD);
	free(buf);
	free(methodbuf);
	free(pathbuf);
	free(tmppathbuf);
	free(isHeaderFuncOutOfMemory);
	free(valuebuf);
	free(headernamebuf);
	while(1){
		pthread_mutex_lock(&stv->activeThreadsMutex);
		if (stv->activeThreads == NULL) {
			pthread_mutex_unlock(&stv->activeThreadsMutex); 
			break;
		}
		pthread_mutex_unlock(&stv->activeThreadsMutex);
		sleep(ACTIVETHREADSCHECKDELAY_MS);
	}
	close(stv->fd);
	free_sharedthreadvariables(stv);
	free(hci);
}

bool start_thread(void *(*request_handler)(void*), void *input,
		sharedthreadvariables *stv, pthread_t *tid){
	if (tid == NULL || stv == NULL || input == NULL || request_handler == NULL){
		mlog("SERVER-CONN", "Could not start new thread (bad arguments).");
		return false;
	}
	bool wasAdded = add_activethread(stv, tid);	
	if (!wasAdded){
		mlog("SERVER-CONN", "Could not add thread to list of active threads. Aborting creation.");
		return false;	 
	}
	int createRetValue;                                                		
	createRetValue = pthread_create(tid, NULL, request_handler, input);
	if (createRetValue != 0){
		mlog("SERVER-CONN", strerror(createRetValue));
		remove_activethread(stv, tid);
		return false;	
	}
	pthread_detach(*tid);
	return true;
}

void end_self_thread(handlerequestinput *hri, 
		sharedthreadvariables *stv, pthread_t *tidPtr){
	free_handlerequestinput(hri);
	remove_activethread(stv, tidPtr);
	pthread_exit(NULL);
}

void end_self_thread_100continue(handle100continueinput *h100ci,
		sharedthreadvariables *stv, pthread_t *tidPtr){
	free_handle100continueinput(h100ci);
	remove_activethread(stv, tidPtr);
	pthread_exit(NULL);	
}

void free_handlerequestinput(handlerequestinput *hri){
	free(hri->body);
	free_bstargs(hri->arguments);
	free_bstheaders(hri->headers);
	free(hri);	
}

void free_handle100continueinput(handle100continueinput *h100ci){
	free(h100ci);
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

void free_bstheaders(bstnode *bstheaders){
	free_bstargs(bstheaders); //Stessa implementazione...
}

void free_sharedthreadvariables(sharedthreadvariables *sharedthreadvariables){
	pthread_mutex_destroy(&(sharedthreadvariables->activeThreadsMutex));
	pthread_mutex_destroy(&(sharedthreadvariables->fdMutex));
	free(sharedthreadvariables);
}

bool add_activethread(sharedthreadvariables *stv, pthread_t *tidPtr){
	linkedlistnode *lln = create_linkedlistnode((void*) tidPtr);
	if (lln == NULL) return false;
	pthread_mutex_lock(&stv->activeThreadsMutex);
	lln->next = stv->activeThreads;
	stv->activeThreads = lln;
	pthread_mutex_unlock(&stv->activeThreadsMutex);
	return true;
}

void remove_activethread(sharedthreadvariables *stv, pthread_t *tidPtr){
	pthread_mutex_lock(&stv->activeThreadsMutex);  	
        stv->activeThreads = remove_linkedlistnode
        	(stv->activeThreads, tidPtr);
        pthread_mutex_unlock(&stv->activeThreadsMutex);
}
