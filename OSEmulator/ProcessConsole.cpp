#include "ProcessConsole.h"
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <iomanip>

ProcessConsole::ProcessConsole(std::shared_ptr<Process> process)
    : attachedProcess(process), running(true), currentInput("")
{
    showCursor(false);
    system("cls");
    if (attachedProcess) {
        capturedLogs = attachedProcess->getLogs(); // Frozen snapshot
    }

    // Draw the static dashboard exactly ONCE when the window opens
    printDashboard();
}

ProcessConsole::~ProcessConsole() {
    showCursor(true);
    system("cls"); // Clean the screen when jumping back to main menu
}

bool ProcessConsole::isRunning() const {
    return running;
}

void ProcessConsole::printDashboard() const {
    // 1. Header Information
    std::cout << "Process name: " << attachedProcess->getName() << "\n";
    std::cout << "ID: " << attachedProcess->getPID() << "\n";
    std::cout << "Logs:\n";

    // 2. Print Logs (Capped at 15 to keep layout clean)
    size_t fixedLogHeight = 15;
    size_t startIndex = (capturedLogs.size() > fixedLogHeight) ? capturedLogs.size() - fixedLogHeight : 0;

    for (size_t i = 0; i < capturedLogs.size(); ++i) {
        if (startIndex + i < capturedLogs.size()) {
            std::cout << capturedLogs[startIndex + i] << "\n";
        }
    }

    std::cout << "\n";
    std::cout << "Current instruction line: " << attachedProcess->getCurrentLine() << "\n";
    std::cout << "Lines of code: " << attachedProcess->getTotalLines() << "\n\n";

    // 3. Finished State Marker
    if (attachedProcess->getState() == ProcessState::FINISHED) {
        std::cout << "Finished!\n\n";
    }
}

void ProcessConsole::processInput() {
    if (_kbhit()) {
        char ch = _getch();

        if (ch == '\r') { // Enter key pressed
            if (currentInput == "process-smi") {
                if (attachedProcess) {
                    // Drop down a line to permanently "lock in" the command they just typed
                    std::cout << "\n";

                    // Append the SMI output directly below it
                    std::string output = attachedProcess->getProcessSMI(currentInput, consoleWidth);
                    std::cout << output;
                }
                currentInput = ""; // Reset input buffer
            }
            else if (currentInput == "clear") {
                // Bonus feature: Type 'clear' to reset the view back to just the dashboard
                system("cls");
                printDashboard();
                currentInput = "";
            }
            else if (currentInput == "exit") {
                running = false;
            }
            else {
                // Unrecognized command - drop down a line so the prompt stays clean
                std::cout << "\n";
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
    // By completely removing setCursorPosition(0,0), the console can now scroll naturally!
    // This loop now ONLY redraws the prompt line.

    std::string prompt = "root:\\> " + currentInput;

    // std::setw pads the rest of the row with empty spaces to wipe out lingering characters on backspace
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