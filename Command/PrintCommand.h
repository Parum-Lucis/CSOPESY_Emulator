#pragma once
#include "ACommand.h"
#include <string>
#include <unordered_map>
#include <cstdint>
#include <iostream>

class PrintCommand : public ACommand {
public:
    PrintCommand(std::unordered_map<std::string, uint16_t>& processMemory,
                 std::string  toPrint);

    void execute() override;

private:
    std::unordered_map<std::string, uint16_t>& localMemory;
    std::string target;
};
