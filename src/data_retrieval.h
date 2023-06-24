#ifndef DATA_RETRIEVAL_H
#define DATA_RETRIEVAL_H

#include <stdbool.h>


//curl custom buffer
typedef struct {
    char *buffer;
    size_t len;
    size_t buflen;
} get_request;


void retrieve_sets();
void get_db_sets();
void retrieve_set_cards(char *set_id);
get_request send_get_request(char *url, bool use_api_key);
char* get_api_key();



#endif