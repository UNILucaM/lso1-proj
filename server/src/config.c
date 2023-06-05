#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <limits.h>

#include "config.h"

serverconfig *load_serverconfig_from_file(char *filePath){
	if (filePath == NULL) return NULL;
	serverconfig *serverconfig = malloc(sizeof(serverconfig));
	if (serverconfig == NULL) return NULL;
	serverconfig->dbName = NULL;
	serverconfig->dbUsername = NULL;
	serverconfig->dbPassword = NULL;
	serverconfig->port = PORT_UNSPECIFIED;
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
                		serverconfig->dbName = 
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (serverconfig->dbName == NULL){
					hasFailed = true;
					break;
				}
				strcpy(serverconfig->dbName, parametervaluebuf);
                	} else if (strcmp(parameternamebuf, "dbusername") == 0){
				serverconfig->dbUsername =
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (serverconfig->dbUsername == NULL){
					hasFailed = true;
					break;
				}
				strcpy(serverconfig->dbName, parametervaluebuf);
			} else if (strcmp(parameternamebuf, "dbpassword") == 0){
				serverconfig->dbPassword =
					malloc((strlen(parametervaluebuf)+1)*sizeof(char));
				if (serverconfig->dbPassword == NULL){
					hasFailed = true;
					break;
				}
				strcpy(serverconfig->dbPassword, parametervaluebuf);
			} else if (strcmp(parameternamebuf, "serverport") == 0){
				int port = strtol(parametervaluebuf, NULL, 10);
				if (port == LONG_MIN || port == LONG_MAX){
					if (errno == ERANGE){
						hasFailed = true;
						break;
					}
				}
				serverconfig->port = port;
			}		
		}
	} else hasFailed = true;	
	if (configFile != NULL) fclose(configFile);
	free(parameternamebuf);
	free(parametervaluebuf);
	free(lineptrbuf);
	if (hasFailed){
		free_serverconfig(serverconfig);	
		return NULL;
	}
	return serverconfig;		
}

void free_serverconfig(serverconfig *serverConfig){
	free(serverConfig->dbName);    	
        free(serverConfig->dbUsername);
        free(serverConfig->dbPassword);
        free(serverConfig);
}
