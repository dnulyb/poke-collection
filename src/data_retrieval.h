#ifndef DATA_RETRIEVAL_H
#define DATA_RETRIEVAL_H

#include <stdbool.h>
#include "linked_list.h"

//curl custom buffer
typedef struct {
    char *buffer;
    size_t len;
    size_t buflen;
} get_request;


void retrieve_sets();
ll_node* get_db_data(const char *query, 
                        int (*cb)(void *, int,  char **, char **));
void retrieve_set_cards(char *set_id);
int check_set_exists(char *set_id);
int set_card_owned(char *set_id, char *number, bool owned);
int check_card_exists(char *set_id, char *number);
get_request send_get_request(char *url, bool use_api_key);
char* get_api_key();



#endif