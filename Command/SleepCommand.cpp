#include "SleepCommand.h"
#include <stdexcept>
#include <utility>

SleepCommand::SleepCommand(std::shared_ptr<Process> process, std::string sleepDuration)
    : currentProcess(std::move(process)), durationStr(std::move(sleepDuration)) {}

void SleepCommand::execute() {
    try {
        const int duration = std::stoi(durationStr);

        if (duration < 0) {
            return 0;
        }

        if (duration > 255) {
            return 255;
        }

        currentProcess->setState(ProcessState::WAITING);

        currentProcess->setSleepTicks(duration);

    } catch (const std::invalid_argument& e) {
    } catch (const std::out_of_range& e) {}
}