#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PGconn *get_db_conn(char *dbName, char* username, char *password, char *hostaddr, char *err){
	char *formatStr = "user=%s password=%s dbname=%s hostaddr=%s";
	if (dbName == NULL || username == NULL || password == NULL || hostaddr == NULL || err == NULL){
		strcpy(err, "Invalid parameters.");
		return NULL;
	}
	int paramN = 4;
	/*Sottraiamo 2paramN - 1 perché ogni stringa da rimpiazzare viene indicata con
	%s in formatStr, quindi sono 2 caratteri extra per parametro, 
	quindi 2paramN caratteri extra, ma dobbiamo contare anche che 
	serve un carattere per il NULL, quindi 2paramN - 1*/
	int n = strlen(formatStr) + strlen(dbName) + strlen(username)
	+ strlen(password) + strlen(hostaddr) - (2*paramN) - 1;
	char *buf = malloc(sizeof(char)*n);
	if (buf == NULL){
		strcpy(err, "Can't allocate buf.");
		return NULL;
	}
	if (sprintf(buf, formatStr, username, password, dbName, hostaddr) >  0){
		PGconn *conn = PQconnectdb(buf);
		if (PQstatus(conn) == CONNECTION_BAD){
			strcpy(err, PQerrorMessage(conn));
			PQfinish(conn);
		}
		else return conn;	
	}
	return NULL;
	
}
