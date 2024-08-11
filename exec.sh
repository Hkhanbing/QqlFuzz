#!/bin/sh

clang++ -o otltest -I/opt/ShenTong/drivers/aci/include/ -I. -I./db2-include/inlcude/ -L/opt/ShenTong/drivers/aci/linux64 -L/opt/ShenTong/drivers/odbc/lib -Wl,-rpath-link,/opt/ShenTong/drivers/odbc/lib -lodbc -lpthread -laci otl.cpp otl.cc -DOTL_ORA11G

patchelf --replace-needed libaci.so /opt/ShenTong/drivers/aci/linux64/libaci.so ./otltest

./otltest
