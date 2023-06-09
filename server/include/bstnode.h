#include <stdbool.h>
#include <stdarg.h>
/*In questo file viene definito un nodo generico di un BST
(Binary Search Tree). Nel progetto è impiegata sia per
il routing delle richieste che per la memorizzazione
dei loro header e argomenti.*/

#ifndef BSTNODE_H
#define BSTNODE_H
typedef struct bstnode{
	char *key;
	void *value;
	struct bstnode *left;
	struct bstnode *right;
}bstnode;
#endif
bstnode *init_bst(bstnode*, ...);
bstnode *create_bstnode(char*, void*);
bstnode *add_bstnode(bstnode*, bstnode*);
bstnode *search(bstnode*, char*);
void print_bst_keys_inorder(bstnode*);
