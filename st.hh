/// @file
/// @brief schema and dut classes for ShenTong database

#ifndef ST_HH
#define ST_HH

// 写个我自己的
extern "C"  {
#include <sqlite3.h>
}

#include "schema.hh"
#include "relmodel.hh"
#include "dut.hh"

struct sqlite_connection {
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  void q(const char *query);
  sqlite_connection(std::string &conninfo);
  ~sqlite_connection();
};

struct schema_sqlite : schema, sqlite_connection {
  schema_sqlite(std::string &conninfo, bool no_catalog);
  virtual std::string quote_name(const std::string &id) {
    return id;
  }
};

struct dut_sqlite : dut_base, sqlite_connection {
  virtual void test(const std::string &stmt);
  dut_sqlite(std::string &conninfo);
};

#endif
