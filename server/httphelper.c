#include "httphelper.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

char* get_header_field_value(char *input, char *headerFieldName, bool *isOutOfMemory){
	
	char* str = strstr(input, headerFieldName);
	if (str == NULL) return NULL;
	char* copy = malloc(sizeof(str)+1);
	if (copy == NULL){
		*isOutOfMemory = TRUE;
		return;
	}
	strcpy(copy, str);
	strtok(copy, " ");
	char* token = strtok(copy, "\r\n");
	free(copy);
	return token;	
}

int convert_string_to_supportedmethods_enum(char* str){
	int i;
	int n = sizeof(supportmethodconversiontable) / 
		sizeof(supportmethodsconversiontable[0]);
	for (i = 0; i < n; i++){
		if (strcmp(supportmethodconversiontable[i].str,
			str) == 0)
			return supportmethodsconversiontable[i].value;
	}
	return UNSUPPORTED;
}
