#include "DeclareCommand.h"
#include <cctype>
#include <utility>

DeclareCommand::DeclareCommand(std::unordered_map<std::string, uint16_t>& processMemory,
                               std::string  varName,
                               std::string  value)
    : localMemory(processMemory), var(std::move(varName)), val(std::move(value)) {}

void DeclareCommand::execute() {
    uint32_t evaluatedValue = 0;

    bool isNumber = true;
    for (const char c : val) {
        if (!std::isdigit(c)) {
            isNumber = false;
            break;
        }
    }

    if (isNumber) {
        evaluatedValue = std::stoul(val);
    } else {

        if (!localMemory.contains(val)) {
            localMemory[val] = 0;
        }
        evaluatedValue = localMemory[val];
    }

    if (evaluatedValue > UINT16_MAX) {
        evaluatedValue = UINT16_MAX;
    }

    localMemory[var] = static_cast<uint16_t>(evaluatedValue);
}