#include "otl.hh"
#include <iostream>
using namespace std;

// init database
otl_connect* init_db(){
    otl_connect::otl_initialize();
    otl_connect* db = new otl_connect;
    return db;
}
// get url and db
void rlogin(char *url, otl_connect *db){
    otl_connect::otl_initialize();

    try{
        db->rlogon(url);
        printf("Connected to database\n");

    }
    catch(otl_exception& p)
    {
        cerr<<p.msg<<endl;
        return;
    }
}

// command exec
void db_exec(otl_connect *db, char *sql, otl_stream *otlCur){
    try{
        otl_stream otlCur(1, sql, *db);
        return ;
    }
    catch(otl_exception& p)
    {
        cerr<<p.msg<<endl;
        return ;
    }
}