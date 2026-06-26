#pragma once
#include "ACommand.h"
#include <string>
#include <unordered_map>
#include <cstdint>

class DeclareCommand : public ACommand {
public:
    DeclareCommand(std::unordered_map<std::string, uint16_t>& processMemory,
                   std::string  varName,
                   std::string  value);

    void execute() override;

private:
    std::unordered_map<std::string, uint16_t>& localMemory;
    std::string var;
    std::string val;
};
