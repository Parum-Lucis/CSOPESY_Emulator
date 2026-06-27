#include "MainMenuConsole.h"
#include "configManager.h" 
#include "ProcessScheduler.h"
#include "ConsoleManager.h"
#include "ProcessConsole.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <windows.h>
#include <conio.h>
#include <algorithm>

MainMenuConsole::MainMenuConsole() : running(true), initialized(false), currentInput("") {
    showCursor(false);
    system("cls");
    // Set the initial default welcome message
    lastCommandOutput = "Type 'initialize' to setup the system.\n";
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
            if (currentInput == "exit") {
                ProcessScheduler::getInstance()->stop();
                running = false;
                system("cls");
            }
            else if (!currentInput.empty()) {
                std::string responseStr = "Command not recognized.";

                if (currentInput == "initialize") {
                    if (!initialized) {
                        ConfigManager::getInstance()->initialize();
                        initialized = true;

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
                        if (ProcessScheduler::getInstance()->isGeneratorRunning()) {
                            responseStr = "Command Recognized: scheduler-start\n=> Error: CPU Scheduler is already running.";
                        }
                        else {
                            ProcessScheduler::getInstance()->start();
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

                        for (const auto& p : allProcesses) {
                            if (p->getState() == ProcessState::RUNNING) {
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

                        // Trim trailing whitespace characters to prevent name mismatches
                        processName.erase(processName.find_last_not_of(" \n\r\t") + 1);

                        // Call the thread-safe scheduler method
                        bool success = ProcessScheduler::getInstance()->createManualProcess(processName);

                        if (!success) {
                            responseStr = "Error: Process '" + processName + "' already exists.";
                        }
                        else {
                            // Find the newly created process from the scheduler to attach to the console view
                            auto processList = ProcessScheduler::getInstance()->getAllProcesses();
                            auto it = std::find_if(processList.begin(), processList.end(),
                                [&processName](const std::shared_ptr<Process>& procPtr) {
                                    return procPtr && procPtr->getName() == processName;
                                });

                            if (it != processList.end()) {
                                std::shared_ptr<Process> targetProcess = *it;
                                std::string screenName = "screen_" + processName;

                                // Register the sub-console for this new process
                                ConsoleManager::getInstance()->registerConsole(screenName, [targetProcess]() {
                                    return std::make_shared<ProcessConsole>(targetProcess);
                                    });

                                // Clear out the Main Menu state variables completely
                                this->lastCommandOutput = "";
                                this->currentInput = "";

                                // Clear the terminal screen and instantly switch the view
                                system("cls");
                                ConsoleManager::getInstance()->switchConsole(screenName);
                                ConsoleManager::getInstance()->drawConsole();

                                return; // Exit out immediately to hand control over to the new sub-console
                            }
                            else {
                                responseStr = "Error: Process was created but could not be resolved in the tracking list.";
                            }
                        }
                        }
                    else if (currentInput.length() >= 10 && currentInput.substr(0, 10) == "screen -r ") {
                        std::string processName = currentInput.substr(10);


                        // Fetch all processes and find the one that matches the requested name
                        auto processList = ProcessScheduler::getInstance()->getAllProcesses();
                        auto it = std::find_if(processList.begin(), processList.end(),
                            [&processName](const std::shared_ptr<Process>& procPtr) {
                                return procPtr && procPtr->getName() == processName;
                            });

                        if (it != processList.end()) {
                            std::shared_ptr<Process> targetProcess = *it;
                            std::string screenName = "screen_" + processName;

                            if (targetProcess->getState() == ProcessState::FINISHED) {
                                responseStr = "Process " + processName + " has already finished.";
                            }
                            else {

                                // Register the sub-console (overwriting the old lambda is perfectly safe)
                                ConsoleManager::getInstance()->registerConsole(screenName, [targetProcess]() {
                                    return std::make_shared<ProcessConsole>(targetProcess);
                                    });

                                // Clear out the Main Menu state variables completely
                                this->lastCommandOutput = "";
                                this->currentInput = "";

                                // Clear the screen and switch
                                system("cls");
                                ConsoleManager::getInstance()->switchConsole(screenName);
                                ConsoleManager::getInstance()->drawConsole();

                                return;
                            }
                        }
                        else {
                            responseStr = "Error: Process '" + processName + "' not found.";
                        }
                    }
                }

                lastCommandOutput = responseStr;
                currentInput = "";
                system("cls");
            }
            else {
                system("cls"); // Clear screen if user hits enter on blank line
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
    // 1. Force hardware block cursor to stay hidden to eliminate hardware jitter
    showCursor(false);

    // 2. Assemble the entire screen layout in an isolated memory buffer
    std::stringstream frame;

    frame << "=== CSOPESY OS Emulator Main Menu ===\n";
    frame << "=====================================\n";

    if (!lastCommandOutput.empty()) {
        frame << lastCommandOutput << "\n\n";
    }

    // 3. Process the dynamic engine clock and text blinker string
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    bool showBlinker = (ms / 500) % 2 == 0;
    std::string cursorChar = showBlinker ? "|" : " ";

    std::string prompt = "[" + getCurrentDateTime() + "] > " + currentInput + cursorChar;

    // Safety Guard: If consoleWidth is uninitialized or 0, fallback to 80 
    // to prevent stream alignment overflows pushing text away
    int safeWidth = (consoleWidth > 0 && consoleWidth < 1000) ? consoleWidth : 80;

    frame << std::left << std::setw(safeWidth - 1) << prompt;

    // 4. Snap the cursor to the origin and print the whole frame instantly
    setCursorPosition(0, 0);
    std::cout << frame.str();
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