#include "ConsoleManager.h"

ConsoleManager::ConsoleManager() : managerRunning(true) {}

void ConsoleManager::registerConsole(const std::string& name, ConsoleFactory factory) {
    std::lock_guard<std::mutex> lock(managerMutex);
    consoleRegistry[name] = factory;
}

void ConsoleManager::switchConsole(const std::string& name) {
    std::lock_guard<std::mutex> lock(managerMutex);

    auto it = consoleRegistry.find(name);
    if (it != consoleRegistry.end()) {
        // Instantiate the new console and push it to the stack.
        // The previous console is safely suspended right beneath it on the stack.
        consoleStack.push(it->second());
    }
}

bool ConsoleManager::isRunning() const {
    std::lock_guard<std::mutex> lock(managerMutex);
    // The manager is running as long as it hasn't been flagged to stop 
    // and there is at least one active console in the stack.
    return managerRunning && !consoleStack.empty();
}

void ConsoleManager::processInput() {
    std::lock_guard<std::mutex> lock(managerMutex);

    if (consoleStack.empty()) {
        managerRunning = false;
        return;
    }

    auto currentConsole = consoleStack.top();

    // Pass execution down to the active console
    currentConsole->processInput();

    // Constraint 1: Handle the "exit" command cleanly.
    // If the current console's isRunning() returns false (it exited itself),
    // we pop it from the stack. It is destroyed and NEVER designated as "previous".
    // The stack automatically surfaces the actual previous console back to "current".
    if (!currentConsole->isRunning()) {
        consoleStack.pop();

        // If exiting that console leaves us with nothing, shut down the manager.
        if (consoleStack.empty()) {
            managerRunning = false;
        }
    }
}

void ConsoleManager::drawConsole() const {
    std::lock_guard<std::mutex> lock(managerMutex);

    if (!consoleStack.empty()) {
        consoleStack.top()->drawConsole();
    }
}