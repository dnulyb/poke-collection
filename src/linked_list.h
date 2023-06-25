#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct ll_node {

    char *data;
    struct ll_node *next;

} ll_node;

ll_node* list_create();
void list_add(ll_node *head, char *data);
void list_delete(ll_node *head);
void list_print(ll_node *head);

#endif