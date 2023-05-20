#include <stdio.h> 
#include <stdlib.h>

void fatal(char* errmsg){
	perror(errmsg);
	exit(EXIT_FAILED);
}
