#include "config.h"

#include <iostream>
#include <chrono>
// my headers injection ##
// #include <unistd.h>
// #include <sys/wait.h>
// #include <pthread.h>
// #include <iostream>
// #include <string>
// #include <atomic>
// #include <cstdio>
// #include <cstring>
// my headers injection end ##

#ifndef HAVE_BOOST_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
using boost::regex;
using boost::smatch;
using boost::regex_match;
#endif

#include <thread>
#include <typeinfo>

#include "random.hh"
#include "grammar.hh"
#include "relmodel.hh"
#include "schema.hh"
#include "gitrev.h"

#include "log.hh"
#include "dump.hh"
#include "impedance.hh"
#include "dut.hh"

#ifdef HAVE_LIBSQLITE3
#include "sqlite.hh"
#endif

#ifdef HAVE_MONETDB
#include "monetdb.hh"
#endif

#include "postgres.hh"
#include "otl.hh"

using namespace std;

using namespace std::chrono;

extern "C" {
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
}

/* make the cerr logger globally accessible so we can emit one last
	 report on SIGINT */
cerr_logger *global_cerr_logger;

extern "C" void cerr_log_handler(int)
{
	if (global_cerr_logger)
		global_cerr_logger->report();
	exit(1);
}


// my function && global variants injection ##
// 全局变量用于控制循环的运行状态
// std::atomic<bool> running(true);

// char buffer1[0x1000]; // for stdout
// char buffer2[0x1000]; // for stderr
// int outPipe[2]; // 标准输出管道
// int errPipe[2]; // 标准错误管道
// pthread_t outputThread, errorThread;
// // 线程函数：读取子进程的标准输出
// void* readOutputStream(void* arg) {
//     int fd = *static_cast<int*>(arg);
//     FILE* fp = fdopen(fd, "r");
//     if (!fp) {
//         perror("fdopen");
//         return nullptr;
//     }

//     while (running && std::fgets(buffer1, sizeof(buffer1), fp)) {
//         std::cout << buffer1;
//         std::memset(buffer1, 0, sizeof(buffer1)); // 清空缓冲区
//     }

//     fclose(fp);
//     return nullptr;
// }

// // 线程函数：读取子进程的标准错误
// void* readErrorStream(void* arg) {
//     int fd = *static_cast<int*>(arg);
//     FILE* fp = fdopen(fd, "r");
//     if (!fp) {
//         perror("fdopen");
//         return nullptr;
//     }

//     while (running && std::fgets(buffer2, sizeof(buffer2), fp)) {
//         std::cerr << buffer2;
//         std::memset(buffer2, 0, sizeof(buffer2)); // 清空缓冲区
//     }

//     fclose(fp);
//     return nullptr;
// }

// // 执行一个命令并获取其输出
// void executeCommand(const char* cmd, int outPipe[2], int errPipe[2]) {
//     pid_t pid = fork();
//     if (pid == -1) {
//         perror("fork");
//         return;
//     }

//     if (pid == 0) { // 子进程
//         // 关闭父进程中的管道读端
//         close(outPipe[0]);
//         close(errPipe[0]);

//         // 重定向子进程的标准输出和标准错误
//         dup2(outPipe[1], STDOUT_FILENO);
//         dup2(errPipe[1], STDERR_FILENO);

//         // 关闭不再使用的管道写端
//         close(outPipe[1]);
//         close(errPipe[1]);

//         // 执行命令
//         execl("/home/hkbin/Workspace/chaitin_workspace/database_fuzz/QqlFuzz/tool/isql", "isql", "-U", "SYSAUDIT/szoscar55", "-d", "DATEBASE1", "-c", cmd, (char*)nullptr);
//         perror("execl");
//         _exit(1);
//     } else { // 父进程
//         // 关闭子进程中的管道写端
//         close(outPipe[1]);
//         close(errPipe[1]);

//         // 等待子进程结束
//         waitpid(pid, nullptr, 0);
//     }
// }

