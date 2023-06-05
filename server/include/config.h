#define PORT_UNSPECIFIED -1
#define LINEPTRBUFSIZE 412

#ifndef CONFIG_H
#define CONFIG_H
typedef struct serverconfig{
	char *dbName;
	char *dbUsername;
	char *dbPassword;
	int port;
} serverconfig;
#endif 
serverconfig *load_serverconfig_from_file(char*);
void free_serverconfig(serverconfig*);
