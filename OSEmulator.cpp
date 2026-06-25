#include "OSEmulator.h"
#include "ConfigManager.h"
#include "ProcessScheduler.h"
#include <iostream>
#include <sstream>

OSEmulator::OSEmulator() : isRunning(true), isInitialized(false) {}

OSEmulator::~OSEmulator() {
    ProcessScheduler::getInstance()->stop();
}

void OSEmulator::run() {
    printHeader();

    std::string input;
    while (isRunning) {
        std::cout << "root@csopesy:~# ";
        std::getline(std::cin, input);

        if (input.empty()) continue;

        handleCommand(input);
    }
}

void OSEmulator::handleCommand(const std::string& commandLine) {
    std::vector<std::string> args = tokenize(commandLine, ' ');
    const std::string& command = args[0];

    if (command == "initialize") {
        handleInitialize();
    }
    else if (command == "screen") {
        handleScreen(args);
    }
    else if (command == "scheduler-start") {
        handleSchedulerStart();
    }
    else if (command == "scheduler-stop") {
        handleSchedulerStop();
    }
    else if (command == "report-util") {
        handleReportUtil();
    }
    else if (command == "clear") {
        #if defined(_WIN32)
            system("cls");
        #else
            system("clear");
        #endif
    }
    else if (command == "exit") {
        std::cout << "Shutting down CSOPESY Emulator...\n";
        isRunning = false;
    }
    else {
        std::cout << command << ": command not found\n";
    }
}


void OSEmulator::handleInitialize() {
    if (isInitialized) {
        std::cout << "System is already initialized!\n";
        return;
    }

    ConfigManager::getInstance()->initialize("config.txt");
    isInitialized = true;
    std::cout << "System initialized successfully.\n";
}

void OSEmulator::handleSchedulerStart() const {
    if (!isInitialized) {
        std::cout << "Error: System must be initialized before starting the scheduler.\n";
        return;
    }

    ProcessScheduler::getInstance()->start();
    std::cout << "Scheduler started. Generating processes...\n";
}

void OSEmulator::handleSchedulerStop() const {
    if (!isInitialized) {
        std::cout << "Error: System is not initialized.\n";
        return;
    }

    ProcessScheduler::getInstance()->stop();
    std::cout << "Scheduler stopped safely.\n";
}

void OSEmulator::handleReportUtil() const {
    if (!isInitialized) {
        std::cout << "Error: System is not initialized.\n";
        return;
    }

    ProcessScheduler::generateReportUtil();
}

void OSEmulator::handleScreen(const std::vector<std::string>& args) const {
    if (!isInitialized) {
        std::cout << "Error: System is not initialized.\n";
        return;
    }

    if (args.size() < 2) {
        std::cout << "Usage: screen [-s | -r | -ls] [name]\n";
        return;
    }

    const std::string& flag = args[1];

    if (flag == "-ls") {
        std::cout << "Listing all screens (implementation pending)...\n";
    }
    else if (flag == "-s" && args.size() == 3) {
        const std::string& screenName = args[2];
        std::cout << "Creating screen: " << screenName << "\n";
    }
    else if (flag == "-r" && args.size() == 3) {
        const std::string& screenName = args[2];
        std::cout << "Resuming screen: " << screenName << "\n";
    }
    else {
        std::cout << "Invalid screen command format.\n";
    }
}

std::vector<std::string> OSEmulator::tokenize(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

void OSEmulator::printHeader() {
    //TODO PRINT QUATTRO
}
