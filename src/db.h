#ifndef DB_H
#define DB_H

#include <sqlite3.h>

extern const char *insert_sets;


sqlite3* db_open();
void db_close(sqlite3 *db);
void db_setup();
void db_exec(sqlite3 *db, char *query);



#endif