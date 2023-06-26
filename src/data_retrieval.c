/*
    TODO: Currently cannot download sets of cards larger than 250 cards,
            since the pokemontcg api page size is max 250.
            To fix, implement page 2 (and 3, 4, ... n) parsing if 
                more than 250 cards.


    Note: Price data is unreliable for cards with more than 1 version,
            since the pokemontcg price data url is the wrong version for
            many cards.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "data_retrieval.h"
#include "db.h"





// Filepaths and urls
static char *key_location = "../keys.txt"; //location of file containing pokemontcg api key
static char *sets_url = "https://api.pokemontcg.io/v2/sets?select=id,name,printedTotal,total,releaseDate&orderBy=releaseDate";
//Append the set id to set_cards_url to get all cards for that set
static char *set_cards_url = "https://api.pokemontcg.io/v2/cards?select=number,name,cardmarket&q=set.id:"; 


#define CHUNK_SIZE 2048

/*
    ptr:        pointer to the data
    size:       size of each member
    nmemb:      number of members
    userdata:   pointer to the current request

    this function will write the data from ptr to the userdata get_request buffer.
*/
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb; 
    get_request *req = (get_request *) userdata;

    //printf("receive chunk of %zu bytes\n", realsize);

    while (req->buflen < req->len + realsize + 1)
    {
        req->buffer = realloc(req->buffer, req->buflen + CHUNK_SIZE);

        if(req->buffer == NULL){
            fprintf(stderr, "curl callback out of memory\n");
            return 0;  /* out of memory! */

        }

        req->buflen += CHUNK_SIZE;
    }
    memcpy(&req->buffer[req->len], ptr, realsize);
    req->len += realsize;
    req->buffer[req->len] = 0;

    return realsize;
}

char* sets_query_from_json(cJSON *data, int i){

    cJSON *elem = NULL;
    cJSON *id = NULL;
    char *id_value;
    cJSON *name = NULL;
    char *name_value;
    cJSON *ncards_printed = NULL;
    int ncards_printed_value;
    cJSON *ncards_total = NULL;
    int ncards_total_value;
    cJSON *release_date = NULL;
    char *release_date_value;
    char *query;

    //reset variables
    id_value = NULL;
    name_value = NULL;
    ncards_printed_value = 0; //set size printed on the cards
    ncards_total_value = 0; //total number of cards printed in the set
    release_date_value = NULL;

    //Retrieve relevant data
    elem = cJSON_GetArrayItem(data, i);
    if(elem == NULL){
        //Failed to get current card, so skip it
        fprintf(stderr, "retrieve_sets failed to get set number %d\n", i);
        return NULL;
    }

    //Make sure id exists
    id = cJSON_GetObjectItem(elem, "id");
    if(id != NULL){
        id_value = id->valuestring;
    }

    //Make sure name exists
    name = cJSON_GetObjectItem(elem, "name");
    if(name != NULL){
        name_value = name->valuestring;
    }

    //Make sure ncards_printed exists
    ncards_printed = cJSON_GetObjectItem(elem, "printedTotal");
    if(ncards_printed != NULL){
        ncards_printed_value = ncards_printed->valueint;
    }

    //Make sure ncards_total exists
    ncards_total = cJSON_GetObjectItem(elem, "total");
    if(ncards_total != NULL){
        ncards_total_value = ncards_total->valueint;
    }

    //Make sure release_date exists
    release_date = cJSON_GetObjectItem(elem, "releaseDate");
    if(release_date != NULL){
        release_date_value = release_date->valuestring;
    }

    //printf("Set info: %s | %s | %d\n", id, name, ncards);
    
    //Build query and send to db
    query = sqlite3_mprintf(insert_sets, id_value, name_value,
                                ncards_printed_value, ncards_total_value,
                                release_date_value);

    return query;

}

