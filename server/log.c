#include <stdio.h>
#include <time.h>
#include "log.h"
#include <stdlib.h>

void log(char* tag, char* message){

	time_t currenttime;
	now(&currenttime);
	printf("[%s] %s: %s\n", ctime(&currenttime), tag, message);
}
