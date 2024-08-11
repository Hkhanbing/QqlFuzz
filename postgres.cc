#include "postgres.hh"
#include "config.h"
#include "otl.hh"
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

static regex e_timeout("ERROR:  canceling statement due to statement timeout(\n|.)*");
static regex e_syntax("ERROR:  syntax error at or near(\n|.)*");

#define OID long

bool pg_type::consistent(sqltype *rvalue)
{
  pg_type *t = dynamic_cast<pg_type*>(rvalue);

  if (!t) {
    cerr << "unknown type: " << rvalue->name  << endl;
    return false;
  }

  switch(typtype_) {
  case 'b': /* base type */
  case 'c': /* composite type */
  case 'd': /* domain */
  case 'r': /* range */
  case 'm': /* multirange */
  case 'e': /* enum */
    return this == t;
    
  case 'p': /* pseudo type: accept any concrete matching type */
    if (name == "anyarray" || name == "anycompatiblearray") {
      return t->typelem_ != InvalidOid;
    } else if (name == "anynonarray" || name == "anycompatiblenonarray") {
      return t->typelem_ == InvalidOid;
    } else if(name == "anyenum") {
      return t->typtype_ == 'e';
    } else if (name == "\"any\"" || name == "anycompatible") { /* as quoted by quote_ident() */
      return t->typtype_ != 'p'; /* any non-pseudo type */
    } else if (name == "anyelement") {
      return t->typelem_ == InvalidOid;
    } else if (name == "anyrange" || name == "anycompatiblerange") {
      return t->typtype_ == 'r';
    } else if (name == "anymultirange" || name == "anycompatiblemultirange") {
      return t->typtype_ == 'm';
    } else if (name == "record") {
      return t->typtype_ == 'c';
    } else if (name == "cstring") {
      return this == t;
    } else {
      return false;
    }
      
  default:
    throw std::logic_error("unknown typtype");
  }
}

dut_pqxx::dut_pqxx(std::string conninfo)
  : c(conninfo)
{
     c.set_variable("statement_timeout", "'1s'");
     c.set_variable("client_min_messages", "'ERROR'");
     c.set_variable("application_name", "'" PACKAGE "::dut'");
}

void dut_pqxx::test(const std::string &stmt)
{
  try {
#ifndef HAVE_LIBPQXX7
    if(!c.is_open())
       c.activate();
#endif

    pqxx::work w(c);
    w.exec(stmt.c_str());
    w.abort();
  } catch (const pqxx::failure &e) {
    if ((dynamic_cast<const pqxx::broken_connection *>(&e))) {
      /* re-throw to outer loop to recover session. */
      throw dut::broken(e.what());
    }

    if (regex_match(e.what(), e_timeout))
      throw dut::timeout(e.what());
    else if (regex_match(e.what(), e_syntax))
      throw dut::syntax(e.what());
    else
      throw dut::failure(e.what());
  }
}


