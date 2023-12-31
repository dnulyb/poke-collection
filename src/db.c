#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "db.h"

static const char *db_name = "poke_collection.db";

static const char *create_sets = "CREATE TABLE Sets(" \
                                "ID TEXT NOT NULL PRIMARY KEY," \
                                "Name TEXT," \
                                "NCardsPrinted INTEGER," \
                                "NCardsTotal INTEGER," \
                                "ReleaseDate TEXT);";

const char *insert_sets = "INSERT INTO Sets VALUES(%Q,%Q,'%d','%d',%Q);";
const char *select_set_ids = "SELECT ID FROM Sets ORDER BY ReleaseDate ASC;";
const char *set_exists = "SELECT EXISTS(SELECT 1 FROM Sets " \
                            "WHERE ID = %Q);";


static const char *create_cards = "CREATE TABLE Cards(" \
                                "SetID TEXT NOT NULL," \
                                "Number TEXT NOT NULL," \
                                "Name TEXT," \
                                "Collected BOOL," \
                                "Price REAL," \
                                "PRIMARY KEY (SetID, Number)," \
                                "FOREIGN KEY(SetID) REFERENCES Sets(ID));";

const char *insert_cards = "INSERT INTO Cards VALUES (%Q,%Q,%Q,%d,'%.2f');";
const char *card_owned = "UPDATE Cards SET Collected = 1 " \
                            "WHERE SetId = %Q AND Number = %Q;";
const char *card_not_owned = "UPDATE Cards SET Collected = 0 " \
                            "WHERE SetId = %Q AND Number = %Q;";

const char *card_exists = "SELECT EXISTS(SELECT 1 FROM Cards " \
                            "WHERE SetID = %Q AND Number = %Q);";

const char *count_collected = "SELECT COUNT(*) FROM Cards " \
                                "WHERE SetID = %Q AND Collected = 1;";
const char *select_collected = "SELECT Cards.Number, Sets.NCardsPrinted, Cards.Name " \
                                "FROM Cards " \
                                "INNER JOIN Sets ON Sets.ID = Cards.SetID " \
                                "WHERE SetID = %Q AND Collected = 1 " \
                                "ORDER BY CAST (Cards.Number AS INTEGER) ASC;";


//Only use this with short values for now
int callback_single_col(void *list, int ncols, char **data, char **cols){

    ll_node *head = list;

    char *buffer = malloc(sizeof(char) * 32);
    strcpy(buffer, data[0]);

    list_add(head, buffer);

    return 0;

}

int select_cards_callback(void *list, int ncols, char **data, char **cols){

    ll_node *head = list;

    char *buffer = calloc(sizeof(char), 256);
    for(int i = 0; i < ncols; i++){

        strcat(buffer, data[i]);

        if(i == 0)
            strcat(buffer, "/");
        if(i == 1)
            strcat(buffer, " ");
    }

    list_add(head, buffer);

    return 0;

}




sqlite3* db_open(){

    sqlite3 *db;
    int rc;

    rc = sqlite3_open(db_name, &db);

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
int db_exec(sqlite3 *db, const char *query){

    int rc;
    char *err_msg = NULL;

    rc = sqlite3_exec(db, query, 0, 0, &err_msg);

    //printf("db_exec trying to exec the following query: %s\n", query);

    if(rc){
        fprintf(stderr, "Failed sqlite3_exec in db_exec: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg); 
        return -1;       
    }

    return 0;

}

//Does not open or close the db
//Will get data from db and add to provided linked list
void db_exec_callback(sqlite3 *db, const char *query, 
                                int (*cb)(void *, int,  char **, char **), 
                                ll_node *head){

    int rc;
    char *err_msg = NULL;

    rc = sqlite3_exec(db, query, cb, head, &err_msg);

    //printf("db_exec_callback trying to exec the following query: %s\n", query);

    if(rc){
        fprintf(stderr, "Failed sqlite3_exec in db_exec_exists_callback: %s\n", sqlite3_errmsg(db));
        sqlite3_free(err_msg);   
    }

}