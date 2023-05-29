#include <stdbool.h>
#include "bstnode.h"

#define HTTPVER "HTTP/1.1"
#define HTTPEOL "\r\n\"
#define HTTPMAXHEADERNAMELENGTH 40
typedef enum {
	UNSUPPORTED = -1,
	GET,
	POST,
	PUT
}supportedmethod;

const struct{
	supportedmethod value;
	char* str;
} supportmethodconversiontable[] =
	{GET, "GET"},
	{POST, "POST"},
	{PUT, "PUT"}
};

typedef enum{
	CONTINUE,
	OK,
	BAD_REQUEST,
	NOT_FOUND,
	NOT_IMPLEMENTED,
	SERVICE_UNAVAILABLE,
	UNDEFINED
}responsecode;

typedef enum{
	PARSING_REQUEST_LINE,
	PARSING_HEADERS,
	PARSING_TRAILERS,
	PARSING_CHUNKED_BODY,
	OBTAINING_BODY,
	RESPONDING,
	DONE
}requeststatus;

typedef struct routeinfo{
	void (*request_handler)(void*);
	bool requiresBody;
}routeinfo;

const char *responsecodestrings[] = 
	{"100 Continue", "200 OK", "400 Bad Request", "404 Not Found", "501 Not Implemented", "503 Service Unavailable", ""};

char *get_header_field_value(char*, char*, bool*);
int convert_string_to_supportedmethod_enum(char*);
char *find_newline(char*, int);
char *get_response_code_string(responsecode);
char *form_response(responsecode, char*, char*, int*);
int write_response(int, responsecode, char*, char*, bool);
bstnode *mkroute(char*, void*(*)(void*), bool);
//NOTA: L'implementazione fornita distrugge la stringa originale.
bstnode *mkargbst(char*);