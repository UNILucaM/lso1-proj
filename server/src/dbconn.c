#include <libpq-fe.h>
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

const char *productcolumnnames[] =
	{"pid", 
	"200 OK", 
	"400 Bad Request",
	"401 Unauthorized",
	"404 Not Found",
	"405 Method Not Allowed",
	"500 Internal Server Error", 
	"501 Not Implemented", 
	"503 Service Unavailable", 
	""};

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


char *make_json_array_from_productqueryresult(PGresult *res, char *errBuf, bool isSuggested){
	char *body;
	json_t *jsonArray = json_array();
	json_t *jsonProduct;
	json_t *jsonTmp;
	char *value;
	if (jsonArray == NULL){
		strcpy(errBuf, "Out of memory.");
		return NULL;
	}
	int n = PQntuples(res);
	int columnN = (isSuggested) ? 
		PRODUCT_NUMBER_OF_COLUMNS + 1 : 
		PRODUCT_NUMBER_OF_COLUMNS;
	int i, j;
	for (i = 0; i < n; i++){
		jsonProduct = json_object();
		if (jsonProduct == NULL){
			strcpy(errBuf, "Out of memory.");
			return NULL;
		}
		for (j = 0; j < columnN; j++){
			value = PQgetvalue(res, i, j);
			if (value == NULL){
				strcpy(errBuf, "Error retrieving field value.");
				return NULL;
			}
			if (PQgetisnull(res, i, j)) jsonTmp = json_null();
			else switch(j){
				case PRODUCT_COLUMN_PID:
				case PRODUCT_COLUMN_SUGGESTIONSCORE:
					jsonTmp = json_integer(strtol(value, NULL, 10));
					break;
				case PRODUCT_COLUMN_PRODUCTNAME:
				case PRODUCT_COLUMN_PRODUCTTYPEVAR:
				case PRODUCT_COLUMN_IMAGEPATH:
					jsonTmp = json_string(value);
					break;
				case PRODUCT_COLUMN_PRICE:
					jsonTmp = json_real(strtod(value, NULL));
			}
			if (jsonTmp == NULL){
					strcpy(errBuf, "Error creating json.");
					return NULL;
			}
			json_object_set_new(jsonProduct, PQfname(res, j), jsonTmp);
		}
		if (json_array_append_new(jsonArray, jsonProduct) == -1){
				strcpy(errBuf, "Error appending to json.");
				return NULL;
		}
	}
	body = json_dumps(jsonArray, 0);
	json_decref(jsonArray);
	return body;
}