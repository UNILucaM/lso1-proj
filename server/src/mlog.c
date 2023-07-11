#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h> 
#include <pthread.h>

#include "mlog.h"

void mlog(char* tag, char* message, int connectionNum){
	time_t currenttime;
	time(&currenttime);
	char *timeStr = ctime(&currenttime);
	//rimuovi newline
	timeStr[strlen(timeStr)-1] = '\0';
	printf("(%d) {%ld} [%s] %s: %s\n", connectionNum, pthread_self(), timeStr, tag, message);
}
