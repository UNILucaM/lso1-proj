#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PGconn *get_db_conn(char *dbName, char* username, char *password, char **err){
	char *formatStr = "user=%s password=%s dbname=%s";
	if (dbName == NULL || username == NULL || password == NULL || err == NULL){
		if (err != NULL) *err = "Invalid parameters.";
		return NULL;
	}
	/*Sottraiamo 5 perché ogni stringa da rimpiazzare viene indicata con
	%s in formatStr, quindi sono 2 caratteri extra per parametro, 
	quindi 6 caratteri extra, ma dobbiamo contare anche che 
	serve un carattere per il NULL, quindi 5*/
	int n = strlen(formatStr) + strlen(dbName) + strlen(username)
	+ strlen(password) - 5;
	char *buf = malloc(sizeof(char)*n);
	if (buf == NULL){
		if (err != NULL) *err = "Could not allocate buffer";
		return NULL;
	}
	if (sprintf(buf, formatStr, username, password, dbName) >  0){
		PGconn *conn = PQconnectdb(buf);
		if (PQstatus(conn) == CONNECTION_BAD){
			*err = PQerrorMessage(conn);
			PQfinish(conn);
		}
		else return conn;	
	}
	return NULL;
	
}
