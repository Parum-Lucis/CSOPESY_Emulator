#include "SleepCommand.h"
#include <stdexcept>
#include <utility>

SleepCommand::SleepCommand(std::shared_ptr<Process> process, std::string sleepDuration)
    : currentProcess(process), durationStr(std::move(sleepDuration)) { // CHANGED: direct assignment
}

void SleepCommand::execute() {
    try {
        int duration = std::stoi(durationStr);

        if (duration < 0) duration = 0;
        if (duration > 255) duration = 255;

        // CHANGED: Lock the weak_ptr to safely access the Process
        if (auto process = currentProcess.lock()) {
            process->setState(ProcessState::WAITING);
            process->setSleepTicks(duration);
        }

    }
    catch (const std::invalid_argument& e) {}
    catch (const std::out_of_range& e) {}
}