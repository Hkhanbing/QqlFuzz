#include <stdexcept>
#include <cassert>
#include <cstring>
#include "st.hh"
#include <iostream>

#ifndef HAVE_BOOST_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using boost::regex_match;
#endif

using namespace std;

static regex e_syntax("near \".*\": syntax error");
static regex e_user_abort("callback requested query abort");


extern "C" int my_shentong3_busy_handler(void *, int)
{
  throw std::runtime_error("shentong3 timeout");
}

extern "C" int column_callback(void *arg, int argc, char **argv, char **azColName)
{
  (void) argc; (void) azColName;
  table *tab = (table *)arg;
  column c(argv[1], sqltype::get(argv[2]));
  tab->columns().push_back(c); // 获取所有columns
  return 0;
}

shentong_connection::shentong_connection(std::string &conninfo)
{
  handler.start("/home/hkbin/Workspace/chaitin_workspace/database_fuzz/QqlFuzz/tool");
  std::string input_line;
  handler.executeCommand("select version();\n");
}

void shentong_connection::q(const char *query)
{
  if (!strlen(query)) {
    perror("query is empty");
    handler.stop();
    break;
  }
  query += "\n";
  handler.executeCommand(query);
}

shentong_connection::~shentong_connection()
{
  if (handler)
    handler.stop();
}

// schema_shentong::schema_shentong(std::string &conninfo, bool no_catalog)
//   : shentong_connection(conninfo)
// {
// 	std::string query = "SELECT * FROM main.shentong_master where type in ('table', 'view')";

// 	if (no_catalog)
// 		query+= " AND name NOT like 'shentong_%%'";
  
//   version = "shentong " shentong_VERSION " " shentong_SOURCE_ID;

// //   shentong3_busy_handler(db, my_shentong3_busy_handler, 0);
//   cerr << "Loading tables...";

//   rc = shentong3_exec(db, query.c_str(), table_callback, (void *)&tables, &zErrMsg); // 注意有回调函数
//   if (rc!=shentong_OK) {
//     auto e = std::runtime_error(zErrMsg);
//     shentong3_free(zErrMsg);
//     throw e;
//   }

//   if (!no_catalog)
//   {
// 		// shentong_master doesn't list itself, do it manually
// 		table tab("shentong_master", "main", false, false);
// 		tables.push_back(tab); // 导入一张表
//   }
  
//   cerr << "done." << endl;

//   cerr << "Loading columns and constraints...";

//   for (auto t = tables.begin(); t != tables.end(); ++t) {
//     string q("pragma table_info(");
//     q += t->name;
//     q += ");"; // 这里是获取这个表中的columns的类型

//     rc = shentong3_exec(db, q.c_str(), column_callback, (void *)&*t, &zErrMsg);
//     if (rc!=shentong_OK) {
//       auto e = std::runtime_error(zErrMsg);
//       shentong3_free(zErrMsg);
//       throw e;
//     }
//   }

//   cerr << "done." << endl;

// #define BINOP(n,t) do {op o(#n,sqltype::get(#t),sqltype::get(#t),sqltype::get(#t)); register_operator(o); } while(0)

//   BINOP(||, TEXT);  // 开始注册operator
//   BINOP(*, INTEGER);
//   BINOP(/, INTEGER);

//   BINOP(+, INTEGER);
//   BINOP(-, INTEGER);

//   BINOP(>>, INTEGER);
//   BINOP(<<, INTEGER);

//   BINOP(&, INTEGER);
//   BINOP(|, INTEGER);

//   BINOP(<, INTEGER);
//   BINOP(<=, INTEGER);
//   BINOP(>, INTEGER);
//   BINOP(>=, INTEGER);

//   BINOP(=, INTEGER);
//   BINOP(<>, INTEGER);
//   BINOP(IS, INTEGER);
//   BINOP(IS NOT, INTEGER);

//   BINOP(AND, INTEGER);
//   BINOP(OR, INTEGER);
  
// #define FUNC(n,r) do {							\
//     routine proc("", "", sqltype::get(#r), #n);				\
//     register_routine(proc);						\
//   } while(0)

// #define FUNC1(n,r,a) do {						\
//     routine proc("", "", sqltype::get(#r), #n);				\
//     proc.argtypes.push_back(sqltype::get(#a));				\
//     register_routine(proc);						\
//   } while(0)

// #define FUNC2(n,r,a,b) do {						\
//     routine proc("", "", sqltype::get(#r), #n);				\
//     proc.argtypes.push_back(sqltype::get(#a));				\
//     proc.argtypes.push_back(sqltype::get(#b));				\
//     register_routine(proc);						\
//   } while(0)

