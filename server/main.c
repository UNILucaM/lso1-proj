#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/un.h>

#include "errorhandling.h"
#include "log.h"
#include "serverinfo.h"

#define QUEUE_LENGTH 5
#define MAX_BUF_SIZE 256
#define EOT 4
#define PORT 5200

void *thread_server_child_write_routine(void *arg){
        int server_child_fd = *((int*)arg);
       	char* msg = "Hey client.\n\x04";
       	int sizeofmsg = strlen(msg);
       	int n = 0;
       	int byteswritten = 0;
       	while (byteswritten < sizeofmsg){
       		if ((n = write(server_child_fd, msg, sizeofmsg)) < 0) fatal("Error writing");
       		byteswritten += n;
        }
       	pthread_exit(NULL);
}

int main(){
	struct serverinfo* server = create_server(PORT, QUEUE_LENGTH);
	int server_child_fd;
	int pid;
	int* number_of_conns = malloc(sizeof(int));
	*number_of_conns = 0;
	while(1){
		printf("Waiting for new connection... (active connections: %d)\n", *number_of_conns);
		if ((server_child_fd = accept(server_fd, NULL, NULL)) < 0) fatal("Cannot open new socket");
		printf("New connection with client established!\n");
		(*number_of_conns)++;
		if ((pid = fork()) < 0) fatal("Cannot fork");
		else if (pid == 0) {
			pthread_t write_tid;
			void* (*thread_server_child_write_routine_ptr)(void*) = &thread_server_child_write_routine;
			if (pthread_create(&write_tid, NULL, thread_server_child_write_routine_ptr, &server_child_fd) < 0)
				fatal("Cannot create write thread");
			char* msgbuf = malloc(sizeof(char)*MAX_BUF_SIZE);
			int msgsize = 0;
			int n;
			while(1){
				if ((n = read(server_child_fd, msgbuf, MAX_BUF_SIZE)) < 0) fatal("Read error");
				else if (msgbuf[n-1] == EOT) {
					msgbuf += n;
					msgsize += n;
					break;
				}
				else msgbuf += n;
				msgsize += n;
			}
			msgbuf -= msgsize;
			printf("CLIENT MESSAGE: %s\n", msgbuf);
			pthread_join(write_tid, NULL);
			printf("Write finished.\n");
			close(server_child_fd);
			printf("Connection closed. Connections now active: %d", --(*number_of_conns));
			return 0;
		}
	}

}

