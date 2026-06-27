#pragma once

#include "ACommand.h"
#include <string>
#include <chrono>

#pragma once
#include "ACommand.h"
#include "Process.h"
#include <string>
#include <memory>

class SleepCommand : public ACommand {
private:
    std::weak_ptr<Process> currentProcess;
    std::string durationStr;

public:
    SleepCommand(std::shared_ptr<Process> process, std::string sleepDuration);

    void execute() override;
};