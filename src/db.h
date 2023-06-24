#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include "linked_list.h"

extern const char *insert_sets;
extern const char *select_set_ids;
extern const char *insert_cards;

int select_callback(void *list, int ncols, char **data, char **cols);

sqlite3* db_open();
void db_close(sqlite3 *db);
void db_setup();
void db_exec(sqlite3 *db, const char *query);
void db_exec_select_callback(sqlite3 *db, const char *query, ll_node *head);



#endif