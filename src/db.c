#include <stdio.h>
#include "db.h"

static const char *create_sets = "CREATE TABLE Sets(" \
                                "Id TEXT PRIMARY KEY," \
                                "Name TEXT," \
                                "NCards INTEGER);";

const char *insert_sets = "INSERT INTO Sets VALUES(%Q,%Q,'%d');";



sqlite3* db_open(){

    sqlite3 *db;
    int rc;

    rc = sqlite3_open("test.db", &db);

    if(rc){
        fprintf(stderr, "Failed to open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    return db;

}

void db_close(sqlite3 *db){

    sqlite3_close(db);

}


void db_setup(){

    sqlite3 *db = db_open();
    int rc;
    char *err_msg = NULL;

    //Create tables
    rc = sqlite3_exec(db, create_sets, 0, 0, &err_msg);

    if(rc){
        fprintf(stderr, "Failed sqlite3_exec in db_setup: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg);        
    }

    db_close(db);

}

void db_exec(sqlite3 *db, char *query){

    int rc;
    char *err_msg = NULL;

    rc = sqlite3_exec(db, query, 0, 0, &err_msg);

    //printf("db_exec trying to exec the following query: %s\n", query);

    if(rc){
        fprintf(stderr, "Failed sqlite3_exec in db_exec: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg);        
    }

}