schema_pqxx::schema_pqxx(std::string &conninfo, bool no_catalog)
{
  // c.set_variable("application_name", "'" PACKAGE "::schema'"); // 这里是在拼接url 感觉没用
  cout << "into create schema" << endl;
  // pqxx::work w(c);
  // pqxx::result r = w.exec("select version()");
  otl_connect* db = init_db();
  db->rlogon("SYSDBA/szoscar55@localhost:2003/DATEBASE1", 1);
  otl_stream i(1, "select version();", *db);
  char version[100];
  i >> version;
  cout << "version: " << version << endl;
  i.close(); //记得关闭流
  // version = r[0][0].as<string>(); // 这里是在获取版本

  // r = w.exec("SHOW server_version_num");
  // version_num = r[0][0].as<int>(); // 这里是在获取版本号

  // address the schema change in postgresql 11 that replaced proisagg and proiswindow with prokind
  // TODO: 需要更改
  string procedure_is_aggregate = version_num < 110000 ? "proisagg" : "prokind = 'a'";
  string procedure_is_window = version_num < 110000 ? "proiswindow" : "prokind = 'w'";

  cerr << "Loading types...";
  // 这里是在导入type
  // r = w.exec("select case typnamespace when 'pg_catalog'::regnamespace then quote_ident(typname) "
	//      "else format('%I.%I', typnamespace::regnamespace, typname) end, "
	//      "oid, typdelim, typrelid, typelem, typarray, typtype "
	//      "from pg_type "); // 我对应获取SYS_TYPE
  
  otl_stream sql_tp(1, "select typname, oid, typdelim, typrelid, typelem, typelem, typtype from sys_type;", *db);

  // 使用 std::vector 存储每列的值
  string string_name;
  char name[0x100];
  OID oid;
  char *typdelim = new char[0x100];
  OID typrelid;
  OID typelem;
  OID typarray;
  char *typtype = new char[0x100];

  while (!sql_tp.eof()) {
    sql_tp >> name >> oid >> typdelim >> typrelid >> typelem >> typarray >> typtype;
    string_name = name; // 将 char* 转换为 std::string
    cout << "name: " << name << " " << "oid: " << oid << " " << "typdelim: " << typdelim << " " << "typrelid: " << typrelid
    << " " << "typelem: " << typelem << " " << "typarray: " << typarray << " " << "typtype: " << typtype << endl;
  
    pg_type *t = new pg_type(string_name, oid, typdelim[0], typrelid, typelem, typarray, typtype[0]);
    oid2type[oid] = t;
    name2type[string_name] = t;
    types.push_back(t);
  }

  sql_tp.close(); //记得关闭流
  // for (auto row = r.begin(); row != r.end(); ++row) {
  //   string name(row[0].as<string>());
  //   OID oid(row[1].as<OID>());
  //   string typdelim(row[2].as<string>());
  //   OID typrelid(row[3].as<OID>());
  //   OID typelem(row[4].as<OID>());
  //   OID typarray(row[5].as<OID>());
  //   string typtype(row[6].as<string>());
  //   //       if (schema == "pg_catalog")
  //   // 	continue;
  //   //       if (schema == "information_schema")
  //   // 	continue;

  //   pg_type *t = new pg_type(name,oid,typdelim[0],typrelid, typelem, typarray, typtype[0]);
  //   oid2type[oid] = t;
  //   name2type[name] = t;
  //   types.push_back(t);
  // }

  booltype = name2type["bool"];
  inttype = name2type["int4"];

  internaltype = name2type["internal"];
  arraytype = name2type["anyarray"];

  cerr << "done." << endl;
  // 导入table
  cerr << "Loading tables...";
  otl_stream sql_tables(1, "select relname, nspname from sys_namespace n join sys_class c on n.oid = c.relnamespace;" , *db);

  char *relname = new char[0x100];
  char *nspname = new char[0x100];

  while (!sql_tables.eof()) {
    sql_tables >> relname >> nspname;
    cout << "relname: " << relname << " " << "nspname: " << nspname << endl;
    string relname_str = relname;
    string nspname_str = nspname;
    table t(
      relname_str,
      nspname_str,
      true,
      false
    );
    tables.push_back(t);
  }

  sql_tables.close(); //记得关闭流
  // r = w.exec("select table_name, "
	// 	    "table_schema, "
	//             "is_insertable_into, "
	//             "table_type "
	//      "from information_schema.tables");
	     
  // for (auto row = r.begin(); row != r.end(); ++row) {
  //   string schema(row[1].as<string>());
  //   string insertable(row[2].as<string>());
  //   string table_type(row[3].as<string>());

	// if (no_catalog && ((schema == "pg_catalog") || (schema == "information_schema")))
	// 	continue;
      
  //   tables.push_back(table(row[0].as<string>(),
	// 		   schema,
	// 		   ((insertable == "YES") ? true : false),
	// 		   ((table_type == "BASE TABLE") ? true : false)));
  // }

  cerr << "done." << endl;
  // 导入
  cerr << "Loading columns and constraints..." << endl;

  for (auto t = tables.begin(); t != tables.end(); ++t) {
    // cout << "now doing table: " << t->name << endl;

// 假设 t->name.data() 和 t->schema.data() 返回的是 const char*
    std::string query = std::string("select attname, atttypid from sys_attribute ")
                        + "join sys_class c on (c.oid = attrelid) "
                        + "join sys_namespace n on n.oid = relnamespace "
                        + "where not attisdropped and attname not in ('xmin', 'xmax', 'ctid', 'cmin', 'cmax', 'tableoid', 'oid') ";
                        + "and relname = '" + t->name + "' "
                        + "and nspname = '" + t->schema + "';";
    // std::string query = "select attname, atttypid from sys_attribute";

    // cout << "query_string is: " << query << endl;
    otl_stream sql_columns(1, query.c_str(), *db);

    // cout << "finish query sql" << endl;
    char *attname = new char[0x100];
    OID atttypid;
    while (!sql_columns.eof()) {
      // cout << "------------------------------------start parse------------------------------------" << endl;
      sql_columns >> attname >> atttypid;
      // cout << "parse finish" << endl;
      // cout << "attname: " << attname << " " << "atttypid: " << atttypid << endl;
      string attname_str = attname;
      column c(attname_str, oid2type[atttypid]);
      t->columns().push_back(c);
    }
    sql_columns.close(); //记得关闭流

    std::string query1 = "select conname from sys_class t "
                        "join sys_constraint c on (t.oid = c.conrelid) "
                        "where contype in ('f', 'u', 'p') "
                        "and relnamespace = (select oid from sys_namespace where nspname = '"
                        + t->schema + "') "
                        "and relname = '"
                        + t->name + "';";


    otl_stream sql_constraint(1, query1.c_str(), *db);

    char *conname = new char[0x100];
    while (!sql_constraint.eof()) {
      sql_constraint >> conname;
      cout << "conname: " << conname << endl;
      string conname_str = conname;
      t->constraints.push_back(conname_str);
    }

    sql_constraint.close(); //记得关闭流

  }


  // for (auto t = tables.begin(); t != tables.end(); ++t) { // sys_attribute sys_namespace
  //   string q("select attname, "
	//      "atttypid "
	//      "from pg_attribute join pg_class c on( c.oid = attrelid ) "
	//      "join pg_namespace n on n.oid = relnamespace "
	//      "where not attisdropped "
	//      "and attname not in "
	//      "('xmin', 'xmax', 'ctid', 'cmin', 'cmax', 'tableoid', 'oid') ");
  //   q += " and relname = " + w.quote(t->name);
  //   q += " and nspname = " + w.quote(t->schema);

  //   r = w.exec(q);
  //   for (auto row : r) { // 获取字段数据类型 按顺序
  //     column c(row[0].as<string>(), oid2type[row[1].as<OID>()]);
  //     t->columns().push_back(c);
  //   }
  //   // sys_constraint sys_namespace
  //   q = "select conname from pg_class t "
  //     "join pg_constraint c on (t.oid = c.conrelid) "
  //     "where contype in ('f', 'u', 'p') ";
  //   q += " and relnamespace = " " (select oid from pg_namespace where nspname = " + w.quote(t->schema) + ")";
  //   q += " and relname = " + w.quote(t->name);

  //   for (auto row : w.exec(q)) { // 获取约束名字(不一定是唯一的) 按顺序
  //     t->constraints.push_back(row[0].as<string>());
  //   }
    
  // }
  cerr << "done." << endl;

  cerr << "Loading operators...";
  // sys_operator
  // r = w.exec("select oprname, oprleft,"
	// 	    "oprright, oprresult "
	// 	    "from pg_catalog.pg_operator "
  //                   "where 0 not in (oprresult, oprright, oprleft) ");
  // for (auto row : r) {
  //   op o(row[0].as<string>(), // 操作符名字 操作符左右以及结果的类型
	//  oid2type[row[1].as<OID>()],
	//  oid2type[row[2].as<OID>()],
	//  oid2type[row[3].as<OID>()]);
  //   register_operator(o);
  // }
  otl_stream sql_operator(1, "select oprname, oprleft, oprright, oprresult from sys_operator where 0 not in (oprresult, oprright, oprleft);", *db);
  char *oprname = new char[0x100];
  OID oprleft;
  OID oprright;
  OID oprresult;
  while (!sql_operator.eof()) {
    sql_operator >> oprname >> oprleft >> oprright >> oprresult;
    cout << "oprname: " << oprname << " oprleft: " << oprleft << " oprright: " << oprright << " oprresult: " << oprresult << endl;
    string oprname_str = oprname;
    op o(
      oprname_str,
      oid2type[oprleft],
      oid2type[oprright],
      oid2type[oprresult]
    );
    register_operator(o);
  }
  sql_operator.close(); //记得关闭流


  cerr << "done." << endl;
  //TODO: 可能还需要改一下

  // sys_proc
  cerr << "Loading routines..."; // npsname oid 返回值类型 函数名字 
  // r = w.exec("select (select nspname from pg_namespace where oid = pronamespace), oid, prorettype, proname "
	//      "from pg_proc "
	//      "where prorettype::regtype::text not in ('event_trigger', 'trigger', 'opaque', 'internal') "
	//      "and proname <> 'pg_event_trigger_table_rewrite_reason' "
	//      "and proname <> 'pg_event_trigger_table_rewrite_oid' "
	//      "and proname !~ '^ri_fkey_' "
	//      "and not (proretset or " + procedure_is_aggregate + " or " + procedure_is_window + ") ");

  // for (auto row : r) {
  //   routine proc(row[0].as<string>(),
	// 	 row[1].as<string>(),
	// 	 oid2type[row[2].as<long>()],
	// 	 row[3].as<string>());
  //   register_routine(proc);
  // }
  otl_stream sql_routine(1, "select (select nspname from sys_namespace where oid = pronamespace), oid, prorettype, proname from sys_proc where PROISFUNC != 2 and PRORETSET = False and PROISAGG = False;", *db);
  char* nspname_ = new char[0x100];
  OID oid_;
  OID prorettype_;
  char* proname_ = new char[0x100];
  cout << "routine sql finish " << endl;
  while (!sql_routine.eof()) {
    sql_routine >> nspname_ >> oid_ >> prorettype_ >> proname_;
    cout << "nspname: " << nspname_ << " oid: " << oid_ << " prorettype: " << prorettype_ << " proname: " << proname_ << endl;
    string nspname_str = nspname_;
    string proname_str = proname_;
    string oid_str = std::to_string(oid_);
    routine proc(
      nspname_str,
      oid_str, // 这里有点神秘
      oid2type[prorettype_],
      proname_str
    );
    register_routine(proc);
  }

  sql_routine.close();
  cerr << "done." << endl;

  cerr << "Loading routine parameters...";

  for (auto &proc : routines){

    // get counts first
    std::string query2_1 = "select PRONARGS from sys_proc where oid = '"
                        + proc.specific_name + "';";

    otl_stream routine_args_count(1, query2_1.c_str(), *db);
    int count;
    routine_args_count >> count;
    for(int i = 0; i < count; i++){
      // get types
      std::string query2_2 = "select proargtypes[" + std::to_string(i) + "] "
                          + "from sys_proc where oid = '"
                          + proc.specific_name + "';";
      otl_stream routine_args(1, query2_2.c_str(), *db);
      OID proargtypes_;
      routine_args >> proargtypes_;
      proc.argtypes.push_back(oid2type[proargtypes_]);
      cout << "argtype: " << proargtypes_ << endl;
    }
  }
  // for (auto &proc : routines) { // 	函数参数的数据类型的向量
  //   string q("select unnest(proargtypes) "
	//      "from pg_proc ");
  //   q += " where oid = " + w.quote(proc.specific_name);
      
  //   r = w.exec(q);
  //   for (auto row : r) {
  //     sqltype *t = oid2type[row[0].as<OID>()];
  //     assert(t);
  //     proc.argtypes.push_back(t);
  //   }
  // }
  cerr << "done." << endl;
  // sys_proc sys_namespace
  cerr << "Loading aggregates..."; // 获取聚合函数
  otl_stream aggregates(1, "select (select nspname from sys_namespace where oid = pronamespace), oid, prorettype, proname from sys_proc where PROISFUNC != 2 and PRORETSET = False and PROISAGG = True;", *db);

  char* nspname_agg = new char[0x100];
  OID oid_agg;
  OID prorettype_agg;
  char* proname_agg = new char[0x100];
  
  while (!aggregates.eof()) {
    aggregates >> nspname_agg >> oid_agg >> prorettype_agg >> proname_agg;
    cout << "nspname: " << nspname_agg << " oid: " << oid_agg << " prorettype: " << prorettype_agg << " proname: " << proname_agg << endl;
    string nspname_agg_str = nspname_agg;
    string proname_agg_str = proname_agg;
    string oid_agg_str = std::to_string(oid_agg);
    routine proc(
      nspname_agg_str,
      oid_agg_str, // 这里有点神秘
      oid2type[prorettype_],
      proname_agg_str
    );
    register_aggregate(proc);
  }

  // r = w.exec("select (select nspname from pg_namespace where oid = pronamespace), oid, prorettype, proname "
	//      "from pg_proc "
	//      "where prorettype::regtype::text not in ('event_trigger', 'trigger', 'opaque', 'internal') "
	//      "and proname not in ('pg_event_trigger_table_rewrite_reason') "
	//      "and proname not in ('percentile_cont', 'dense_rank', 'cume_dist', "
	//      "'rank', 'test_rank', 'percent_rank', 'percentile_disc', 'mode', 'test_percentile_disc') "
	//      "and proname !~ '^ri_fkey_' "
	//      "and not (proretset or " + procedure_is_window + ") "
	//      "and " + procedure_is_aggregate);

  // for (auto row : r) {
  //   routine proc(row[0].as<string>(),
	// 	 row[1].as<string>(),
	// 	 oid2type[row[2].as<OID>()],
	// 	 row[3].as<string>());
  //   register_aggregate(proc);
  // }

  cerr << "done." << endl;

  cerr << "Loading aggregate parameters..."; // 聚合函数的参数

  for (auto &proc : routines){

    // get counts first
    std::string query3_1 = "select PRONARGS from sys_proc where oid = '"
                        + proc.specific_name + "';";

    otl_stream routine_args_count(1, query3_1.c_str(), *db);
    int count;
    routine_args_count >> count;
    for(int i = 0; i < count; i++){
      // get types
      std::string query3_2 = "select proargtypes[" + std::to_string(i) + "] "
                          + "from sys_proc where oid = '"
                          + proc.specific_name + "';";
      otl_stream routine_args(1, query3_2.c_str(), *db);
      OID proargtypes_;
      routine_args >> proargtypes_;
      proc.argtypes.push_back(oid2type[proargtypes_]);
      cout << "argtype: " << proargtypes_ << endl;
    }
  }

  // sys_proc
  // for (auto &proc : aggregates) {
  //   string q("select unnest(proargtypes) "
	//      "from pg_proc ");
  //   q += " where oid = " + w.quote(proc.specific_name);
      
  //   r = w.exec(q);
  //   for (auto row : r) {
  //     sqltype *t = oid2type[row[0].as<OID>()];
  //     assert(t);
  //     proc.argtypes.push_back(t);
  //   }
  // }
  cerr << "done." << endl;
// #ifdef HAVE_LIBPQXX7
//   c.close();
// #else
//   c.disconnect();
// #endif
  generate_indexes();
}

