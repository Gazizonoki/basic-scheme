#include <iostream>
#include <string>
#include "scheme.h"

int main() {
    Interpreter interpreter;
    std::string str;
    while (std::getline(std::cin, str)) {
        std::cout << interpreter.Run(str) << std::endl;
    }
    return 0;
}
