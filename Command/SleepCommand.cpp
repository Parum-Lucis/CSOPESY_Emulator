#include "SleepCommand.h"
#include <thread>
#include <iostream>
#include <stdexcept>
#include <utility>

SleepCommand::SleepCommand(std::string  sleepDuration)
    : durationStr(std::move(sleepDuration)){}

void SleepCommand::execute() {
    try {
        const int duration = std::stoi(durationStr);

        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    } catch (const std::invalid_argument& e) {
        std::cerr << "Emulator Error: Invalid sleep duration format '" << durationStr << "'." << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Emulator Error: sleep duration '" << durationStr << "'is out of range." << std::endl;
    }
}