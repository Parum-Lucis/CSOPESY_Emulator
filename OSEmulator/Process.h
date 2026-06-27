#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "ACommand.h" // FIXED: Removed double backslashes

enum class ProcessState {
    READY,
    RUNNING,
    WAITING,
    FINISHED
};

class Process {
public:
    Process(size_t id, std::string processName, size_t lines);

    std::unordered_map<std::string, uint16_t>& getLocalMemory();
    void executeNextInstruction();
    void printProcessSMI() const;

    void addCommand(std::shared_ptr<ACommand> command);

    [[nodiscard]] std::string getName() const;
    [[nodiscard]] size_t getPID() const { return pid; }
    [[nodiscard]] ProcessState getState() const { return state; }
    [[nodiscard]] size_t getCurrentLine() const { return currentLine; }
    [[nodiscard]] size_t getTotalLines() const { return totalLines; }
    void setState(ProcessState newState) { state = newState; }
    [[nodiscard]] std::string getCreationTime() const { return creationTime; }
    [[nodiscard]] int getCoreAssigned() const { return coreAssigned; }

    void setSleepTicks(int duration);
    int getSleepTicks() const;
    void decrementSleepTicks();

    // Add a setter for the CPUWorker to use
    void setCoreAssigned(int coreID) { coreAssigned = coreID; }

    // --- NEW ADDITION FOR TESTING ---
    static std::string stateToString(ProcessState state);

private:
    static [[nodiscard]] std::string generateTimestamp();

    size_t pid;
    std::string name;
    ProcessState state;
    int coreAssigned;
    size_t currentLine;
    size_t totalLines;
    std::string creationTime;

    std::unordered_map<std::string, uint16_t> localMemory;
    std::vector<std::shared_ptr<ACommand>> commandList{};

    int sleepTicks{};
};