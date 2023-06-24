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


void retrieve_sets(){

    //Get all sets as JSON from the url
    get_request result = send_get_request(sets_url, true);
    //printf("Received data in retrieve_sets: \n%s\n", result.buffer);
    cJSON *root = cJSON_Parse(result.buffer);

    if(root == NULL){
        fprintf(stderr, "cJSON parsing failed: Invalid JSON.\n");
        return;
    }

    //int n = cJSON_GetArraySize(root);
    cJSON *data = NULL;
    data = cJSON_GetObjectItem(root, "data");
    int set_count = cJSON_GetArraySize(data);
    //printf("set count in JSON: %d\n", set_count);


    // Store data in sqlite database
    sqlite3 *db = db_open();
    if(db != NULL){

        //Transaction to speedup doing many inserts
        db_exec(db, "BEGIN TRANSACTION;");

        //Insert the data, one set at a time
        cJSON *elem = NULL;
        char *id;
        char *name;
        int ncards_printed; //set size printed on the cards
        int ncards_total; //total number of cards printed in the set
        char *release_date;
        char *query;
        for(int i = 0; i < set_count; i++){

            //Retrieve relevant data
            elem = cJSON_GetArrayItem(data, i);
            id = cJSON_GetObjectItem(elem, "id")->valuestring;
            name = cJSON_GetObjectItem(elem, "name")->valuestring;
            ncards_printed = cJSON_GetObjectItem(elem, "printedTotal")->valueint;
            ncards_total = cJSON_GetObjectItem(elem, "total")->valueint;
            release_date = cJSON_GetObjectItem(elem, "releaseDate")->valuestring;
            //printf("Set info: %s | %s | %d\n", id, name, ncards);
            
            //Build query and send to db
            query = sqlite3_mprintf(insert_sets, id, name, ncards_printed, ncards_total, release_date);
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
ll_node* get_db_sets(){

    sqlite3 *db = db_open();
    ll_node *head = list_create();

    if(db != NULL){

        db_exec_callback(db, select_set_ids, head);
    }

    db_close(db);

    return head;

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
        fprintf(stderr, "cJSON parsing failed: Invalid JSON.\n");
        return;
    }

    //int n = cJSON_GetArraySize(root);
    cJSON *data = NULL;
    data = cJSON_GetObjectItem(root, "data");
    int card_count = cJSON_GetArraySize(data);
    //printf("set count in JSON: %d\n", set_count);


    // Store data in sqlite database
    sqlite3 *db = db_open();
    if(db != NULL){

        //Transaction to speedup doing many inserts
        db_exec(db, "BEGIN TRANSACTION;");

        //Insert the data, one set at a time
        cJSON *elem = NULL;
        //We already have the set id as parameter, so no need to fetch it from url
        char *number; 
        char *name;
        cJSON *cardmarket;
        cJSON *prices;
        double avg_price;
        char *query;
        for(int i = 0; i < card_count; i++){

            //Retrieve relevant data
            elem = cJSON_GetArrayItem(data, i);
            number = cJSON_GetObjectItem(elem, "number")->valuestring;
            name = cJSON_GetObjectItem(elem, "name")->valuestring;
            cardmarket = cJSON_GetObjectItem(elem, "cardmarket");
            prices = cJSON_GetObjectItem(cardmarket, "prices");
            avg_price = cJSON_GetObjectItem(prices, "avg30")->valuedouble;
            
            //Build query and send to db
            query = sqlite3_mprintf(insert_cards, set_id, number, name, 0, avg_price);
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
*/
void set_card_owned(char *set_id, char *number, bool owned){

    sqlite3 *db = db_open();
    char *query; 


    if(db != NULL){

        if(owned){
            query = sqlite3_mprintf(card_owned, set_id, number);
        }else{
            query = sqlite3_mprintf(card_not_owned, set_id, number);
        }
        db_exec(db, query);

    }

    db_close(db);


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