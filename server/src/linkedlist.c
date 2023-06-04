#include <stdlib.h>
#include "linkedlist.h"

linkedlistnode *create_linkedlistnode(void *value){
	linkedlistnode *node = malloc(sizeof(linkedlistnode));
	if (node == NULL) return NULL;
	node->value = value;
	node->next = NULL;
	return node;
}

linkedlistnode *add_linkedlistnode(linkedlistnode* root, linkedlistnode* node){
	if (root == NULL) return node;
	else if (node == NULL) return root;
	node->next = root;
	return node;
}

linkedlistnode *remove_linkedlistnode(linkedlistnode *node, void *value){
	if (node == NULL) return NULL;
	if (node->value == value){
		linkedlistnode *nodeToReturn = node->next;
		free(value);
		free(node);
		return nodeToReturn;
	}
	else if (node->next != NULL){
		if (node->next->value == value){
			linkedlistnode *tmp = node->next->next;
			free(node->next->value);
			free(node->next);
			node->next = tmp;
		}
		else node->next->next = 
			remove_linkedlistnode_r(node->next, value);
	}
	return node;
}

static linkedlistnode *remove_linkedlistnode_r(linkedlistnode *node, void *value){
	if (node->next != NULL){
		if (node->next->value == value){
			linkedlistnode *nodeToDelete = node->next;
			linkedlistnode *nodeToReturn = nodeToDelete->next;
			free(nodeToDelete->value);
			free(nodeToDelete);
			return nodeToReturn;
		}
		else node->next = remove_linkedlistnode_r(node->next, value);
	}
	return node->next;
}

void *pop(linkedlistnode **rootPtr){
	linkedlistnode *tmp = *rootPtr;
	void *value = tmp->value;
	free(tmp);
	*rootPtr = (*rootPtr)->next;
	return value;	
}
