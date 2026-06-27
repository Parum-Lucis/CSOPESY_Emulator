#include "Process.h"
#include "PrintCommand.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>
#include <windows.h>
#include <iomanip>

Process::Process(size_t id, std::string  processName, size_t lines)
    : pid(id), name(std::move(processName)), state(ProcessState::READY), coreAssigned(-1), currentLine(0), totalLines(lines)
{
    this->creationTime = generateTimestamp();
}

void Process::executeNextInstruction() {
    if (currentLine < totalLines && state == ProcessState::RUNNING) {
        bool isPrintCommand = false;
        std::string printedValue = "";
        // 1. Execute the command (Kept safety checks from HEAD to prevent crashes)
        if (currentLine < commandList.size() && commandList[currentLine] != nullptr) {
            commandList[currentLine]->execute();
            
            auto printCmd = std::dynamic_pointer_cast<PrintCommand>(commandList[currentLine]);
            if (printCmd != nullptr) {
                isPrintCommand = true;
                printedValue = printCmd->getPrintValue(); // Capture the specific print value!
            }
        }

        // 2. GENERATE LOG HERE (It must be before the finish check!)
       if (isPrintCommand) {
            std::stringstream ss;
            ss << "Core: " << coreAssigned 
               << " \"" << printedValue << "\""; // Appends the print value securely
            addLog(ss.str());
        }

        // 3. Increment the line
        currentLine++;

        // 4. Check if finished
        if (currentLine >= totalLines) {
            state = ProcessState::FINISHED;
            addLog("Process finished execution."); // This one ONLY prints at the end
        }
    }
}

std::string Process::getProcessSMI(const std::string& currentInput, int consoleWidth) const {
    // 1. Build the ENTIRE output block first in memory so background threads don't interrupt it
    std::stringstream output;

    // Add a couple of newlines to separate it from the previous history cleanly
    output << "\n\n";
    output << "Process name: " << name << "\n";
    output << "ID: " << pid << "\n";
    output << "Logs:\n";

    // 2. Safely get the restricted range of logs
    const auto& allLogs = logs;
    size_t maxLogsToShow = 15;
    size_t startIndex = (allLogs.size() > maxLogsToShow) ? allLogs.size() - maxLogsToShow : 0;

    for (size_t i = startIndex; i < allLogs.size(); ++i) {
        output << allLogs[i] << "\n";
    }

    output << "\n";
    output << "Current instruction line: " << currentLine << "\n";
    output << "Lines of code: " << totalLines << "\n\n";

    // 3. Finished State Marker
    if (state == ProcessState::FINISHED) {
        output << "Finished!\n\n";
    }

    // 4. Print the entire block to the console at once, naturally scrolling the screen down
    return output.str();

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

void Process::addLog(const std::string& message) {
    // Appends the timestamp and the message to the vector
    logs.push_back(generateTimestamp() + " " + message);
}