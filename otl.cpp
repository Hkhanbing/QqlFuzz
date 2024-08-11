#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "otlv4.h"
#include "otl.hh"

using namespace std;

#define OID long

// 初始化数据库连接
otl_connect* init() {
    otl_connect* db = new otl_connect;
    db->rlogon("SYSAUDIT/szoscar55@localhost:2003/DATEBASE1");
    return db;
}

// 读取查询结果并将所有列处理为字符串
void select_and_classify() {
    otl_connect* db = init();
    otl_stream i(1, "select typname, oid, typdelim, typrelid, typelem, typelem, typtype from sys_type;", *db);
    // 尝试描述查询中的列
    int desc_len;
    otl_column_desc* desc = i.describe_select(desc_len);
    // 使用 std::vector 存储每列的值
    char name[0x100];
    OID oid;
    char typdelim[0x100];
    OID typrelid;
    OID typelem;
    OID typarray;
    char typtype[0x100];

    while (!i.eof()) {
        i >> name >> oid >> typdelim >> typrelid >> typelem >> typarray >> typtype;
        cout << "name: " << name << " " << "oid: " << oid << " " << "typdelim: " << typdelim << " " << "typrelid: " << typrelid
        << " " << "typelem: " << typelem << " " << "typarray: " << typarray << " " << "typtype: " << typtype << endl;
    }
}

int main() {
    select_and_classify();
    return 0;
}
