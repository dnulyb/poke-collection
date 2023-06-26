#ifndef DB_H
#define DB_H

#include <sqlite3.h>
#include "linked_list.h"

extern const char *insert_sets;
extern const char *select_set_ids;
extern const char *set_exists;
extern const char *insert_cards;
extern const char *card_owned;
extern const char *card_not_owned;
extern const char *card_exists;
extern const char *select_collected;
extern const char *count_collected;

int callback_single_col(void *list, int ncols, char **data, char **cols);
int select_cards_callback(void *list, int ncols, char **data, char **cols);

sqlite3* db_open();
void db_close(sqlite3 *db);
void db_setup();
int db_exec(sqlite3 *db, const char *query);
void db_exec_callback(sqlite3 *db, const char *query, 
                                int (*cb)(void *, int,  char **, char **), 
                                ll_node *head);


#endif