void retrieve_sets(){

    //Get all sets as JSON from the url
    get_request result = send_get_request(sets_url, true);
    cJSON *root = cJSON_Parse(result.buffer);

    if(root == NULL){
        fprintf(stderr, "cJSON parsing failed: Invalid JSON.\n");
        //Cleanup
        cJSON_Delete(root);
        free(result.buffer);
        return;
    }

    //Find data object
    cJSON *data = NULL;
    data = cJSON_GetObjectItem(root, "data");
    
    if(data == NULL){
        fprintf(stderr, "cJSON parsing failed for retrieve_sets: 'data' not found.\n");
        //Cleanup
        cJSON_Delete(root);
        free(result.buffer);
        return;
    }
    
    int set_count = cJSON_GetArraySize(data);


    // Store data in sqlite database
    sqlite3 *db = db_open();
    if(db != NULL){

        //Transaction to speedup doing many inserts
        db_exec(db, "BEGIN TRANSACTION;");

        //Loop through and store each set
        for(int i = 0; i < set_count; i++){
        
            char *query = sets_query_from_json(data, i);
            db_exec(db, query);
            sqlite3_free(query);

        }

        db_exec(db, "END TRANSACTION;");
        db_close(db);
    }

    
    //Cleanup
    cJSON_Delete(root);
    free(result.buffer);

}

//Caller is responsible for freeing list memory
//  with list_delete
ll_node* get_db_data(const char *query, 
                        int (*cb)(void *, int,  char **, char **)){

    sqlite3 *db = db_open();
    ll_node *head = list_create();

    if(db != NULL){

        db_exec_callback(db, query, cb, head);
    }

    db_close(db);

    return head;

}

//Return: 0 if card exists, else -1 
int check_set_exists(char *set_id){

    sqlite3 *db = db_open();
    ll_node *head = list_create();
    char *query;

    if(db != NULL){

        query = sqlite3_mprintf(set_exists, set_id);
        db_exec_callback(db, query, callback_single_col, head);
        sqlite3_free(query);
    }

    db_close(db);

    if(head->data != NULL){
        if(strcmp("1", head->data) == 0){
            //Set exists
            list_delete(head);
            return 0;
        }
    }

    list_delete(head);

    return -1;
}

char* cards_query_from_json(cJSON *data, int i, char *set_id){

    cJSON *elem = NULL;
    //We already have the set id as parameter, so no need to fetch it from url
    cJSON *number = NULL; 
    char *number_value;
    cJSON *name = NULL;
    char *name_value;
    //Cardmarket link from the retrieved json can be for the wrong
    //  version of the card (very common with base set cards for example).
    //  This means that the price data can be unreliable and should be double-checked
    //      manually especially for (seemingly) valuable cards.
    cJSON *cardmarket = NULL;
    cJSON *prices = NULL;
    cJSON *avg_price = NULL;
    double avg_price_value;
    char *query;

    //reset variables
    number_value = NULL;
    name_value = NULL;
    avg_price_value = 0.0f;

    //Retrieve relevant data
    elem = cJSON_GetArrayItem(data, i);
    if(elem == NULL){
        //Failed to get current card, so skip it
        fprintf(stderr, "retrieve_sets failed to get card number %d for set %s\n", i, set_id);
        return NULL;
    }

    //Make sure number exists
    number = cJSON_GetObjectItem(elem, "number");
    if(number != NULL){
        number_value = number->valuestring;
    }

    //Make sure name exists
    name = cJSON_GetObjectItem(elem, "name");
    if(name != NULL){
        name_value = name->valuestring;
    }

    //Make sure avg_price exists
    cardmarket = cJSON_GetObjectItem(elem, "cardmarket");
    if(cardmarket != NULL){
        prices = cJSON_GetObjectItem(cardmarket, "prices");
        if(prices != NULL){
            avg_price = cJSON_GetObjectItem(prices, "avg30");
            if(avg_price != NULL){
                avg_price_value = avg_price->valuedouble;
            }
        }
    }
    
    //Build query and send to db
    query = sqlite3_mprintf(insert_cards, set_id, number_value,
                                name_value, 0, avg_price_value);

    return query;

}

