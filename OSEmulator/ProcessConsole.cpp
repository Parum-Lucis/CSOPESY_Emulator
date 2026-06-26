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
    // Reset cursor to top left to rewrite without flickery screen-clears
    setCursorPosition(0, 0);

    // Safely output current details about the running process
    std::cout << std::left << std::setw(20) << "Process Name:" << attachedProcess->getName() << "\n";
    std::cout << std::left << std::setw(20) << "PID:" << attachedProcess->getPID() << "\n";
    std::cout << "Logs:\n";
    //for (const auto& logLine : attachedProcess->getLogs()) {
    //    std::cout << logLine << "\n";
    //}

    std::cout << std::left << std::setw(20) << "Current instruction line:" << attachedProcess->getCurrentLine() << "\n";
    std::cout << std::left << std::setw(20) << "Lines of code:" << attachedProcess->getTotalLines() << "\n";

    if (attachedProcess->getState() == ProcessState::FINISHED) {
        std::cout << "Finished!\n\n";
    }
    std::string prompt = "root:\\> " + currentInput;
    std::cout << "\r" << std::left << std::setw(consoleWidth - 1) << prompt;
    
    
    
    int core = attachedProcess->getCoreAssigned();
    std::cout << std::left << std::setw(20) << "Core Assigned:";
    if (core == -1) std::cout << "None\n";
    else std::cout << core << "\n";
    std::cout << "==================================================\n";

    if (attachedProcess->getState() == ProcessState::FINISHED) {
        std::cout << "Process execution completed successfully.\n";
    }
    else {
        // Calculate progress percentage
        double progress = attachedProcess->getTotalLines() > 0
            ? (static_cast<double>(attachedProcess->getCurrentLine()) / attachedProcess->getTotalLines()) * 100.0
            : 0.0;
        std::cout << "Progress: " << std::fixed << std::setprecision(1) << progress << "%\n";
    }

    std::cout << "Type 'exit' to return to the Main Menu.\n\n";

    // Handle interactive command-line blinking representation at the bottom
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    bool showBlinker = (ms / 500) % 2 == 0;
    std::string cursorChar = showBlinker ? "|" : " ";

    //std::string prompt = "root@" + attachedProcess->getName() + " > " + currentInput + cursorChar;

    // Use line clearing spaces to handle changing prompt text widths cleanly
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