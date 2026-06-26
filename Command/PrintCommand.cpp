#include "PrintCommand.h"
#include <cctype>
#include <utility>

PrintCommand::PrintCommand(std::unordered_map<std::string, uint16_t>& processMemory,
                           std::string  toPrint)
    : localMemory(processMemory), target(std::move(toPrint)) {}

void PrintCommand::execute() {
    bool isNumber = true;
    for (const char c : target) {
        if (!std::isdigit(c)) {
            isNumber = false;
            break;
        }
    }

    if (isNumber) {
        std::cout << target << std::endl;
    } else {
        if (!localMemory.contains(target)) {
            localMemory[target] = 0;
        }
        std::cout << localMemory[target] << std::endl;
    }
}