void retrieve_set_cards(char *set_id){

    //Build the complete url
    char set_cards_url_complete[128];
    strcpy(set_cards_url_complete, set_cards_url);
    strcat(set_cards_url_complete, set_id);

    //Get all sets as JSON from the url
    get_request result = send_get_request(set_cards_url_complete, true);
    //printf("Received data in retrieve_sets: \n%s\n", result.buffer);
    cJSON *root = cJSON_Parse(result.buffer);

    if(root == NULL){
        fprintf(stderr, "cJSON parsing failed for set %s: Invalid JSON.\n", set_id);
        //Cleanup
        cJSON_Delete(root);
        free(result.buffer);
        return;
    }

    //int n = cJSON_GetArraySize(root);
    cJSON *data = NULL;
    data = cJSON_GetObjectItem(root, "data");

    if(data == NULL){
        fprintf(stderr, "cJSON parsing failed for set %s: 'data' not found.\n", set_id);
        //Cleanup
        cJSON_Delete(root);
        free(result.buffer);
        return;
    }

    int card_count = cJSON_GetArraySize(data);
    //printf("set count in JSON: %d\n", set_count);


    // Store data in sqlite database
    sqlite3 *db = db_open();
    if(db != NULL){

        //Transaction to speedup doing many inserts
        db_exec(db, "BEGIN TRANSACTION;");

        //Insert the data, one set at a time
        for(int i = 0; i < card_count; i++){

            char *query = cards_query_from_json(data, i, set_id);
            db_exec(db, query);
            sqlite3_free(query);

        }

        db_exec(db, "END TRANSACTION;");
        db_close(db);
    }

    
    //Cleanup
    cJSON_Delete(root);
    free(result.buffer);

}

/*
    Marks the card matching (set_id, number) as owned or not owned

    Example: 
        set_card_owned("swsh1", "6", true);
        set_card_owned("swsh1", "6", false);
*/
int set_card_owned(char *set_id, char *number, bool owned){

    sqlite3 *db = db_open();
    char *query; 
    int res = 0;

    if(db != NULL){


        if(owned){
            query = sqlite3_mprintf(card_owned, set_id, number);
        }else{
            query = sqlite3_mprintf(card_not_owned, set_id, number);
        }
        res = db_exec(db, query);
        sqlite3_free(query);


    }

    db_close(db);

    return res;

}

//Return: 0 if card exists, else -1 
int check_card_exists(char *set_id, char *number){

    sqlite3 *db = db_open();
    ll_node *head = list_create();
    char *query;

    if(db != NULL){

        query = sqlite3_mprintf(card_exists, set_id, number);
        db_exec_callback(db, query, callback_single_col, head);
        sqlite3_free(query);
    }

    db_close(db);

    if(head->data != NULL){
        if(strcmp("1", head->data) == 0){
            //Card exists
            return 0;
        }
    }

    return -1;
}



/* 
    Send GET request to the provided url
    
    param:  url          - the url for the request
            use_api_key  - true if api key should be put in the 
                            request header, otherwise false 
    return: The data received from the request, in a buffer.
                note: buffer must be free'd by the caller  
*/
get_request send_get_request(char *url, bool use_api_key){

    //Setup request
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    get_request req = {.buffer = NULL, .len = 0, .buflen = 0};

    if(curl){

        //Setup curl header

        struct curl_slist *headers = NULL;
        //Remove unnecessary default curl header
        headers = curl_slist_append(headers, "Accept:");

        // Add API key to header if required
        if(use_api_key){

            // Get the API key as string 
            char *key = get_api_key();
            if(key == NULL){
                //Failed to get api key
                fprintf(stderr, "Failed to get api key from file, " \
                                    "proceeding without api key.\n");
            }else{

                // Construct the complete header string
                char header[128];
                strcpy(header, "X-Api-Key: ");
                strcat(header, key);

                free(key);

                //Add the custom api header
                headers = curl_slist_append(headers, header);
            }

            
        }

        
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        //Add url
        curl_easy_setopt(curl, CURLOPT_URL, url);

        //Debugging verbose
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Setup write to custom buffer
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);

        // Send the request
        res = curl_easy_perform(curl);

        // Check for errors
        if(res != CURLE_OK){
            fprintf(stderr, "retrieve_sets() curl request failed: %s\n", curl_easy_strerror(res));
        }

        // The received data is now stored in req.buffer

        //printf("curl request result: %u\n", res);
        printf("send_get_request received this many bytes: %zu\n", req.len);
        //printf("Received data: \n%s\n", req.buffer);

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return req;

}

char* get_api_key(){

    char *key = malloc(sizeof(char) * 128);
    FILE *fp;
    
    fp = fopen(key_location, "r");
    if(fp == NULL){
        fprintf(stderr, "fopen: Error opening file containing API key\n");
        return NULL;
    }

    if((fgets(key, 128, fp)) == NULL) {
        fprintf(stderr, "fgets: Error reading API key\n");
        return NULL;
    }

    //printf("length of string: %ld\n", strlen(buffer));

    fclose(fp);

    return key;

}