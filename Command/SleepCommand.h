#pragma once

#include "ACommand.h"
#include <string>
#include <chrono>

class SleepCommand : public ACommand{
    public:
        explicit SleepCommand(std::string  sleepDuration);

        void execute() override;

    private:
        std::remove_reference_t<std::string &> durationStr;
};



