typedef struct linkedlistnode{
	void  *value;
	struct linkedlistnode *next;
}linkedlistnode;

linkedlistnode *create_linkedlistnode(void*);
linkedlistnode *add_first_linkedlistnode(linkedlistnode*, linkedlistnode*);
linkedlistnode *remove_linkedlistnode(linkedlistnode*, void*);
static linkedlistnode *remove_linkedlistnode_r(linkedlistnode*, void*);
void *pop(linkedlistnode**);
