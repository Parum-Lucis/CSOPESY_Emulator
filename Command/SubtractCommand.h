#pragma once
#include "ACommand.h"
#include <string>
#include <unordered_map>
#include <cstdint>

class SubtractCommand : public ACommand {
public:
    SubtractCommand(std::unordered_map<std::string, uint16_t>& processMemory,
                    std::string  targetVar,
                    std::string  operand1,
                    std::string  operand2);

    void execute() override;

private:
    std::unordered_map<std::string, uint16_t>& localMemory;
    std::string var;
    std::string op1;
    std::string op2;

    [[nodiscard]] uint32_t evaluateOperand(const std::string& operand) const;
};
