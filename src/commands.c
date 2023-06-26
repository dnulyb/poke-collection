
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "commands.h"
#include "db.h"
#include "data_retrieval.h"

void cards_download_all(){

    ll_node *head = get_db_data(select_set_ids, callback_single_col);
    ll_node *temp = head;
    while(temp != NULL){

        retrieve_set_cards(temp->data);
        printf("set cards downloaded: %s\n", temp->data);
        temp = temp->next;
    }
    list_delete(head);

}

void cards_download_single_set(char *set_id){

    //Check if set exists in db
    if(check_set_exists(set_id) != 0){
        //Set does not exist
        fprintf(stderr, "Error: Cards not downloaded: Set does not exist in db.\n");
        return;
    }

    retrieve_set_cards(set_id);
    printf("set cards downloaded: %s\n", set_id);

}

/*
    Full setup of the database.
    Creates the .db file and downloads all card data from all sets.
*/
void setup_full(){

    printf("setup_full started:\n");

    //Create db tables
    db_setup(); 
    printf("db tables created.\n");
    //Get info for all sets
    retrieve_sets();
    printf("set data downloaded.\n");

    //Get card data for each set
    printf("downloading cards from sets:\n");
    cards_download_all();
    printf("cards download complete.\n");

    printf("setup_full complete.\n");
    
}

/*
    Lite setup of the database.
    Will create all tables,
        but only downloads Sets data and no Cards data.
*/
void setup_lite(){

    printf("setup_lite started:\n");

    //Create db tables
    db_setup(); 
    printf("db tables created.\n");
    //Get info for all sets
    retrieve_sets();
    printf("set data downloaded.\n");
    printf("card data will not be downloaded," \
            " it must be done manually with a separate command.\n");

    printf("setup_lite complete.\n");

}

void collected(char *set_id){

    //Check if set exists in db
    if(check_set_exists(set_id) != 0){
        //Set does not exist
        fprintf(stderr, "Error: Cannot display collected cards: Set does not exist in db.\n");
        return;
    }

    //Make sure there is data to collect
    char *check_query = sqlite3_mprintf(count_collected, set_id);
    ll_node *check = get_db_data(check_query, callback_single_col);
    sqlite3_free(check_query);

    if(strcmp(check->data, "0") == 0){
        printf("No collected cards for set %s.\n", set_id);
        list_delete(check);
        return;
    }

    char *query = sqlite3_mprintf(select_collected, set_id);
    ll_node *head = get_db_data(query, select_cards_callback);
    ll_node *temp = head;

    printf("Collected cards for set %s:\n", set_id);
    while(temp != NULL){
        printf("%s\n", temp->data);
        temp = temp->next;
    }
    printf("End of collected cards for set %s.\n", set_id);
    list_delete(check);
    list_delete(head);
    sqlite3_free(query);


}

//argv[2] contains the set id
//argv[3] ... argv[n] contains the card numbers
void set_collected_status(int argc, char *argv[], bool collected){

    int ncards = argc-2;
    if(ncards <= 0){
        if(collected){
            printf("No cards to add, exiting...\n");
        }else{
            printf("No cards to remove, exiting...\n");
        }
        return;
    }

    int ncards_added = 0;
    char *set_id = argv[2]; //TODO: Check if set exists before trying to add cards
    
    int res = 0;
    for(int i = 3; i < argc; i++){

        char *number = argv[i];

        //Make sure card exists in db
        if(check_card_exists(set_id, number) != 0){
            printf("Card does not exist in db.\n");
        }else{
            res = set_card_owned(set_id, argv[i], collected);
            if(res == 0){
                ncards_added++;
            }
        }

    }

    if(collected){
        printf("%d cards successfully marked as collected.\n", ncards_added);
    }else{
        printf("%d cards successfully marked as removed.\n", ncards_added);
    }

}


void perform_command(int argc, char *argv[]){


    //argv[1] contains the command, argv[2] ... argv[n] contains the command arguments
    char *command = argv[1];

    if(strcmp(command, "add") == 0){

        set_collected_status(argc, argv, true);

    }else if(strcmp(command, "remove") == 0){

        set_collected_status(argc, argv, false);

    }else if(strcmp(command, "collected") == 0){

        if(argv[2] == NULL){
            fprintf(stderr, "No set given, exiting...\n");
            return;
        }
        collected(argv[2]);

    }else if(strcmp(command, "missing") == 0){

        fprintf(stderr, "missing command not implemented\n");

    }else if(strcmp(command, "setup_full") == 0){
        
        setup_full();
    
    }else if(strcmp(command, "setup_lite") == 0){

        setup_lite();

    }else if(strcmp(command, "download_cards_all") == 0){

        cards_download_all();

    }else if(strcmp(command, "download_cards_set") == 0){

        if(argv[2] == NULL){
            fprintf(stderr, "No set given, exiting...\n");
            return;
        }
        cards_download_single_set(argv[2]);

    }else{
        fprintf(stderr, "ERROR: Invalid command. Exiting...\n");
    }

}

