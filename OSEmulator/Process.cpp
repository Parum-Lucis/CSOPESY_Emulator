#include "Process.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

Process::Process(size_t id, std::string  processName, size_t lines)
    : pid(id), name(std::move(processName)), state(ProcessState::READY), coreAssigned(-1), currentLine(0), totalLines(lines)
{
    this->creationTime = generateTimestamp();
}

void Process::executeNextInstruction() {
    if (currentLine < totalLines && state == ProcessState::RUNNING) {

        // ADDED: Safety check to prevent out-of-bounds vector access
        if (currentLine < commandList.size() && commandList[currentLine] != nullptr) {
            commandList[currentLine]->execute();
        }

        // We still increment the line so the simulation progresses
        currentLine++;

        if (currentLine >= totalLines) {
            state = ProcessState::FINISHED;
        }
    }
}

void Process::printProcessSMI() const {
    // TODO ADD CORRECT PRINT PROCESS SMI;
    std::cout << "Process name: " << name << "\n";
    std::cout << "ID: " << pid << "\n\n";

    if (state == ProcessState::FINISHED) {
        std::cout << "Finished!\n";
    } else {
        std::cout << "Current instruction line: " << currentLine << "\n";
        std::cout << "Lines of code: " << totalLines << "\n";
    }
}

std::string Process::getName() const {
    return name;
}

std::string Process::generateTimestamp() {
    auto t = std::time(nullptr);
    std::tm tm = {};

    #if defined(_WIN32) || defined(_WIN64)
        localtime_s(&tm, &t);
    #else
        localtime_r(&t, &tm);
    #endif

    std::ostringstream oss;
    oss << "(" << std::put_time(&tm, "%m/%d/%Y %I:%M:%S%p") << ")";
    return oss.str();
}

// --- NEW ADDITION FOR TESTING ---
std::string Process::stateToString(ProcessState state) {
    switch (state) {
    case ProcessState::READY: return "READY";
    case ProcessState::RUNNING: return "RUNNING";
    case ProcessState::WAITING: return "WAITING";
    case ProcessState::FINISHED: return "FINISHED";
    default: return "UNKNOWN";
    }
}