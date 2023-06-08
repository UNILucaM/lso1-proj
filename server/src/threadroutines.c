#include "threadroutines.h"
#include "config.h"
#include "mlog.h"
#include "httphelper.h"
#include "bstnode.h"
#include "dbconn.h"
#include "errorhandling.h"
#include "strutil.h"
#include "mimage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <jansson.h>
#include <sys/socket.h>

void *thread_images_routine(void *arg){
	char *tag = "SERVER-IMAGES";
	if (arg == NULL){
		mlog(tag, 
			"Passed NULL to thread_images_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
	}
	handlerequestinput *hri = (handlerequestinput*)arg;
	sharedthreadvariables *stv = hri->stv;                                    	
	if (stv == NULL){
		mlog(tag,
        		"Stv is NULL. Program will be terminated.");
        fatal("Unexpected NULL sharedThreadVariables while staring thread.");
	}
	responsecode statusCode = UNDEFINED;
	bstnode *argRoot = hri->arguments;
	bstnode *headerRoot = hri->headers;
	serverconfig *sc = hri->serverConfig;
	bstnode *genericNode; 
	time_t *headerLastModified = NULL;
	bool isIfModifiedSince = false;
	genericNode = search(headerRoot, "If-Modified-Since");
	if (genericNode == NULL) genericNode = search(headerRoot, "If-Unmodified-Since");
	else isIfModifiedSince = true;
	if (genericNode != NULL){
		headerLastModified = get_http_time_from_str((char*)genericNode->value);
	}
	char *imageName = NULL;
	char *tmpImageName = NULL;
	char *baseImagePath = ((sc->imagePath == NULL) ? IMAGES_PATH : sc->imagePath);
	char *image;
	time_t lastModified;
	int imageByteSize;
	imagesize imgSize = SIZE_UNDEFINED;
	genericNode = search(argRoot, "imagename");
	if (genericNode == NULL) statusCode = BAD_REQUEST;
	else{
		imageName = (char*) genericNode->value;
		genericNode = search(argRoot, "size");
		if (genericNode != NULL) 
			imgSize = convert_string_to_imagesize_enum((char*) genericNode->value);
		if (imgSize == SIZE_UNDEFINED) imgSize = MEDIUM;
		tmpImageName = get_path_for_size(imageName, imgSize);
		if (tmpImageName == NULL) statusCode = BAD_REQUEST;
		else{
			imageName = malloc(sizeof(char)*
				(strlen(tmpImageName) + strlen(baseImagePath) + 1));
			if (imageName == NULL) statusCode == INTERNAL_SERVER_ERROR;
			else {
				strcpy(imageName, baseImagePath);
				strcat(imageName, tmpImageName);
				image = load_image_from_path_timebased
					(imageName, &imageByteSize, headerLastModified, isIfModifiedSince);
				if (image == NULL && imageByteSize != NO_NEED_TO_LOAD){
					mlog(tag, (char*) get_image_load_error_string(imageByteSize));
					statusCode = (imageByteSize == STAT_ERROR) ? 
						NOT_FOUND : INTERNAL_SERVER_ERROR;
				} else statusCode = OK;
				free(imageName);
			}
		}
	}
	if (tmpImageName != NULL) free(tmpImageName);
	if (headerLastModified != NULL) free(headerLastModified);
	char *header = malloc(sizeof(char)*BUFSIZE);
	int contentLength = (imageByteSize < 0) ? 0 : imageByteSize;
	bool shouldClose = false;
	if (header != NULL){
		 shouldClose = create_basic_header(header, headerRoot, contentLength);
		 strcat(header, "\r\n");
		 if (imageByteSize == NO_NEED_TO_LOAD){
			statusCode = (isIfModifiedSince) ? 
				NOT_MODIFIED : PRECONDITION_FAILED;
		}
	}
	else {mlog(tag, "Could not allocate header."); statusCode = SERVICE_UNAVAILABLE;}
	attempt_response(tag, stv, statusCode, header, image, shouldClose);
	if (header != NULL) free(header);
	if (image != NULL) free(image);
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_products_routine(void *arg){
	char *tag = "SERVER-PRODUCTS";
	if (arg == NULL){
		mlog(tag,
			"Passed NULL to thread_products_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
	}
	handlerequestinput *hri = (handlerequestinput*)arg; 
	sharedthreadvariables *stv = hri->stv;
	if (stv == NULL){
			mlog(tag,
        		"Stv is NULL. Program will be terminated.");
        	fatal("Unexpected NULL sharedThreadVariables while staring thread.");
	}
	responsecode statusCode = UNDEFINED;
	char *errBuf = malloc(sizeof(char)*ERRBUFSIZE);
	if (errBuf == NULL) {
		mlog(tag,
			"Could not allocate memory for errBuf. Quitting thread.");
		end_self_thread(hri, hri->stv, hri->tid);
	}
	errBuf[0] = '\0';
	bstnode *argroot = hri->arguments;
	bstnode *argNode;
	bstnode *headerRoot = hri->headers;
	int type = PRODUCT_TYPE_INVALID;
	char *username;
	char *body = NULL;
	if (argroot != NULL){
		argNode = search(argroot, "type");
		if (argNode != NULL && argNode->value != NULL){
			if (strcmp((char*)(argNode->value), "cocktail") == 0){
				type = PRODUCT_TYPE_COCKTAIL;
			}
			else if (strcmp((char*)(argNode->value), "frullato") == 0) {
				type = PRODUCT_TYPE_FRULLATO;
			}
			else if (strcmp((char*)(argNode->value), "suggested") == 0) {
				argNode = search(argroot, "username");
				if (argNode != NULL && argNode->value != NULL){
					type = PRODUCT_TYPE_SUGGESTED;
					username = (char*) (argNode->value);
				}
				else type = PRODUCT_TYPE_INVALID;
			} else {
				type = PRODUCT_TYPE_INVALID;
			}
		} 
	}
	if (type == PRODUCT_TYPE_INVALID) statusCode = BAD_REQUEST;
	else {
		//database
		serverconfig *sc = hri->serverConfig;  
		PGconn *conn = get_db_conn
			(sc->dbName, sc->dbUsername, sc->dbPassword, sc->dbAddr, errBuf);    
		if (conn == NULL || errBuf[0] != '\0'){                                                                                 
			if (errBuf[0] != '\0') {                                                                                          
				mlog(tag, errBuf);                                                                         
				statusCode = INTERNAL_SERVER_ERROR;                                                                                         
			}                                                                                                                 
			else {                                                                                                            
				mlog(tag,                                                                                   
					"Unexpected errBuf NULL, but conn is NULL. Quitting thread, but this should not happen.");
				free(errBuf);        
				end_self_thread(hri, hri->stv, hri->tid);                                                                 
			}                                                                                                                 
		} else {
			char *statement;
			PGresult *queryResult;
			if (type == PRODUCT_TYPE_COCKTAIL) statement = GETPRODUCTCOCKTAILSTATEMENT;
			else if (type == PRODUCT_TYPE_FRULLATO) statement = GETPRODUCTFRULLATOSTATEMENT;
			else statement = GETPRODUCTSUGGESTEDSCORESTATEMENT;
			if (type != PRODUCT_TYPE_SUGGESTED){
				queryResult =                                                                                           
					PQexec(conn,                                                                                        
					statement);
			} else {
				const char *params[] = {username};               
				queryResult =                                                                                           
					PQexecParams(conn,                                                                                        
					statement,                                                                                
					1,                                                                                                
					NULL,                                                                                             
					params,
					NULL, NULL, 0);
			}
			ExecStatusType execStatus = PQresultStatus(queryResult);                                                                                                                                                                                                            
			if (execStatus != PGRES_TUPLES_OK){
				if (execStatus == PGRES_NONFATAL_ERROR ||
					execStatus == PGRES_FATAL_ERROR)
				{statusCode = BAD_REQUEST;}
				else statusCode = INTERNAL_SERVER_ERROR; 
				mlog(tag,
					PQerrorMessage(conn));	
			} else {
				body = make_json_array_from_productqueryresult
					(queryResult, errBuf, (type == PRODUCT_TYPE_SUGGESTED));
				if (body == NULL || errBuf[0] != '\0'){
					mlog(tag, ((errBuf[0] != '\0') ? errBuf : "Body is null."));
					statusCode = BAD_REQUEST;
				} else statusCode = OK;
			}
			PQclear(queryResult);
		}
		PQfinish(conn);                        
	}			
	free(errBuf);
	char *header = malloc(sizeof(char)*BUFSIZE);
	int contentLength = (body == NULL) ? 0 : (int) strlen(body);
	bool shouldClose = false;
	if (header != NULL){
		 shouldClose = create_basic_header(header, headerRoot, contentLength);
		 strcat(header, "\r\n");
	}
	else {mlog(tag, "Could not allocate header."); statusCode = SERVICE_UNAVAILABLE;}
	attempt_response(tag, stv, statusCode, header, body, shouldClose);
	if (header != NULL) free(header);
	if (body != NULL) free(body);
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_login_routine(void* arg){
	char *tag = "SERVER-LOGIN";
	if (arg == NULL){
		mlog(tag, 
			"Passed NULL to thread_login_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
	}
	handlerequestinput *hri = (handlerequestinput*)arg;
	sharedthreadvariables *stv = hri->stv;                                    	
	if (stv == NULL){
		mlog(tag,
        		"Stv is NULL. Program will be terminated.");
        fatal("Unexpected NULL sharedThreadVariables while staring thread.");
	}
	responsecode statusCode = UNDEFINED;
	char *errBuf = malloc(sizeof(char)*ERRBUFSIZE);
	if (errBuf == NULL) {
		mlog(tag,
			"Could not allocate memory for errBuf. Quitting thread.");
		end_self_thread(hri, hri->stv, hri->tid);
	}
	errBuf[0] = '\0';
	//parsing body jansson
	const char *requestUsername = NULL;
	const char *requestPassword = NULL;
	json_t *root = NULL;
	if (hri->body != NULL){
		json_error_t jsonErr;
		root  = json_loads(hri->body, 0, &jsonErr);
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
	}
	//database                                                                                                          
	if (requestUsername != NULL && requestPassword != NULL){
		const char *params[] = {requestUsername, requestPassword};
		serverconfig *sc = hri->serverConfig;                                                                                      
		PGconn *conn = get_db_conn
			(sc->dbName, sc->dbUsername, sc->dbPassword, sc->dbAddr, errBuf);                                            
		if (conn == NULL || errBuf[0] != '\0'){                                                                                 
			if (errBuf[0] != '\0') {                                                                                          
				mlog(tag, errBuf);                                                                         
				statusCode = INTERNAL_SERVER_ERROR;                                                                                         
			}                                                                                                                 
			else {                                                                                                            
				mlog(tag,                                                                                   
					"Unexpected errBuf NULL, but conn is NULL. Quitting thread, but this should not happen.");
				free(errBuf);        
				end_self_thread(hri, hri->stv, hri->tid);                                                                 
			}                                                                                                                 
		}	                                                                                                                 
		if (statusCode  == UNDEFINED){                                                                                                          
			PGresult *queryResult =                                                                                           
				PQexecParams(conn,                                                                                        
				LOGINSTATEMENT,                                                                                
				2,                                                                                                
				NULL,                                                                                             
				params,
				NULL, NULL, 0);
			ExecStatusType execStatus = PQresultStatus(queryResult);                                                                                                                                                                                                            
			if (execStatus != PGRES_TUPLES_OK){
				if (execStatus == PGRES_NONFATAL_ERROR ||
					execStatus == PGRES_FATAL_ERROR)
				{statusCode = BAD_REQUEST;}
				else statusCode = INTERNAL_SERVER_ERROR; 
				mlog(tag,
					PQerrorMessage(conn));	
			} else {
				if (PQntuples(queryResult) == 1) statusCode = OK;
				else statusCode = UNAUTHORIZED;
			}
			PQclear(queryResult);
		}
		PQfinish(conn);                                                                                                                      
	} else statusCode = BAD_REQUEST;
	free(errBuf);
	if (root != NULL) json_decref(root);
	char *header = malloc(sizeof(char)*BUFSIZE);
	int contentLength = 0;
	bool shouldClose = false;
	if (header != NULL){
		 shouldClose = create_basic_header(header, hri->headers, contentLength);
		 strcat(header, "\r\n");
	}
	else {mlog(tag, "Could not allocate header."); statusCode = SERVICE_UNAVAILABLE;}
	attempt_response(tag, stv, statusCode, header, NULL, shouldClose);
	free(header);
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_products_purchase_routine(void* arg){
	char *tag = "SERVER-PRODUCTSPURCHASE";
	if (arg == NULL){                                         		
		mlog(tag,
			"Passed NULL to thread_products_purchase_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
	}
	handlerequestinput *hri = (handlerequestinput*)arg; 
	sharedthreadvariables *stv = hri->stv;
	if (stv == NULL){
		mlog(tag,
			"Stv is NULL. Program will be terminated.");
		fatal("Unexpected NULL sharedThreadVariables while staring thread.");
	}
	responsecode statusCode = UNDEFINED;
	char *errBuf = malloc(sizeof(char)*ERRBUFSIZE);
	if (errBuf == NULL) {
		mlog(tag,
			"Could not allocate memory for errBuf. Quitting thread.");
		end_self_thread(hri, hri->stv, hri->tid);
	}
	errBuf[0] = '\0';
	//parsing body jansson
	const char *requestUsername = NULL;
	json_t *root = NULL;
	json_t *jsonArray = NULL;
	json_t *jsonUnpurchasedArray = NULL;
	bool wasAtleastOnePurchaseSuccessful = false;
	if (hri->body != NULL){
		json_error_t jsonErr;
		root  = json_loads(hri->body, 0, &jsonErr);
		if (root != NULL){
			if (json_is_object(root)){
				json_t *jsonUsername = json_object_get(root, "username");
				if (jsonUsername != NULL && json_is_string(jsonUsername))
				{
					requestUsername = json_string_value(jsonUsername);
				}
			}
		}
	}
	serverconfig *sc = hri->serverConfig;
	bstnode *headerRoot = hri->headers;
	PGconn *conn = get_db_conn
		(sc->dbName, sc->dbUsername, sc->dbPassword, sc->dbAddr, errBuf);
	if (conn == NULL || errBuf[0] != '\0'){                                                                                 
		if (errBuf[0] != '\0') {                                                                                          
			mlog(tag, errBuf);                                                                         
			statusCode = SERVICE_UNAVAILABLE;                                                                                         
		}                                                                                                                 
		else {                                                                                                            
			mlog(tag,                                                                                   
				"Unexpected *errBuf NULL, but conn is NULL. Quitting thread, but this should not happen.");
			free(errBuf);        
			end_self_thread(hri, hri->stv, hri->tid);                                                                 
		}                                                                                                                 
	}		
	if (requestUsername != NULL && statusCode == UNDEFINED){
		jsonArray = json_object_get(root, "products");
		jsonUnpurchasedArray = json_array();
		if (jsonArray != NULL && jsonUnpurchasedArray != NULL && json_is_array(jsonArray)){
			json_t *jsonProduct;
			json_t *jsonProductPid;
			json_t *jsonProductQuantity;
			int pid;
			int quantity;
			size_t index;
			//11 = numero massimo di cifre in un int + 1
			char strPid[11];
			char strQuantity[11];
			const char *params[] = {requestUsername, strPid, strQuantity};
			serverconfig *sc = hri->serverConfig;			
			json_array_foreach(jsonArray, index, jsonProduct) {
				if (json_is_object(jsonProduct)){
					jsonProductPid = json_object_get(jsonProduct, "pid");
					jsonProductQuantity = json_object_get(jsonProduct, "quantity");
					if (json_is_integer(jsonProductPid) 
						&& json_is_integer(jsonProductQuantity)){
						pid = json_integer_value(jsonProductPid);
						quantity = json_integer_value(jsonProductQuantity);
						sprintf(strPid, "%d", pid);
						sprintf(strQuantity, "%d", quantity);
						PGresult *queryResult =                                                                                           
							PQexecParams(conn,                                                                                        
							SALESTATEMENT,                                                                                
							3,                                                                                                
							NULL,                                                                                             
							params,
							NULL, NULL, 0);
						ExecStatusType execStatus = PQresultStatus(queryResult);                                                                                                                                                                                                            
						if (execStatus != PGRES_COMMAND_OK){
							if (execStatus == PGRES_NONFATAL_ERROR ||
								execStatus == PGRES_FATAL_ERROR)
							{
								jsonProduct = json_object();
								if (jsonProduct == NULL) statusCode = SERVICE_UNAVAILABLE;
								else {
									jsonProductPid = json_integer(pid);
									json_object_set_new(jsonProduct, "pid", jsonProductPid);
									if (json_array_append_new(jsonUnpurchasedArray, jsonProduct) == -1)
										statusCode = SERVICE_UNAVAILABLE;
								}
							}
							else statusCode = INTERNAL_SERVER_ERROR; 
							mlog(tag,
								PQerrorMessage(conn));	
						} else wasAtleastOnePurchaseSuccessful = true;
						PQclear(queryResult);
						if (statusCode == INTERNAL_SERVER_ERROR || statusCode == SERVICE_UNAVAILABLE){
							break;
						}
					}
						                              
				}
			} PQfinish(conn);        
		} else statusCode = BAD_REQUEST;
	} else statusCode = BAD_REQUEST;
	if (statusCode == UNDEFINED) {
		statusCode = (wasAtleastOnePurchaseSuccessful) ? 
			OK : BAD_REQUEST;
	}
	if (root != NULL) json_decref(root);
	if (jsonArray != NULL) json_decref(jsonArray);
	char *body = NULL;
	if (json_array_size(jsonUnpurchasedArray) != 0) body = json_dumps(jsonUnpurchasedArray, 0);
	json_decref(jsonUnpurchasedArray);
	char *header = malloc(sizeof(char)*BUFSIZE);
	int contentLength = (body == NULL) ? 0 : (int) strlen(body);
	bool shouldClose = false;
	if (header != NULL){
		 shouldClose = create_basic_header(header, headerRoot, contentLength);
		 strcat(header, "\r\n");
	}
	else {mlog(tag, "Could not allocate header."); statusCode = SERVICE_UNAVAILABLE;}
	attempt_response(tag, stv, statusCode, header, body, shouldClose);
	free(header);
	free(body);
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_register_routine(void* arg){
	char *tag = "SERVER-REGISTER";
	if (arg == NULL){
		mlog(tag, 
			"Passed NULL to thread_register_routine. Program will be terminated.");
		fatal("Unexpected NULL argument while starting thread.");
	}
	handlerequestinput *hri = (handlerequestinput*)arg;
	sharedthreadvariables *stv = hri->stv;                                    	
	if (stv == NULL){
		mlog(tag,
        		"Stv is NULL. Program will be terminated.");
        fatal("Unexpected NULL sharedThreadVariables while staring thread.");
	}
	responsecode statusCode = UNDEFINED;
	char *errBuf = malloc(sizeof(char)*ERRBUFSIZE);
	if (errBuf == NULL) {
		mlog(tag,
			"Could not allocate memory for errBuf. Quitting thread.");
		end_self_thread(hri, hri->stv, hri->tid);
	}
	errBuf[0] = '\0';
	//parsing body jansson
	const char *requestUsername = NULL;
	const char *requestPassword = NULL;
	json_t *root = NULL;
	if (hri->body != NULL){
		json_error_t jsonErr;
		root  = json_loads(hri->body, 0, &jsonErr);
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
	}
	//database                                                                                                          
	if (requestUsername != NULL && requestPassword != NULL){
		const char *params[] = {requestUsername, requestPassword};
		serverconfig *sc = hri->serverConfig;                                                                                      
		PGconn *conn = get_db_conn
			(sc->dbName, sc->dbUsername, sc->dbPassword, sc->dbAddr, errBuf);                                            
		if (conn == NULL || errBuf[0] != '\0'){                                                                                 
			if (errBuf[0] != '\0') {                                                                                          
				mlog(tag, errBuf);                                                                         
				statusCode = SERVICE_UNAVAILABLE;                                                                                         
			}                                                                                                                 
			else {                                                                                                            
				mlog(tag,                                                                                   
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
				mlog(tag,
					PQerrorMessage(conn));	
			} else statusCode = OK;
			PQclear(queryResult);
		}
		PQfinish(conn);                                                                                                                      
	} else statusCode = BAD_REQUEST;
	free(errBuf);
	if (root != NULL) json_decref(root);
	char *header = malloc(sizeof(char)*BUFSIZE);
	int contentLength = 0;
	bool shouldClose = false;
	if (header != NULL){
		 shouldClose = create_basic_header(header, hri->headers, contentLength);
		 strcat(header, "\r\n");
	}
	else {mlog(tag, "Could not allocate header."); statusCode = SERVICE_UNAVAILABLE;}
	attempt_response(tag, stv, statusCode, header, NULL, shouldClose);
	free(header);
	end_self_thread(hri, hri->stv, hri->tid);
}

void *thread_send_100_continue(void* arg){
	char *tag;
	if (arg == NULL){                                         		
        	mlog(tag,
        		"Passed NULL to thread_send_100_continue. Program will be terminated.");
        	fatal("Unexpected NULL argument while starting thread.");
        }
        handle100continueinput *h100ci = (handle100continueinput*)arg;
	sharedthreadvariables *stv = h100ci->stv;
	if (stv == NULL){
		mlog(tag,
			"Stv is NULL. Program will be terminated.");
		fatal("Unexpected NULL sharedThreadVariables while staring thread.");
	}
	attempt_response(tag, stv, CONTINUE, "\r\n", NULL, false);
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
	char *versionbuf = malloc(sizeof(char)*VERSIONBUFSIZE);
	char *nextStartForMemmove;
	char *startLinePtr = buf;
	char *endLinePtr = NULL;
	char *pathptr = NULL;
	char *token = NULL;
	char *arguments = NULL;
	char *body = NULL;
	char *response = NULL;
	char *tag = "SERVER-CONN";
	
	bool requiresBody = false;
	bool isRequestLineParsed = false;
	bool isContentLengthParsed = false;
	bool isHostParsed = false;
	bool isDataChunkEncoded = false;
	bool shouldSend100Continue = false;
	bool shouldCloseConnection = false;
	bool isHTTP1Point0 = false;
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
	if (buf == NULL || pathbuf == NULL || methodbuf == NULL || tmppathbuf == NULL 
	|| headernamebuf == NULL || valuebuf == NULL || isHeaderFuncOutOfMemory == NULL){
		mlog(tag, 
			"Could not allocate memory for request.");
		return NULL;
	}
	memset(buf, 0, BUFSIZE);
	while(requestStatus != DONE || shouldCloseConnection == false){
		if(requestStatus == DONE){
			size_t bytesReadFromNextMessage = (size_t) ((byteCount - 1) - messageEndOffset);
			if (realUsedSize != BUFSIZE){
				int reallocSize = (bytesReadFromNextMessage < BUFSIZE) ? BUFSIZE : bytesReadFromNextMessage;
				buf = realloc(buf, reallocSize);
				if (buf == NULL){
					mlog(tag, 
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
				if (bytesReadFromNextMessage > 0) memmove(buf, buf+messageEndOffset+1, bytesReadFromNextMessage);
				else bytesReadFromNextMessage = 0;
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
				isHTTP1Point0 = false;
				requestStatus = PARSING_REQUEST_LINE;
				headerRoot = NULL;
				bstargroot = NULL;
			}
			
		}
		if (requestStatus == OBTAINING_BODY){
			if ((buf + messageEndOffset) >= (buf+realUsedSize)){
				realUsedSize = messageEndOffset+1;
				int endLinePtrOffset = endLinePtr - buf;
				buf = realloc(buf, realUsedSize);
				if (buf == NULL){
					mlog(tag, 
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
					mlog(tag, 
						"Could not reallocate memory for request.");
					errCode = SERVICE_UNAVAILABLE;
				}
			}
			bytesJustRead = read(stv->fd, nextReadLocation, toRead);
			if (bytesJustRead == 0) {
				mlog(tag, 
					"Connection closed unexpectedly.");
				shouldCloseConnection = true;
				requestStatus = DONE;
				continue;
			} else if (bytesJustRead == -1 && 
				(errno == EAGAIN || errno == EWOULDBLOCK)){
				mlog(tag,
					"Socket read timeout. Attempting to send timeout response.");
				shouldCloseConnection = true;
				attempt_error_response(tag, stv, REQUEST_TIMEOUT);
				requestStatus = DONE;
				continue;	
			}
			byteCount += bytesJustRead;
			nextReadLocation += bytesJustRead;
			if (requestStatus == OBTAINING_BODY){
				if ((buf + byteCount - 1) >= (buf + messageEndOffset)){
					requestStatus = RESPONDING;
					break;
				}
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
					mlog(tag, "Didn't find endline. Rereading...");
					break;
				};  	
			}
			switch(requestStatus){
				case PARSING_REQUEST_LINE:
					if (sscanf(startLinePtr, "%s %s %s", methodbuf, pathbuf, versionbuf) != 3) {
						errCode = BAD_REQUEST; 
						break;
					}
					if (strcmp(versionbuf, "HTTP/1.0") == 0) isHTTP1Point0 = true;
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
						routeinfo *rinfo = (routeinfo*)(requestedroute->value);
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
						if ((!isHostParsed && !isDataChunkEncoded && !isHTTP1Point0) ||
							(!isHostParsed && requestStatus == PARSING_TRAILERS)) 
							{errCode = BAD_REQUEST; break;}
						if (!requiresBody) {
							requestStatus = RESPONDING; 
							messageEndOffset = (size_t) (startLinePtr + 1 - buf); 
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
						mlog(tag, "Could not allocate memory for header.");
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
								mlog(tag, 
									"Could not reallocate memory for request.");
								errCode = SERVICE_UNAVAILABLE;				
							} else {
								strncpy(body, buf+bodyStartOffset, contentLength);
								body[contentLength] = '\0';
							}
						}
						newThreadInput = malloc(sizeof(handlerequestinput));
						if (newThreadInput == NULL) {
							free(newThreadInput);
							mlog(tag, 
									"Could not allocate memory for request.");
							errCode = SERVICE_UNAVAILABLE;
						}
						else {
							if (arguments == NULL) newThreadInput->arguments = NULL;
							else{
								bstargroot = mkargbst(arguments);
								if (bstargroot == NULL) {
									free(newThreadInput);
									mlog(tag,
										"Could not allocate memory for request.");
									errCode = SERVICE_UNAVAILABLE;
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
						mlog(tag, tmpErrMessageBuf);
						shouldCloseConnection = true;
						attempt_error_response(tag, stv, errCode);
						free(tmpErrMessageBuf);
					}
					else {
						newThreadInput->stv = stv;
						newThreadInput->body = body;
						newThreadInput->contentLength = contentLength;
						newThreadInput->method = method;
						newThreadInput->headers = headerRoot;
						newThreadInput->arguments = bstargroot;
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
	mlog(tag, "Exiting thread...");
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
	pthread_attr_t *attr;
	int initRet = pthread_attr_init(attr);
	if (initRet != 0 || pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED) != 0){
		if (initRet == 0) pthread_attr_destroy(attr);
		mlog("SERVER-CONN", "Could not set detached state. Aborting thread creation.");
		remove_activethread(stv, tid);
		return false;
	}                                                		
	createRetValue = pthread_create(tid, attr, request_handler, input);
	if (createRetValue != 0){
		mlog("SERVER-CONN", strerror(createRetValue));
		pthread_attr_destroy(attr);
		remove_activethread(stv, tid);
		return false;	
	}
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

void attempt_error_response(char *logtag, sharedthreadvariables *stv, responsecode errCode){
	attempt_response(logtag, stv, errCode, "Connection: close\r\n\r\n", NULL, true);
}

void attempt_response(char *logtag, sharedthreadvariables *stv, 
	responsecode code, char *header, char *body,
	bool shouldClose){
	int writeResult;
	pthread_mutex_lock(&stv->fdMutex);
	writeResult = write_response(stv->fd, code, header, body, shouldClose);
	pthread_mutex_unlock(&stv->fdMutex);
	if (writeResult < 0){
		bool timedOut = (errno == EAGAIN || errno == EWOULDBLOCK);
		char *errStr = (timedOut) ? 
			"Write timed out." : strerror(writeResult);
		mlog(logtag, errStr);
	} else mlog(logtag, "Write completed.");
}
