#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>

#include "config.h"

serverconfig *load_serverconfig_from_file(char *filePath){
	if (filePath == NULL) return NULL;
	serverconfig *sc = malloc(sizeof(serverconfig));
	if (sc == NULL) return NULL;
	sc->dbName = NULL;
	sc->dbUsername = NULL;
	sc->dbPassword = NULL;
	sc->dbAddr = NULL;
	sc->imagePath = NULL;
	sc->port = PORT_UNSPECIFIED;
	bool hasFailed = false;
	char *parameternamebuf = malloc(sizeof(char)*LINEPTRBUFSIZE/2);
	char *parametervaluebuf = malloc(sizeof(char)*LINEPTRBUFSIZE/2);
	char *lineptrbuf = malloc(sizeof(char)*LINEPTRBUFSIZE);
	FILE *configFile = fopen(filePath, "r");
	if (configFile != NULL && lineptrbuf != NULL){
		size_t  n = LINEPTRBUFSIZE;
		while(getline(&lineptrbuf, &n, configFile) != -1){
			if(sscanf(lineptrbuf, "%[^:]: %s\n", 
                		parameternamebuf, parametervaluebuf) != 2){
				hasFailed = true;
				break;
                		
                	}
                	if(strcmp(parameternamebuf, "dbname") == 0){
                		sc->dbName = 
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (sc->dbName == NULL){
					hasFailed = true;
					break;
				}
				strcpy(sc->dbName, parametervaluebuf);
                	} else if (strcmp(parameternamebuf, "dbusername") == 0){
				sc->dbUsername =
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (sc->dbUsername == NULL){
					hasFailed = true;
					break;
				}
				strcpy(sc->dbUsername, parametervaluebuf);
			} else if (strcmp(parameternamebuf, "dbpassword") == 0){
				sc->dbPassword =
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (sc->dbPassword == NULL){
					hasFailed = true;
					break;
				}
				strcpy(sc->dbPassword, parametervaluebuf);
			} else if (strcmp(parameternamebuf, "serverport") == 0){
				int port = strtol(parametervaluebuf, NULL, 10);
				if (port == LONG_MIN || port == LONG_MAX){
					if (errno == ERANGE){
						hasFailed = true;
						break;
					}
				}
				sc->port = port;
			} else if (strcmp(parameternamebuf, "dbaddr") == 0){           	
				sc->dbAddr =
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (sc->dbAddr == NULL){
					hasFailed = true;
					break;
				}
				strcpy(sc->dbAddr, parametervaluebuf);
			} else if (strcmp(parameternamebuf, "imagepath") == 0){
				sc->imagePath =
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (sc->imagePath == NULL){
					hasFailed = true;
					break;
				}
				strcpy(sc->imagePath, parametervaluebuf);
			}
		}
	} else hasFailed = true;	
	if (configFile != NULL) fclose(configFile);
	free(parameternamebuf);
	free(parametervaluebuf);
	free(lineptrbuf);
	if (hasFailed){
		free_serverconfig(sc);	
		return NULL;
	}
	return sc;		
}

void free_serverconfig(serverconfig *serverConfig){
	if (serverConfig->dbName != NULL)
		free(serverConfig->dbName);    	
	if (serverConfig->dbUsername != NULL)
		free(serverConfig->dbUsername);
	if (serverConfig->dbPassword != NULL)
		free(serverConfig->dbPassword);
	if (serverConfig->dbAddr != NULL)
		free(serverConfig->dbAddr);
	if (serverConfig->imagePath != NULL)
		free(serverConfig->imagePath);
    	free(serverConfig);
}
