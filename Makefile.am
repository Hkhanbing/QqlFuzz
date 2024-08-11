# 包含本地设置文件，如果存在
-include Makefile.local

# 定义生成的源文件
BUILT_SOURCES = gitrev.h

# 定义程序目标
bin_PROGRAMS = sqlsmith

# 条件编译的源文件
DUT = postgres.cc

if DUT_MONETDB
DUT += monetdb.cc
endif

if DUT_SQLITE
DUT += sqlite.cc
endif


sqlsmith_SOURCES = relmodel.cc schema.cc $(DUT) random.cc prod.cc expr.cc grammar.cc log.cc dump.cc impedance.cc sqlsmith.cc otl.cc

# 添加编译和链接标志
sqlsmith_CPPFLAGS = $(CPPFLAGS) -I/opt/ShenTong/drivers/aci/include/ -I. -I./db2-include/inlcude/ -I/usr/include/postgresql -DOTL_ORA11G -w
sqlsmith_LDFLAGS = $(LDFLAGS) -Wl,-rpath-link,/opt/ShenTong/drivers/odbc/lib

# 链接库
sqlsmith_LDADD = $(LIBPQXX_LIBS) $(MONETDB_MAPI_LIBS) $(BOOST_REGEX_LIB) $(POSTGRESQL_LIBS) \
                 /opt/ShenTong/drivers/aci/linux64/libaci.so \
                 /opt/ShenTong/drivers/odbc/lib/libodbc.so.2 -L/opt/ShenTong/drivers/aci/linux64/ -L/opt/ShenTong/drivers/odbc/lib/  -lpthread -laci

# 全局编译器和链接器标志
AM_CPPFLAGS = $(BOOST_CPPFLAGS) $(LIBPQXX_CFLAGS) $(POSTGRESQL_CPPFLAGS) $(MONETDB_MAPI_CFLAGS) -Wall -Wextra
AM_LDFLAGS = $(BOOST_LDFLAGS) $(POSTGRESQL_LDFLAGS)

# 包含在分发包中的额外文件
EXTRA_DIST = gitrev.h dump.hh expr.hh grammar.hh log.hh prod.hh		\
    random.hh relmodel.hh schema.hh impedance.hh known.txt known_re.txt log.sql	\
    README.org TODO.org ast.png logo.png dump.xsl util.hh sqlite.hh	\
    dut.hh postgres.hh monetdb.hh log-v1.0-to-v1.2.sql boring_sqlstates.txt

# 生成 gitrev.h 文件的规则
gitrev.h: $(HEADERS) $(SOURCES)
	-if git describe --exclude='debian*' --dirty --tags --always > /dev/null ; then \
	echo "#define GITREV \"$$(git describe --exclude='debian*' --dirty --tags --always)\"" > $@ ;\
	else \
	echo "#define GITREV \"unreleased\"" > $@ ;\
	fi

# 定义伪目标 filterdump
filterdump:
	psql -Xc 'copy (select error from known) to stdout' |sort -u > known.txt
	psql -Xc 'copy (select re from known_re) to stdout' |sort -u > known_re.txt
	psql -Xc 'copy (select sqlstate from boring_sqlstates) to stdout' |sort -u > boring_sqlstates.txt

.PHONY: filterdump