extern "C" {
    void dut_libpq_notice_rx(void *arg, const PGresult *res);
}

void dut_libpq_notice_rx(void *arg, const PGresult *res)
{
    (void) arg;
    (void) res;
}

void dut_libpq::connect(std::string &conninfo)
{
    if (conn) {
	PQfinish(conn);
    }
    conn = PQconnectdb(conninfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK)
    {
	char *errmsg = PQerrorMessage(conn);
	if (strlen(errmsg))
	    throw dut::broken(errmsg, "08001");
    }

    command("set statement_timeout to '1s'");
    command("set client_min_messages to 'ERROR';");
    command("set application_name to '" PACKAGE "::dut';");

    PQsetNoticeReceiver(conn, dut_libpq_notice_rx, (void *) 0);
}

dut_libpq::dut_libpq(std::string conninfo)
    : conninfo_(conninfo)
{
    connect(conninfo);
}

void dut_libpq::command(const std::string &stmt)
{
    if (!conn)
	connect(conninfo_);
    PGresult *res = PQexec(conn, stmt.c_str());

    switch (PQresultStatus(res)) {

    case PGRES_FATAL_ERROR:
    default:
    {
	const char *errmsg = PQresultErrorMessage(res);
	if (!errmsg || !strlen(errmsg))
	     errmsg = PQerrorMessage(conn);

	const char *sqlstate = PQresultErrorField(res, PG_DIAG_SQLSTATE);
	if (!sqlstate || !strlen(sqlstate))
	     sqlstate =  (CONNECTION_OK != PQstatus(conn)) ? "08000" : "?????";
	
	std::string error_string(errmsg);
	std::string sqlstate_string(sqlstate);
	PQclear(res);

	if (CONNECTION_OK != PQstatus(conn)) {
            PQfinish(conn);
	    conn = 0;
	    throw dut::broken(error_string.c_str(), sqlstate_string.c_str());
	}
	if (sqlstate_string == "42601")
	     throw dut::syntax(error_string.c_str(), sqlstate_string.c_str());
	else
	     throw dut::failure(error_string.c_str(), sqlstate_string.c_str());
    }

    case PGRES_NONFATAL_ERROR:
    case PGRES_TUPLES_OK:
    case PGRES_SINGLE_TUPLE:
    case PGRES_COMMAND_OK:
	PQclear(res);
	return;
    }
}

void dut_libpq::test(const std::string &stmt)
{
    command("ROLLBACK;");
    command("BEGIN;");
    command(stmt.c_str());
    command("ROLLBACK;");
}
