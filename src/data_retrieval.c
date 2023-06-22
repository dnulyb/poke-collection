#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// Filepaths and urls
static char *key_location = "../keys.txt"; //user has to make sure keys.txt exists and contains the api key
static char *set_url = "https://api.pokemontcg.io/v2/sets?select=id,name,printedTotal&orderBy=releaseDate";


//curl custom buffer
typedef struct {
    unsigned char *buffer;
    size_t len;
    size_t buflen;
} get_request;

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


void test_data_retrieval(){

    printf("test_data_retrieval start..\n");
    CURL *curl = curl_easy_init();
    cJSON *json = cJSON_Parse("this is just a test, not a json file");
    printf("test_data_retrieval complete.\n");

}

void retrieve_sets(){

    char key[128];
    FILE *fp;

    // Get API key from file
    fp = fopen(key_location, "r");
    if(fp == NULL){
        fprintf(stderr, "fopen: Error opening keys.txt\n");
        return;
    }

    if((fgets(key, 128, fp)) == NULL) {
        fprintf(stderr, "fgets: Error opening keys.txt\n");
        return;
    }

    //printf("length of string: %ld\n", strlen(buffer));

    fclose(fp);

    // Construct the complete header string
    char header[128];
    strcpy(header, "X-Api-Key: ");
    strcat(header, key);


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

        //Add the custom header
        headers = curl_slist_append(headers, header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        //Add url
        curl_easy_setopt(curl, CURLOPT_URL, set_url);

        //Debugging verbose
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Setup write to custom buffer
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&req);

        // Send the request
        res = curl_easy_perform(curl);

        // Make sure there are no errors
        if(res != CURLE_OK){
            fprintf(stderr, "retrieve_sets() curl request failed: %s\n", curl_easy_strerror(res));
        }

        // The received data is now stored in req.buffer

        //printf("curl request result: %u\n", res);
        printf("Total received bytes: %zu\n", req.len);
        //printf("Received data: \n%s\n", req.buffer);
        free(req.buffer);

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

}