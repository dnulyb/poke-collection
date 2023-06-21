#include <stdio.h>
#include <sqlite3.h>
#include "db.h"


int db_test(){


    sqlite3 *db;
    int rc;

    rc = sqlite3_open("test.db", &db);

    if(rc){
        fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
    }else{
        printf("test db opened successfully.\n");
    }

    sqlite3_close(db);

    return 0;

}