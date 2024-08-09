#include <typeinfo>
#include "config.h"
#include "schema.hh"
#include "relmodel.hh"
#include <pqxx/pqxx>
#include "gitrev.h"

using namespace std;
using namespace pqxx;

void schema::generate_indexes() {

  cerr << "Generating indexes...";

  for (auto &type: types) { // 疑似pg特有types
    assert(type);
    for(auto &r: aggregates) { // 纯聚合函数
      if (type->consistent(r.restype))
	      aggregates_returning_type[type].push_back(&r);
    }

    for(auto &r: routines) { // 获取函数/存储过程的返回值
      if (!type->consistent(r.restype))
        continue;
      routines_returning_type[type].push_back(&r);
      if(!r.argtypes.size())
	      parameterless_routines_returning_type[type].push_back(&r); // 获得无参数的函数返回值
    }
    
    for (auto &t: tables) {
      for (auto &c: t.columns()) {
        if (type->consistent(c.type)) {
          tables_with_columns_of_type[type].push_back(&t); // 获得table的类型
          break;
        }
      }
    }

    for (auto &concrete: types) {
      if (type->consistent(concrete))
	      concrete_type[type].push_back(concrete);
    }

    for (auto &o: operators) {
      if (type->consistent(o.result))
	      operators_returning_type[type].push_back(&o); // 记录操作符的返回值
    }
  }

  for (auto &t: tables) {
    if (t.is_base_table)
      base_tables.push_back(&t); // 获取base table
  }
  
  cerr << "done." << endl;

  assert(booltype);
  assert(inttype);
  assert(internaltype);
  assert(arraytype);

}
