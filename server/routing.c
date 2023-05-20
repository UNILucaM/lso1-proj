#include <stdlib.h>
import "route.h"

/*Implementazione basata su ABR.
Si usa il risultato di strcmp sul valore path
per i confronti.*/

route* create_route(char* path, void (*request_handler)(char*)){
	route *route = malloc(sizeof(route));
	if (route == NULL) return;
	route->path = path;
	route->request_handler = request_handler;
	route->left = NULL;
	route->right = NULL;
	return route;
}

route* add_route(route* root, route* node){
	if (root == NULL)
	{
		root = node;
		return root;
	}
	int strcmpresult = strcmp(root->path, node->path);
	if (strcmpresult == 0) return root;
	else if (strcmpresult < 0)
		root->left = add_route(root->left, node);
	else root->right = add_route(root->right, node);
	return root;
}

route* search(route* root, char* path){
	if (root == NULL) return NULL;
	int strcmpresult = strcmp(root->path, path);
	if (strcmpresult == 0) return root;
	else if (strcmpresult < 0)
		return search(root->left, path);
	else return search(root->right, path);                                	
}
