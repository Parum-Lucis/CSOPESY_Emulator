#pragma once
#include <string>
#include <iostream>

class ACommand {
public:
    virtual ~ACommand() = default;

    virtual void execute() = 0;
};