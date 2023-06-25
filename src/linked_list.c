
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "linked_list.h"


ll_node* list_create(){
    ll_node *head = malloc(sizeof(ll_node));
    head->data = NULL;
    head->next = NULL;
    return head;
}

void list_add(ll_node *head, char *data){

    // If there is room in head, add data there
    if(head->data == NULL){
        head->data = data;
        return;
    }


    ll_node *new_node = malloc(sizeof(ll_node));
    new_node->data = data;
    new_node->next = NULL;

    ll_node *temp = head;
    while(temp->next != NULL){
        temp = temp->next;
    }
    temp->next = new_node;

}

// Frees the list and all data in the list
void list_delete(ll_node *head){

    if(head->next != NULL){
        list_delete(head->next);
    }

    free(head->data);
    free(head);

}

void list_print(ll_node *head){

    if(head == NULL){
        return;
    }

    printf("%s\n", head->data);
    list_print(head->next);
}

