#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include "data_retrieval.h"
#include "db.h"

// Filepaths and urls
static char *key_location = "../keys.txt"; //location of file containing pokemontcg api key
static char *sets_url = "https://api.pokemontcg.io/v2/sets?select=id,name,printedTotal&orderBy=releaseDate";




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

        //Transaction to avoid doing many inserts
        db_exec(db, "BEGIN TRANSACTION;");

        //Insert the data, one set at a time
        cJSON *elem = NULL;
        char *id;
        char *name;
        int ncards; //number of cards printed in the set
        char *query;
        for(int i = 0; i < set_count; i++){

            //Retrieve relevant data
            elem = cJSON_GetArrayItem(data, i);
            id = cJSON_GetObjectItem(elem, "id")->valuestring;
            name = cJSON_GetObjectItem(elem, "name")->valuestring;
            ncards = cJSON_GetObjectItem(elem, "printedTotal")->valueint;
            //printf("Set info: %s | %s | %d\n", id, name, ncards);
            
            //Build query and send to db
            query = sqlite3_mprintf(insert_sets, id, name, ncards);
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

            // Construct the complete header string
            char header[128];
            strcpy(header, "X-Api-Key: ");
            strcat(header, key);

            free(key);

            //Add the custom api header
            headers = curl_slist_append(headers, header);
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
        printf("Total received bytes: %zu\n", req.len);
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
        //return
    }

    if((fgets(key, 128, fp)) == NULL) {
        fprintf(stderr, "fgets: Error reading API key\n");
        //return;
    }

    //printf("length of string: %ld\n", strlen(buffer));

    fclose(fp);

    return key;

}