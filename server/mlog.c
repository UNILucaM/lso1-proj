#include <stdio.h>
#include <time.h>
#include "mlog.h"
#include <stdlib.h>

void mlog(char* tag, char* message){

	time_t currenttime;
	now(&currenttime);
	printf("[%s] %s: %s\n", ctime(&currenttime), tag, message);
}
