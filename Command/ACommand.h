#pragma once

class ACommand {
public:
    virtual ~ACommand() = default;

    virtual void execute() = 0;
};