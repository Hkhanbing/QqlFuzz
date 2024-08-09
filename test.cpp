#include "ProcessHandler.h"
#include <iostream>

int main() {
    ProcessHandler handler;
    handler.start("/home/hkbin/Workspace/chaitin_workspace/database_fuzz/QqlFuzz/tool");

    std::string input_line;
    while (true) {
        std::cout << "Enter command: ";
        std::getline(std::cin, input_line);
        if (input_line.empty()) {
            handler.stop();
            break;
        }
        input_line += "\n";
        handler.executeCommand(input_line);
    }

    return 0;
}
