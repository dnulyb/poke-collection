#include <stdio.h>
#include "commands.h"
#include "data_retrieval.h"
#include "db.h"


int main(int argc, char *argv[]) {


    /*if(argc < 2){
        printf("No command given, exiting...\n");
        return 0;
    }*/

    //perform_command(argc, argv);

    db_setup();
    retrieve_sets();


}

