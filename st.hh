/// @file
/// @brief schema and dut classes for ShenTong database

#ifndef ST_HH
#define ST_HH

// 写个我自己的
#include "ProcessHandler.h"

#include "schema.hh"
#include "relmodel.hh"
#include "dut.hh"

struct shentong_connection {
  ProcessHandler handler;
  char *zErrMsg = 0; // 暂时不做错误处理
  int rc;
  void q(const char *query);
  shentong_connection(std::string &conninfo);
  ~shentong_connection();
};

struct schema_shentong : schema, shentong_connection {
  schema_shentong(std::string &conninfo, bool no_catalog);
  virtual std::string quote_name(const std::string &id) {
    return id;
  }
};

struct dut_shentong : dut_base, shentong_connection {
  virtual void test(const std::string &stmt);
  dut_shentong(std::string &conninfo);
};

#endif
