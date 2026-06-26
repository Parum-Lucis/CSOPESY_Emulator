#pragma once
#include "AConsole.h"
#include <memory>
#include <stack>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>

// A factory function type to generate fresh console instances on demand.
// This allows seamless integration of future consoles without hardcoding them.
using ConsoleFactory = std::function<std::shared_ptr<AConsole>()>;

class ConsoleManager {
private:
    // A stack naturally manages the "current" (top) and "previous" (under top) states.
    std::stack<std::shared_ptr<AConsole>> consoleStack;

    // Registry to hold mapped console factories
    std::unordered_map<std::string, ConsoleFactory> consoleRegistry;

    // Mutex to ensure thread-safe access to the current console state
    mutable std::mutex managerMutex;

    // Tracks if the manager itself is running
    bool managerRunning;

public:
    ConsoleManager();
    ~ConsoleManager() = default;

    // Registers a new console type. 
    // E.g., manager.registerConsole("dummy", []() { return std::make_shared<DummyConsole>(); });
    void registerConsole(const std::string& name, ConsoleFactory factory);

    // Switches to a new console. This acts as the "non-exit command".
    // It pushes the new console to the stack, designating the old one as the "previous".
    void switchConsole(const std::string& name);

    // --- Core Manager Interface ---

    // Thread-safe check to see if a console is currently running
    bool isRunning() const;

    // Processes input for the current console and handles "exit" popping
    void processInput();

    // Renders the current console safely
    void drawConsole() const;
};