// #define FUNC3(n,r,a,b,c) do {						\
//     routine proc("", "", sqltype::get(#r), #n);				\
//     proc.argtypes.push_back(sqltype::get(#a));				\
//     proc.argtypes.push_back(sqltype::get(#b));				\
//     proc.argtypes.push_back(sqltype::get(#c));				\
//     register_routine(proc);						\
//   } while(0)
//   // 开始注册function
//   FUNC(last_insert_rowid, INTEGER);
//   FUNC(random, INTEGER);
//   FUNC(shentong_source_id, TEXT);
//   FUNC(shentong_version, TEXT);
//   FUNC(total_changes, INTEGER);

//   FUNC1(abs, INTEGER, REAL);
//   FUNC1(hex, TEXT, TEXT);
//   FUNC1(length, INTEGER, TEXT);
//   FUNC1(lower, TEXT, TEXT);
//   FUNC1(ltrim, TEXT, TEXT);
//   FUNC1(quote, TEXT, TEXT);
//   FUNC1(randomblob, TEXT, INTEGER);
//   FUNC1(round, INTEGER, REAL);
//   FUNC1(rtrim, TEXT, TEXT);
//   FUNC1(soundex, TEXT, TEXT);
//   FUNC1(shentong_compileoption_get, TEXT, INTEGER);
//   FUNC1(shentong_compileoption_used, INTEGER, TEXT);
//   FUNC1(trim, TEXT, TEXT);
//   FUNC1(typeof, TEXT, INTEGER);
//   FUNC1(typeof, TEXT, NUMERIC);
//   FUNC1(typeof, TEXT, REAL);
//   FUNC1(typeof, TEXT, TEXT);
//   FUNC1(unicode, INTEGER, TEXT);
//   FUNC1(upper, TEXT, TEXT);
//   FUNC1(zeroblob, TEXT, INTEGER);

//   FUNC2(glob, INTEGER, TEXT, TEXT);
//   FUNC2(instr, INTEGER, TEXT, TEXT);
//   FUNC2(like, INTEGER, TEXT, TEXT);
//   FUNC2(ltrim, TEXT, TEXT, TEXT);
//   FUNC2(rtrim, TEXT, TEXT, TEXT);
//   FUNC2(trim, TEXT, TEXT, TEXT);
//   FUNC2(round, INTEGER, REAL, INTEGER);
//   FUNC2(substr, TEXT, TEXT, INTEGER);

//   FUNC3(substr, TEXT, TEXT, INTEGER, INTEGER);
//   FUNC3(replace, TEXT, TEXT, TEXT, TEXT);

//   // 开始注册内聚函数

// #define AGG(n,r, a) do {						\
//     routine proc("", "", sqltype::get(#r), #n);				\
//     proc.argtypes.push_back(sqltype::get(#a));				\
//     register_aggregate(proc);						\
//   } while(0)
//   // 内聚函数
//   AGG(avg, INTEGER, INTEGER);
//   AGG(avg, REAL, REAL);
//   AGG(count, INTEGER, REAL);
//   AGG(count, INTEGER, TEXT);
//   AGG(count, INTEGER, INTEGER);
//   AGG(group_concat, TEXT, TEXT);
//   AGG(max, REAL, REAL);
//   AGG(max, INTEGER, INTEGER);
//   AGG(sum, REAL, REAL);
//   AGG(sum, INTEGER, INTEGER);
//   AGG(total, REAL, INTEGER);
//   AGG(total, REAL, REAL);
// // TODO 这里没看懂后面那几个 
//   // booltype = sqltype::get("INTEGER");
//   // inttype = sqltype::get("INTEGER");

//   // internaltype = sqltype::get("internal");
//   // arraytype = sqltype::get("ARRAY");

//   true_literal = "1";
//   false_literal = "0";
//   // 这里疑似PG特有types才能进入 别的只会找base table
//   generate_indexes();
//   shentong3_close(db);
//   db = 0;
// }

// dut_shentong::dut_shentong(std::string &conninfo)
//   : shentong_connection(conninfo)
// {
//   q("PRAGMA main.auto_vacuum = 2");
// }

// extern "C" int dut_callback(void *arg, int argc, char **argv, char **azColName)
// {
//   (void) arg; (void) argc; (void) argv; (void) azColName;
//   return shentong_ABORT;
// }

// void dut_shentong::test(const std::string &stmt)
// {
//   alarm(6);
//   rc = shentong3_exec(db, stmt.c_str(), dut_callback, 0, &zErrMsg);
//   if( rc!=shentong_OK ){
//     try {
//       if (regex_match(zErrMsg, e_syntax))
// 	throw dut::syntax(zErrMsg);
//       else if (regex_match(zErrMsg, e_user_abort)) {
// 	shentong3_free(zErrMsg);
// 	return;
//       } else 
// 	throw dut::failure(zErrMsg);
//     } catch (dut::failure &e) {
//       shentong3_free(zErrMsg);
//       throw;
//     }
//   }
// }

