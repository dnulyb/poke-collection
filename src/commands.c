
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "commands.h"
#include "db.h"
#include "data_retrieval.h"
#include "linked_list.h"



void test(){


}

//argv[2] contains the set id
//argv[3] ... argv[n] contains the card numbers
void add_to_collection(int argc, char *argv[]){

    int ncards = argc-2;
    if(ncards <= 0){
        printf("No cards to add, exiting...\n");
        return;
    }

    int ncards_added = 0;
    char *set_id = argv[2]; //TODO: Check if set exists before trying to add cards
    
    int res = 0;
    for(int i = 3; i < argc; i++){

        char *number = argv[i];

        ll_node *head = check_card_exists(set_id, number);
        if(strcmp("1", head->data) != 0){
            printf("Card does not exist in db.\n");
        }else{
            res = set_card_owned(set_id, argv[i], true);
            if(res == 0){
                ncards_added++;
            }
        }

        list_delete(head);

    }

    printf("%d cards successfully marked as collected.\n", ncards_added);


}

void remove_from_collection(int argc, char *argv[]){

}

void set_missing(char *set){

}

void set_collected(char *set){

}



void perform_command(int argc, char *argv[]){


    //argv[1] contains the command, argv[2] ... argv[n] contains the command arguments
    char *command = argv[1];

    if(strcmp(command, "add") == 0){

        add_to_collection(argc, argv);

    }else if(strcmp(command, "remove") == 0){

        fprintf(stderr, "remove command not implemented\n");


        
    }else if(strcmp(command, "collected") == 0){

        fprintf(stderr, "collected command not implemented\n");


    }else if(strcmp(command, "missing") == 0){

        fprintf(stderr, "missing command not implemented\n");


    }else if(strcmp(command, "testdb") == 0){
        printf("Testing db...\n");
        test();
    
    }else{
        printf("ERROR: Invalid command\n");
    }

}