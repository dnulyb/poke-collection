
#include <string.h>
#include <stdio.h>
#include "commands.h"
#include "db.h"



void test(){

    db_test();

}

void add_to_collection(int argc, char *argv[]){

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

        fprintf(stderr, "add command not implemented\n");

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