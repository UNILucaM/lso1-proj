#include <stdio.h> 
#include <stdlib.h>
#include "log.h"

void fatal(char* errmsg){
	log("FATAL", errmsg);
	perror(NULL);
	exit(EXIT_FAILED);
}
