#include "SubtractCommand.h"
#include <cctype>
#include <utility>

SubtractCommand::SubtractCommand(std::unordered_map<std::string, uint16_t>& processMemory,
                                 std::string  targetVar,
                                 std::string  operand1,
                                 std::string  operand2)
    : localMemory(processMemory), var(std::move(targetVar)), op1(std::move(operand1)), op2(std::move(operand2)) {}

void SubtractCommand::execute() {
    const uint32_t val1 = evaluateOperand(op1);
    const uint32_t val2 = evaluateOperand(op2);

    uint32_t result = 0;

    if (val1 > val2) {
        result = val1 - val2;
    }

    localMemory[var] = static_cast<uint16_t>(result);
}

uint32_t SubtractCommand::evaluateOperand(const std::string& operand) const {
    bool isNumber = true;
    for (const char c : operand) {
        if (!std::isdigit(c)) {
            isNumber = false;
            break;
        }
    }

    if (isNumber) {
        return std::stoul(operand);
    }
    if (!localMemory.contains(operand)) {
        localMemory[operand] = 0;
    }
    return localMemory[operand];
}
