/// @file
/// @brief supporting classes for the grammar

#ifndef RELMODEL_HH
#define RELMODEL_HH
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <memory>
#include <cassert>

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::make_pair;
using std::shared_ptr;

struct sqltype {
  string name;
  static map<string, struct sqltype*> typemap;
  static struct sqltype *get(string s);
  sqltype(string n) : name(n) { }

  /** This function is used to model postgres-style pseudotypes.
      A generic type is consistent with a more concrete type.
      E.G., anyarray->consistent(intarray) is true
            while int4array->consistent(anyarray) is false

      There must not be cycles in the consistency graph, since the
      grammar will use fixpoint iteration to resolve type conformance
      situations in the direction of more concrete types  */
  virtual bool consistent(struct sqltype *rvalue);
};

struct column { // 一个column由name和type构成
  string name;
  sqltype *type;
  column(string name) : name(name) { } // 构造函数 方便初始化
  column(string name, sqltype *t) : name(name), type(t) {
    assert(t);
  }
};

struct relation {
  vector<column> cols; // relation
  virtual vector<column> &columns() { return cols; }
};

struct named_relation : relation { // 继承自relation
  string name;
  virtual string ident() { return name; }
  virtual ~named_relation() { }
  named_relation(string n) : name(n) { }
};

struct aliased_relation : named_relation {
  relation *rel;
  virtual ~aliased_relation() { }
  aliased_relation(string n, relation* r) : named_relation(n), rel(r) { } // 构造函数
  virtual vector<column>& columns() { return rel->columns(); }
};

struct table : named_relation {
  string schema; // 模式
  bool is_insertable;
  bool is_base_table;
  vector<string> constraints; // 约束条件
  table(string name, string schema, bool insertable, bool base_table)
    : named_relation(name),
      schema(schema),
      is_insertable(insertable),
      is_base_table(base_table) { }
  virtual string ident() { return schema + "." + name; }
  virtual ~table() { };
};

struct scope { // 支持嵌套作用域 比如嵌套查询 视图
  struct scope *parent;
  /// available to table_ref productions
  vector<named_relation*> tables;
 /// available to column_ref productions
  vector<named_relation*> refs;
  struct schema *schema;
  /// Counters for prefixed stmt-unique identifiers
  shared_ptr<map<string,unsigned int> > stmt_seq;
  scope(struct scope *parent = 0) : parent(parent) {
    if (parent) {
      schema = parent->schema;
      tables = parent->tables;
      refs = parent->refs;
      stmt_seq = parent->stmt_seq;
    }
  }
  vector<pair<named_relation*, column> > refs_of_type(sqltype *t) {
    vector<pair<named_relation*, column> > result;
    for (auto r : refs)
      for (auto c : r->columns())
	if (t->consistent(c.type))
	  result.push_back(make_pair(r,c));
    return result;
  }
  /** Generate unique identifier with prefix. */
  string stmt_uid(const char* prefix) {
    string result(prefix);
    result += "_";
    result += std::to_string((*stmt_seq)[result]++);
    return result;
  }
  /** Reset unique identifier counters. */
  void new_stmt() {
    stmt_seq = std::make_shared<map<string,unsigned int> >();
  }
};

struct op { // op
  string name;
  sqltype *left;
  sqltype *right;
  sqltype *result;
  op(string n,sqltype *l,sqltype *r, sqltype *res)
    : name(n), left(l), right(r), result(res) { }
  op() { }
};

struct routine { // 表示数据库中的存储过程或函数
  string specific_name;
  string schema; // 递归嵌套的语句
  vector<sqltype *> argtypes; // 参数类型
  sqltype *restype; // 返回值类型
  string name; // 记录了函数名
  routine(string schema, string specific_name, sqltype* data_type, string name)
    : specific_name(specific_name), schema(schema), restype(data_type), name(name) {
    assert(data_type);
  }
  virtual string ident() {
    if (schema.size())
      return schema + "." + name;
    else
      return name;
  }
};

#endif
