#define PORT_UNSPECIFIED -1
#define LINEPTRBUFSIZE 412

typedef struct serverconfig{
	char *dbName;
	char *dbUsername;
	char *dbPassword;
	int port;
} serverconfig;

serverconfig *load_serverconfig_from_file(char*);
void free_serverconfig(serverconfig*);
