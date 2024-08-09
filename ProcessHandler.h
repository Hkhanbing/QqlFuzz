#ifndef PROCESS_HANDLER_H
#define PROCESS_HANDLER_H

#include <string>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <cstdio>
#include <cstring>

class ProcessHandler {
public:
    ProcessHandler();
    ~ProcessHandler();

    // 初始化并启动线程
    bool start(const std::string& workingDir);
    
    // 执行命令
    bool executeCommand(const std::string& cmd);

    // 停止进程
    void stop();

private:
    // 线程函数：读取子进程的标准输出
    void readOutputStream();
    
    // 线程函数：读取子进程的标准错误
    void readErrorStream();

    // 执行命令的实际实现
    bool execute(const std::string& cmd);

    std::atomic<bool> running;
    int outPipe[2];
    int errPipe[2];
    std::thread outputThread;
    std::thread errorThread;
    std::string workingDir;
};

#endif // PROCESS_HANDLER_H
