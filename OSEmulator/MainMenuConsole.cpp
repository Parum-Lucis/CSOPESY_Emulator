#include "MainMenuConsole.h"
#include "configManager.h" 
#include "ProcessScheduler.h" // Added ProcessScheduler include
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <conio.h>

MainMenuConsole::MainMenuConsole() : running(true), initialized(false), currentInput("") {
    showCursor(false);
    system("cls");

    std::cout << "=== CSOPESY OS Emulator Main Menu ===\n";
    std::cout << "=====================================\n";
    std::cout << "Type 'initialize' to setup the system.\n\n";
}

MainMenuConsole::~MainMenuConsole() {
    showCursor(true);
    std::cout << "\n\nExiting Main Menu Console...\n";
}

bool MainMenuConsole::isRunning() const {
    return running;
}

void MainMenuConsole::processInput() {
    if (_kbhit()) {
        char ch = _getch();
        if (ch == '\r') {
            std::string cleanPrompt = "[" + getCurrentDateTime() + "] > " + currentInput;
            std::cout << "\r" << std::left << std::setw(consoleWidth - 1) << cleanPrompt;

            if (currentInput == "exit") {
                running = false;
                std::cout << "\n";
            }
            else if (!currentInput.empty()) {
                std::string responseStr = "Command not recognized.";

                if (currentInput == "initialize") {
                    if (!initialized) {
                        ConfigManager::getInstance()->initialize();
                        initialized = true;

						//FOR TESTING: Display the loaded configuration
                        std::stringstream ss;
                        ss << "Command Recognized: initialize\n";
                        ss << "=> System initialized successfully.\n\n";
                        ss << "--- Loaded Configuration ---\n";
                        ss << "num-cpu: " << ConfigManager::getInstance()->getNumCPU() << "\n";
                        ss << "scheduler: " << ConfigManager::getInstance()->getScheduler() << "\n";
                        ss << "quantum-cycles: " << ConfigManager::getInstance()->getQuantumCycles() << "\n";
                        ss << "batch-process-freq: " << ConfigManager::getInstance()->getBatchProcessFreq() << "\n";
                        ss << "min-ins: " << ConfigManager::getInstance()->getMinIns() << "\n";
                        ss << "max-ins: " << ConfigManager::getInstance()->getMaxIns() << "\n";
                        ss << "delay-per-exec: " << ConfigManager::getInstance()->getDelayPerExec();

                        responseStr = ss.str();
                    }
                    else {
                        responseStr = "System is already initialized.";
                    }
                }
                else if (!initialized) {
                    responseStr = "Error: System not initialized. Please run 'initialize' first.";
                }
                else {
                    if (currentInput == "scheduler-start") {
                        // Guard: Check if it's already running
                        if (ProcessScheduler::getInstance()->isGeneratorRunning()) {
                            responseStr = "Command Recognized: scheduler-start\n=> Error: CPU Scheduler is already running.";
                        }
                        else {
                            ProcessScheduler::getInstance()->start();

                            // Wait briefly so the background thread can generate the first process
                            std::this_thread::sleep_for(std::chrono::milliseconds(50));

                            std::stringstream ss;
                            ss << "Command Recognized: scheduler-start\n";
                            ss << "=> CPU Scheduler and dummy process generation started.\n";

                            auto first = ProcessScheduler::getInstance()->getFirstProcess();
                            if (first) {
                                ss << "\n--- First Process Configuration ---\n";
                                ss << "Process Name: " << first->getName() << "\n";
                                ss << "PID: " << first->getPID() << "\n";
                                ss << "State: " << Process::stateToString(first->getState()) << "\n";
                                ss << "Total Lines: " << first->getTotalLines() << "\n";
                            }
                            else {
                                ss << "\n[Warning: Thread has not initialized the first process yet.]\n";
                            }

                            responseStr = ss.str();
                        }
                    }
                    else if (currentInput == "scheduler-stop") {
                        // Guard: Check if it's already stopped
                        if (!ProcessScheduler::getInstance()->isGeneratorRunning()) {
                            responseStr = "Command Recognized: scheduler-stop\n=> Error: CPU Scheduler is not currently running.";
                        }
                        else {
                            ProcessScheduler::getInstance()->stop();

                            std::stringstream ss;
                            ss << "Command Recognized: scheduler-stop\n";
                            ss << "=> CPU Scheduler dummy process generation stopped.\n";

                            auto latest = ProcessScheduler::getInstance()->getLatestProcess();
                            size_t qSize = ProcessScheduler::getInstance()->getReadyQueueSize();

                            ss << "\n--- Scheduler Status ---\n";
                            if (latest) {
                                ss << "Latest Process Generated: " << latest->getName() << "\n";
                                ss << "Latest PID: " << latest->getPID() << "\n";
                                ss << "State: " << Process::stateToString(latest->getState()) << "\n";
                                ss << "Total Lines: " << latest->getTotalLines() << "\n";
                            }
                            else {
                                ss << "Latest Process Generated: None\n";
                            }
                            ss << "Current Ready Queue Size: " << qSize << "\n";

                            responseStr = ss.str();
                        }
                    }
                    else if (currentInput == "report-util" || currentInput == "screen -ls") {
                        auto scheduler = ProcessScheduler::getInstance();

                        size_t coresTotal = ConfigManager::getInstance()->getNumCPU();
                        size_t coresUsed = scheduler->getCoresUsed();
                        size_t coresAvailable = coresTotal - coresUsed;
                        double cpuUtil = coresTotal > 0 ? (static_cast<double>(coresUsed) / coresTotal) * 100.0 : 0.0;

                        std::stringstream ss;
                        ss << "CPU Utilization: " << std::fixed << std::setprecision(0) << cpuUtil << "%\n";
                        ss << "Cores used: " << coresUsed << "\n";
                        ss << "Cores available: " << coresAvailable << "\n";
                        ss << "----------------------------------------------\n";

                        auto allProcesses = scheduler->getAllProcesses();
                        int runningCount = 0;
                        int finishedCount = 0;
                        std::stringstream runningStream;
                        std::stringstream finishedStream;

                        runningStream << "Running Processes:\n";
                        finishedStream << "Finished Processes:\n";

                        // NOTE: Filtered to only display RUNNING and FINISHED processes per instructions
                        for (const auto& p : allProcesses) {
                            if (p->getState() == ProcessState::RUNNING) {
                                // Formatting relies on Process::getCreationTime() returning "(MM/DD/YYYY HR:MN:SSAM/PM)"
                                runningStream << std::left << std::setw(6) << p->getName() << "   "
                                    << p->getCreationTime() << "   Core: " << std::right 
                                    << std::setw(2) << p->getCoreAssigned() << "   "
                                    << std::right << std::setw(5) << p->getCurrentLine() << " / "
                                    << std::right << std::setw(5) << p->getTotalLines() << "\n";
                                runningCount++;
                            }
                            else if (p->getState() == ProcessState::FINISHED) {
                                finishedStream << std::left << std::setw(6) << p->getName() << "   "
                                    << p->getCreationTime() << "   Finished" << "   "
                                    << std::right << std::setw(5) << p->getCurrentLine() << " / "
                                    << std::right << std::setw(5) << p->getTotalLines() << "\n";
                                finishedCount++;
                            }
                        }

                        if (runningCount == 0) runningStream << "No running processes.\n";
                        if (finishedCount == 0) finishedStream << "No finished processes.\n";

                        ss << runningStream.str() << "\n" << finishedStream.str();
                        ss << "----------------------------------------------\n";

                        if (currentInput == "screen -ls") {
                            responseStr = "Command Recognized: screen -ls\n\n" + ss.str();
                        }
                        else {
                            std::ofstream logFile("csopesy-log.txt", std::ios::out);
                            if (logFile.is_open()) {
                                logFile << ss.str();
                                logFile.close();
                                responseStr = "Command Recognized: report-util\n=> Report saved to csopesy-log.txt.";
                            }
                            else {
                                responseStr = "Command Recognized: report-util\n=> Error: Could not open csopesy-log.txt for writing.";
                            }
                        }
                    }
                    else if (currentInput.length() >= 10 && currentInput.substr(0, 10) == "screen -s ") {
                        std::string processName = currentInput.substr(10);
                        responseStr = "Command Recognized: screen -s (Process Name: " + processName + ")";
                        // FUTURE IMPLEMENTATION PLACEHOLDER
                    }
                    else if (currentInput.length() >= 10 && currentInput.substr(0, 10) == "screen -r ") {
                        std::string processName = currentInput.substr(10);
                        responseStr = "Command Recognized: screen -r (Process Name: " + processName + ")";
                        // FUTURE IMPLEMENTATION PLACEHOLDER
                    }
                }

                std::cout << "\n" << responseStr << "\n";
                currentInput = "";
            }
            else {
                std::cout << "\n";
            }
        }
        else if (ch == '\b') {
            if (!currentInput.empty()) currentInput.pop_back();
        }
        else if (ch >= 32 && ch <= 126) {
            currentInput += ch;
        }
    }
}

void MainMenuConsole::drawConsole() const {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    bool showBlinker = (ms / 500) % 2 == 0;
    std::string cursorChar = showBlinker ? "|" : " ";

    std::string prompt = "[" + getCurrentDateTime() + "] > " + currentInput + cursorChar;
    std::cout << "\r" << std::left << std::setw(consoleWidth - 1) << prompt;
}

void MainMenuConsole::setCursorPosition(int x, int y) const {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void MainMenuConsole::showCursor(bool show) const {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = show;
    SetConsoleCursorInfo(out, &cursorInfo);
}

std::string MainMenuConsole::getCurrentDateTime() const {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_info;
    localtime_s(&tm_info, &time);
    std::stringstream ss;
    ss << std::put_time(&tm_info, "%m/%d/%Y %I:%M:%S%p");
    return ss.str();
}