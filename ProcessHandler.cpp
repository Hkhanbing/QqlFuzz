#include "ProcessHandler.h"
#include <unistd.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstring>
#include <iostream>

ProcessHandler::ProcessHandler() : running(true) {
    pipe(outPipe);
    pipe(errPipe);
}

ProcessHandler::~ProcessHandler() {
    stop();
}

bool ProcessHandler::start(const std::string& workingDir) {
    this->workingDir = workingDir;
    outputThread = std::thread(&ProcessHandler::readOutputStream, this);
    errorThread = std::thread(&ProcessHandler::readErrorStream, this);
    return true;
}

bool ProcessHandler::executeCommand(const std::string& cmd) {
    return execute(cmd);
}

void ProcessHandler::stop() {
    running = false;
    close(outPipe[0]);
    close(errPipe[0]);
    if (outputThread.joinable()) outputThread.join();
    if (errorThread.joinable()) errorThread.join();
}

void ProcessHandler::readOutputStream() {
    char buffer[0x1000];
    FILE* fp = fdopen(outPipe[0], "r");
    if (!fp) {
        perror("fdopen");
        return;
    }

    while (running && std::fgets(buffer, sizeof(buffer), fp)) {
        std::cout << buffer;
        std::memset(buffer, 0, sizeof(buffer));
    }

    fclose(fp);
}

void ProcessHandler::readErrorStream() {
    char buffer[0x1000];
    FILE* fp = fdopen(errPipe[0], "r");
    if (!fp) {
        perror("fdopen");
        return;
    }

    while (running && std::fgets(buffer, sizeof(buffer), fp)) {
        std::cerr << buffer;
        std::memset(buffer, 0, sizeof(buffer));
    }

    fclose(fp);
}

bool ProcessHandler::execute(const std::string& cmd) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return false;
    }

    if (pid == 0) {
        // Child process
        close(outPipe[0]);
        close(errPipe[0]);
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(errPipe[1], STDERR_FILENO);
        close(outPipe[1]);
        close(errPipe[1]);

        chdir(workingDir.c_str());
        execl("/home/hkbin/Workspace/chaitin_workspace/database_fuzz/QqlFuzz/tool/isql",
              "isql", "-U", "SYSAUDIT/szoscar55", "-d", "DATEBASE1", "-c", cmd.c_str(), (char*)nullptr);
        perror("execl");
        _exit(1);
    } else {
        // Parent process
        close(outPipe[1]);
        close(errPipe[1]);
        waitpid(pid, nullptr, 0);
    }

    return true;
}
