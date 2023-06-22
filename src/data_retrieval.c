#include <curl/curl.h>
#include <cjson/cJSON.h>


void test_data_retrieval(){

    printf("test_data_retrieval start..\n");
    CURL *curl = curl_easy_init();
    cJSON *json = cJSON_Parse("this is just a test, not a json file");
    printf("test_data_retrieval complete.\n");

}