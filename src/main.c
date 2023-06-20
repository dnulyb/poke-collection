#include <stdio.h>
#include "commands.h"


int main(int argc, char *argv[]) {


    if(argc < 2){
        printf("No command given, exiting...\n");
        return 0;
    }

    perform_command(argc, argv);

}

