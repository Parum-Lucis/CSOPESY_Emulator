#include "ProcessConsole.h"
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <iomanip>
#include <chrono>

ProcessConsole::ProcessConsole(std::shared_ptr<Process> process)
    : attachedProcess(process), running(true), currentInput("")
{
    showCursor(false);
    system("cls");
}

ProcessConsole::~ProcessConsole() {
    showCursor(true);
    system("cls"); // Clean the screen when jumping back to main menu
}

bool ProcessConsole::isRunning() const {
    return running;
}

void ProcessConsole::processInput() {
    if (_kbhit()) {
        char ch = _getch();

        if (ch == '\r') { // Enter key pressed
            if (currentInput == "process-smi") {
                // Clear the screen first to make room for the print
                //system("cls");

                // Call the process function, passing a lambda that calls our draw method
                if (attachedProcess) {
                    attachedProcess->printProcessSMI(currentInput, consoleWidth);
                }

                currentInput = ""; // Reset input buffer
            }
            if (currentInput == "exit") {
                running = false;
            }
            else {
                // If they type anything else, just clear the field or handle other sub-commands
                currentInput = "";
            }
        }
        else if (ch == '\b') { // Backspace
            if (!currentInput.empty()) {
                currentInput.pop_back();
            }
        }
        else if (ch >= 32 && ch <= 126) { // Printable characters
            currentInput += ch;
        }
    }
}

void ProcessConsole::drawConsole() const {
    // Reset cursor to top left to rewrite smoothly without screen-clearing flicker
    setCursorPosition(0, 0);

    // 1. Header Information matching your spec layout
    std::cout << "Process name: " << attachedProcess->getName() << "\n";
    std::cout << "ID: " << attachedProcess->getPID() << "\n";

    std::cout << "Logs:\n";

    // 1. Get the full list of logs
    const auto& allLogs = attachedProcess->getLogs();

    // 2. Filter out execution steps to extract only non-stepping completed milestones
    std::vector<std::string> completedTaskLogs;
    for (const auto& log : allLogs) {
        // If the log contains "executed line:", skip it so we only track task milestones
        if (log.find("executed line:") != std::string::npos) {
            continue;
        }
        completedTaskLogs.push_back(log);
    }

    // 3. Set a strict limit on how many milestone logs to display at once
    size_t maxLogsToShow = 15;

    // 4. Calculate where to start printing based on our filtered list
    size_t startIndex = (completedTaskLogs.size() > maxLogsToShow) ? completedTaskLogs.size() - maxLogsToShow : 0;

    // 5. Print only the restricted range of completed task milestones
    for (size_t i = startIndex; i < completedTaskLogs.size(); ++i) {
        // Pads each line out to consoleWidth to wipe out older residual text cleanly
        std::cout << std::left << std::setw(consoleWidth) << completedTaskLogs[i] << "\n";
    }

    // Pad structural fields to prevent lingering character ghosting from old text
    std::cout << std::left << std::setw(consoleWidth) << "" << "\n";

    std::string currLineStr = "Current instruction line: " + std::to_string(attachedProcess->getCurrentLine());
    std::cout << std::left << std::setw(consoleWidth) << currLineStr << "\n";

    std::string totalLinesStr = "Lines of code: " + std::to_string(attachedProcess->getTotalLines());
    std::cout << std::left << std::setw(consoleWidth) << totalLinesStr << "\n\n";

    // 2. Finished State Marker
    if (attachedProcess->getState() == ProcessState::FINISHED) {
        std::cout << std::left << std::setw(consoleWidth) << "Finished!" << "\n\n";
    }

    // 3. Command Line Prompt Rendering (drawn strictly ONCE at the very bottom)
    std::string prompt = "root:\\> " + currentInput;

    // std::setw pads the line with empty spaces to cleanly wipe out lingering characters
    std::cout << "\r" << std::left << std::setw(consoleWidth - 1) << prompt;
}
void ProcessConsole::setCursorPosition(int x, int y) const {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void ProcessConsole::showCursor(bool show) const {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = show;
    SetConsoleCursorInfo(out, &cursorInfo);
}