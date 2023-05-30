#include <stdio.h> 
#include <stdlib.h>
#include "mlog.h"

void fatal(char* errmsg){
	mlog("FATAL", errmsg);
	perror(NULL);
	exit(EXIT_FAILURE);
}