// my function && global variants injection end ##


int main(int argc, char *argv[])
{


    char* buffer = new char[0x1000];

    otl_connect* db = init_db();

    rlogin("SYSAUDIT/szoscar55@localhost:2003/DATEBASE1", db);

    char strSql[] = "select version();";

    otl_stream otlCur(1, strSql, *db);
    // otl_stream ret;
    // db_exec(db, strSql, &ret);

    otlCur >> buffer;

    cout << buffer << endl;

    cout << "exit" << endl;


	cerr << PACKAGE_NAME " " GITREV << endl;

	map<string,string> options;
	regex optregex("--(help|log-to|verbose|target|sqlite|monetdb|version|dump-all-graphs|dump-all-queries|seed|dry-run|max-queries|rng-state|exclude-catalog)(?:=((?:.|\n)*))?");
	
	for(char **opt = argv+1 ;opt < argv+argc; opt++) {
		smatch match;
		string s(*opt);
		if (regex_match(s, match, optregex)) {
			options[string(match[1])] = match[2];
		} else {
			cerr << "Cannot parse option: " << *opt << endl;
			options["help"] = "";
		}
	}

	if (options.count("help")) {
		cerr <<
			"    --target=connstr     postgres database to send queries to" << endl <<
#ifdef HAVE_LIBSQLITE3
			"    --sqlite=URI         SQLite database to send queries to" << endl <<
#endif
#ifdef HAVE_MONETDB
			"    --monetdb=connstr    MonetDB database to send queries to" <<endl <<
#endif
			"    --log-to=connstr     log errors to postgres database" << endl <<
			"    --seed=int           seed RNG with specified int instead of PID" << endl <<
			"    --dump-all-queries   print queries as they are generated" << endl <<
			"    --dump-all-graphs    dump generated ASTs" << endl <<
			"    --dry-run            print queries instead of executing them" << endl <<
			"    --exclude-catalog    don't generate queries using catalog relations" << endl <<
			"    --max-queries=long   terminate after generating this many queries" << endl <<
			"    --rng-state=string    deserialize dumped rng state" << endl <<
			"    --verbose            emit progress output" << endl <<
			"    --version            print version information and exit" << endl <<
			"    --help               print available command line options and exit" << endl;
		return 0;
	} else if (options.count("version")) {
		return 0;
	}

	

	try
		{
			// find attach DBMS ### need edit to adjust my DBMS

			// 这里是在测试数据库可连接性

			shared_ptr<schema> schema;
			if (options.count("sqlite")) {
			#ifdef HAVE_LIBSQLITE3
				schema = make_shared<schema_sqlite>(options["sqlite"], options.count("exclude-catalog"));
			#else
				cerr << "Sorry, " PACKAGE_NAME " was compiled without SQLite support." << endl;
				return 1;
			#endif
			}
			else if(options.count("monetdb")) {
			#ifdef HAVE_MONETDB
				schema = make_shared<schema_monetdb>(options["monetdb"]);
			#else
				cerr << "Sorry, " PACKAGE_NAME " was compiled without MonetDB support." << endl;
				return 1;
			#endif
			}
			else{
				cout << "start to create pqxx_schema" << endl;
				schema = make_shared<schema_pqxx>(options["target"], options.count("exclude-catalog")); // 这里我填url就行了
				cout << "finish create pqxx_schema" << endl;
			}
			// find attach DBMS end ###
			scope scope;
			long queries_generated = 0;
			schema->fill_scope(scope); // 填充生成词

			if (options.count("rng-state")) {
				istringstream(options["rng-state"]) >> smith::rng;
			} else {
				smith::rng.seed(options.count("seed") ? stoi(options["seed"]) : getpid());
			}

			vector<shared_ptr<logger> > loggers;

			loggers.push_back(make_shared<impedance_feedback>());

			if (options.count("log-to"))
				loggers.push_back(make_shared<pqxx_logger>(
					options.count("sqlite") ? options["sqlite"] : options["target"],
				options["log-to"], *schema));

			if (options.count("verbose")) {
				auto l = make_shared<cerr_logger>();
				global_cerr_logger = &*l;
				loggers.push_back(l);
				signal(SIGINT, cerr_log_handler);
			}
			
			if (options.count("dump-all-graphs"))
				loggers.push_back(make_shared<ast_logger>());

			if (options.count("dump-all-queries"))
				loggers.push_back(make_shared<query_dumper>());

			if (options.count("dry-run")) {
				while (1) {
					shared_ptr<prod> gen = statement_factory(&scope);
					gen->out(cout);
					for (auto l : loggers)
						l->generated(*gen);
					cout << ";" << endl;
					queries_generated++;

					if (options.count("max-queries")
							&& (queries_generated >= stol(options["max-queries"])))
							return 0;
				}
			}

			// dut is important 需要研究一下dut是什么
			shared_ptr<dut_base> dut;
			
			if (options.count("sqlite")) {
			#ifdef HAVE_LIBSQLITE3
				dut = make_shared<dut_sqlite>(options["sqlite"]);
			#else
				cerr << "Sorry, " PACKAGE_NAME " was compiled without SQLite support." << endl;
				return 1;
			#endif
			}
			else if(options.count("monetdb")) {
			#ifdef HAVE_MONETDB	   
				dut = make_shared<dut_monetdb>(options["monetdb"]);
			#else
				cerr << "Sorry, " PACKAGE_NAME " was compiled without MonetDB support." << endl;
				return 1;
			#endif
			}
			else
				dut = make_shared<dut_libpq>(options["target"]);

			
			// my code1 injection ##

			// if (pipe(outPipe) == -1 || pipe(errPipe) == -1) {
			// 	perror("pipe");
			// 	return 1;
			// }
			// pthread_create(&outputThread, nullptr, readOutputStream, &outPipe[0]);
			// pthread_create(&errorThread, nullptr, readErrorStream, &errPipe[0]);

			// 向子进程写入输入并处理交互
			// chdir("/home/hkbin/Workspace/chaitin_workspace/database_fuzz/QqlFuzz/tool");
			// std::string input_line;

			// my code1 injection end ##

			while (1) /* Loop to recover connection loss */
			{
				try {
					while (1) { /* Main loop */
						if (options.count("max-queries")
						&& (++queries_generated > stol(options["max-queries"]))) {
							if (global_cerr_logger)
								global_cerr_logger->report();
							return 0;
						}
						
						/* Invoke top-level production to generate AST */
						shared_ptr<prod> gen = statement_factory(&scope);

						for (auto l : loggers)
							l->generated(*gen);
					
						/* Generate SQL from AST */
						ostringstream s;
						gen->out(s);

						/* Try to execute it */
						try {
							dut->test(s.str());
							for (auto l : loggers)
								l->executed(*gen); // next injection executeCommand(input_line.c_str(), outPipe, errPipe);
						} 
						catch (const dut::failure &e) {
							for (auto l : loggers)
							try {
								l->error(*gen, e);
							} catch (runtime_error &e) {
								cerr << endl << "log failed: " << typeid(*l).name() << ": "
										<< e.what() << endl;
							}
							if ((dynamic_cast<const dut::broken *>(&e))) {
							/* re-throw to outer loop to recover session. */
							throw;
							}
						}
					}
				}
				catch (const dut::broken &e) {
					/* Give server some time to recover. */
					this_thread::sleep_for(milliseconds(1000));
				}
				}
			}
	catch (const exception &e) {
		cerr << e.what() << endl;
		cout << "error found!" << endl;
		// my code2 injection ##
	    // 关闭管道
		// close(outPipe[0]);
		// close(errPipe[0])
		// 等待线程完成
		// pthread_join(outputThread, nullptr);
		// pthread_join(errorThread, nullptr);
		// my code2 injection end ##
		return 1;
	}
	cout << "all finished " << endl;
}
