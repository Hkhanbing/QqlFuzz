#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <atomic>
#include <cstdio>
#include <cstring>

// 全局变量用于控制循环的运行状态
std::atomic<bool> running(true);

char buffer1[0x1000]; // for stdout
char buffer2[0x1000]; // for stderr

// 线程函数：读取子进程的标准输出
void* readOutputStream(void* arg) {
    int fd = *static_cast<int*>(arg);
    FILE* fp = fdopen(fd, "r");
    if (!fp) {
        perror("fdopen");
        return nullptr;
    }

    while (running && std::fgets(buffer1, sizeof(buffer1), fp)) {
        std::cout << buffer1;
        std::memset(buffer1, 0, sizeof(buffer1)); // 清空缓冲区
    }

    fclose(fp);
    return nullptr;
}

// 线程函数：读取子进程的标准错误
void* readErrorStream(void* arg) {
    int fd = *static_cast<int*>(arg);
    FILE* fp = fdopen(fd, "r");
    if (!fp) {
        perror("fdopen");
        return nullptr;
    }

    while (running && std::fgets(buffer2, sizeof(buffer2), fp)) {
        std::cerr << buffer2;
        std::memset(buffer2, 0, sizeof(buffer2)); // 清空缓冲区
    }

    fclose(fp);
    return nullptr;
}

// 执行一个命令并获取其输出
void executeCommand(const char* cmd, int outPipe[2], int errPipe[2]) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) { // 子进程
        // 关闭父进程中的管道读端
        close(outPipe[0]);
        close(errPipe[0]);

        // 重定向子进程的标准输出和标准错误
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(errPipe[1], STDERR_FILENO);

        // 关闭不再使用的管道写端
        close(outPipe[1]);
        close(errPipe[1]);

        // 执行命令
        execl("/home/hkbin/Workspace/chaitin_workspace/database_fuzz/QqlFuzz/tool/isql", "isql", "-U", "SYSAUDIT/szoscar55", "-d", "DATEBASE1", "-c", cmd, (char*)nullptr);
        perror("execl");
        _exit(1);
    } else { // 父进程
        // 关闭子进程中的管道写端
        close(outPipe[1]);
        close(errPipe[1]);

        // 等待子进程结束
        waitpid(pid, nullptr, 0);
    }
}

int main() {
    int outPipe[2]; // 标准输出管道
    int errPipe[2]; // 标准错误管道

    if (pipe(outPipe) == -1 || pipe(errPipe) == -1) {
        perror("pipe");
        return 1;
    }

    pthread_t outputThread, errorThread;
    pthread_create(&outputThread, nullptr, readOutputStream, &outPipe[0]);
    pthread_create(&errorThread, nullptr, readErrorStream, &errPipe[0]);

    // 向子进程写入输入并处理交互
    chdir("/home/hkbin/Workspace/chaitin_workspace/database_fuzz/QqlFuzz/tool");
    std::string input_line;
    while (running) {
        // 读取用户输入
        std::cout << "Enter command: ";
        std::getline(std::cin, input_line);
        if (input_line.empty()) {
            // 空输入时退出循环
            running = false;
            break;
        }
        input_line += "\n"; // 追加换行符

        // 执行命令并获取输出
        executeCommand(input_line.c_str(), outPipe, errPipe);
    }

    // 关闭管道
    close(outPipe[0]);
    close(errPipe[0]);

    // 等待线程完成
    pthread_join(outputThread, nullptr);
    pthread_join(errorThread, nullptr);

    return 0;
}
