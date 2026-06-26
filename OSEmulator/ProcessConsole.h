#pragma once
#include "AConsole.h"
#include "Process.h"
#include <memory>
#include <string>

class ProcessConsole : public AConsole {
public:
    explicit ProcessConsole(std::shared_ptr<Process> process);
    virtual ~ProcessConsole();

    // Core interface methods overridden from AConsole
    void processInput() override;
    void drawConsole() const override;
    bool isRunning() const override;

private:
    std::shared_ptr<Process> attachedProcess;
    bool running;
    std::string currentInput;
    int consoleWidth = 80;

    // UI Utilities matching MainMenuConsole styling
    void setCursorPosition(int x, int y) const;
    void showCursor(bool show) const;
};