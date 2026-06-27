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
        commandList[currentLine]->execute();
        currentLine++;

        if (currentLine >= totalLines) {
            state = ProcessState::FINISHED;
        }
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

void Process::addCommand(std::shared_ptr<ACommand> command) {
    commandList.push_back(command);
}

std::unordered_map<std::string, uint16_t>& Process::getLocalMemory() {
    return localMemory;
}

void Process::setSleepTicks(int duration) {
    this->sleepTicks = duration;
}

int Process::getSleepTicks() const {
    return this->sleepTicks;
}

void Process::decrementSleepTicks() {
    if (this->sleepTicks > 0) {
        this->sleepTicks--;
    }
}