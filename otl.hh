#include "otlv4.h"
#include <stdlib.h>
#include <iostream>

// init_db
otl_connect* init_db();

// get url and db
void rlogin(char *url, otl_connect *db);

// command exec
void db_exec(otl_connect *db, char *sql, otl_stream *otlCur);
