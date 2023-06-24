#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "db.h"

static const char *create_sets = "CREATE TABLE Sets(" \
                                "Id TEXT PRIMARY KEY," \
                                "Name TEXT," \
                                "NCardsPrinted INTEGER,"
                                "NCardsTotal INTEGER);";

const char *insert_sets = "INSERT INTO Sets VALUES(%Q,%Q,'%d','%d');";
const char *select_set_ids = "SELECT Id FROM Sets";



static const char *create_cards = "CREATE TABLE Cards(" \
                                "Id TEXT PRIMARY KEY," \
                                "Name TEXT," \
                                "Number TEXT," \
                                "Collected BOOL," \
                                "Price REAL);";

const char *insert_cards = "INSERT INTO Cards VALUES (%Q,%Q,%Q,%d,'%.2f');";


int select_callback(void *list, int ncols, char **data, char **cols){

    ll_node *head = list;

    char *buffer = malloc(sizeof(char) * 32);
    strcpy(buffer, data[0]);


    list_add(head, buffer);

    return 0;
    
}




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

    //Create Sets table
    rc = sqlite3_exec(db, create_sets, 0, 0, &err_msg);

    if(rc){
        fprintf(stderr, "Failed create_sets in db_setup: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg);        
    }

    //Create Cards table
    rc = sqlite3_exec(db, create_cards, 0, 0, &err_msg);

    if(rc){
        fprintf(stderr, "Failed create_cards in db_setup: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg);        
    }

    db_close(db);
}

//Does not open or close the db
void db_exec(sqlite3 *db, const char *query){

    int rc;
    char *err_msg = NULL;

    rc = sqlite3_exec(db, query, 0, 0, &err_msg);

    //printf("db_exec trying to exec the following query: %s\n", query);

    if(rc){
        fprintf(stderr, "Failed sqlite3_exec in db_exec: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg);        
    }

}

//Does not open or close the db
//Will get data from db and add to provided linked list
void db_exec_select_callback(sqlite3 *db, const char *query, ll_node *head){

    int rc;
    char *err_msg = NULL;

    rc = sqlite3_exec(db, query, select_callback, head, &err_msg);

    //printf("db_exec trying to exec the following query: %s\n", query);

    if(rc){
        fprintf(stderr, "Failed sqlite3_exec in db_exec_callback: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg);        
    }

}