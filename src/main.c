#include <stdio.h>
#include "commands.h"
#include "data_retrieval.h"
#include "db.h"
#include "linked_list.h"


int main(int argc, char *argv[]) {


    if(argc < 2){
        printf("No command given, exiting...\n");
        return 0;
    }

    perform_command(argc, argv);
    
    int setup_test = 0;
    if(setup_test > 0){
        db_setup();
        retrieve_sets();
        retrieve_set_cards("swsh1");

        ll_node *head = get_db_sets();
        list_print(head);
        list_delete(head);
    }

    //set_card_owned("swsh1", "6", true);
    //set_card_owned("swsh1", "6", false);

}

