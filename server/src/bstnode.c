#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

#include "bstnode.h"

/*Implementazione basata su ABR.
Si usa il risultato di strcmp sul valore key
per i confronti.*/

bstnode *create_bstnode(char* key, void* value){
	bstnode* node = malloc(sizeof(bstnode));
	if (node == NULL) return NULL;
	node->key = key;
	node->value = value;
	node->left = NULL;
	node->right = NULL;
	return node;
}

bstnode *add_bstnode(bstnode* root, bstnode* node){
	if (root == NULL)
	{
		root = node;
		return root;
	}
	int strcmpresult = strcmp(root->key, node->key);
	if (strcmpresult == 0) return root;
	else if (strcmpresult < 0)
		root->left = add_bstnode(root->left, node);
	else root->right = add_bstnode(root->right, node);
	return root;
}

bstnode *init_bst(bstnode *node, ...){
	va_list args;
	va_start(args, node);
	bstnode *root = NULL;
	for (bstnode *n = node;
		n != NULL; n = va_arg(args, bstnode*))
		root = add_bstnode(root, n);
	va_end(args);
	return root;	
}

bstnode *search(bstnode* root, char* key){
	if (root == NULL) return NULL;
	int strcmpresult = strcmp(root->key, key);
	if (strcmpresult == 0) return root;
	else if (strcmpresult < 0)
		return search(root->left, key);
	else return search(root->right, key);                                	
}

void print_bst_keys_inorder(bstnode *root){
    if (root != NULL) {
        print_bst_keys_inorder(root->left);
        printf("%s\n", root->key);
        print_bst_keys_inorder(root->right);
    }
}
