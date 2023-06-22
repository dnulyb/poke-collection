#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// Filepaths and urls
static char *key_location = "../keys.txt"; //user has to make sure keys.txt exists and contains the api key
static char *set_url = "https://api.pokemontcg.io/v2/sets?select=id,name,printedTotal&orderBy=releaseDate";


void test_data_retrieval(){

    printf("test_data_retrieval start..\n");
    CURL *curl = curl_easy_init();
    cJSON *json = cJSON_Parse("this is just a test, not a json file");
    printf("test_data_retrieval complete.\n");

}

void retrieve_sets(){

    char header[128];
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
    strcpy(header, "X-Api-Key: ");
    strcat(header, key);


    //Setup request
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
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

        // Send the request
        res = curl_easy_perform(curl);

        // Make sure there are no errors
        if(res != CURLE_OK){
            fprintf(stderr, "retrieve_sets() curl request failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

}