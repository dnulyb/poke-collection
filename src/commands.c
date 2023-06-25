
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "commands.h"
#include "db.h"
#include "data_retrieval.h"


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
    ll_node *head = get_db_sets();
    ll_node *temp = head;
    while(temp != NULL){

        retrieve_set_cards(temp->data);
        printf("set cards downloaded: %s\n", temp->data);
        temp = temp->next;
    }
    printf("cards download complete.\n");
    list_delete(head);

    printf("setup_full complete.\n");
    
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

        fprintf(stderr, "collected command not implemented\n");


    }else if(strcmp(command, "missing") == 0){

        fprintf(stderr, "missing command not implemented\n");


    }else if(strcmp(command, "setup_full") == 0){
        
        setup_full();
    
    }else{
        printf("ERROR: Invalid command\n");
    }

}

