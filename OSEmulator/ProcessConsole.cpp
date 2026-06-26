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

    // 2. Set a strict limit on how many logs to display at once
    size_t maxLogsToShow = 15; // Adjust this number based on your screen height preference

    // 3. Calculate where to start printing so we only get the end of the list
    size_t startIndex = (allLogs.size() > maxLogsToShow) ? allLogs.size() - maxLogsToShow : 0;

    // 4. Print only the restricted range
    for (size_t i = startIndex; i < allLogs.size(); ++i) {
        std::cout << allLogs[i] << "\n";
    }

    std::cout << "\n";

    std::cout << "Current instruction line: " << attachedProcess->getCurrentLine() << "\n";
    std::cout << "Lines of code: " << attachedProcess->getTotalLines() << "\n\n";

    // 2. Finished State Marker
    if (attachedProcess->getState() == ProcessState::FINISHED) {
        std::cout << "Finished!\n\n";
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