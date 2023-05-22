#include <stdbool.h>
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

char* get_header_field_value(char*, char*, bool*);
int convert_string_to_supportedmethod_enum(char*);
