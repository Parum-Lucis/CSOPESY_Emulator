#pragma once

// Dummy interface to satisfy Process.h compilation
class ACommand {
public:
    virtual ~ACommand() = default;

    // Pure virtual function to mimic a command execution
    virtual void execute() = 0;
};