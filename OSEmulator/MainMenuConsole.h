#pragma once
#include "AConsole.h"
#include <string>

class MainMenuConsole : public AConsole {
private:
    bool running;
    bool initialized; // Constraint 1: Track if "initialize" has been called
    std::string currentInput;
    std::string lastCommandOutput;

    const int consoleWidth = 120;

    // Private Helper Methods
    void setCursorPosition(int x, int y) const;
    void showCursor(bool show) const;
    std::string getCurrentDateTime() const;

public:
    MainMenuConsole();
    ~MainMenuConsole() override;

    bool isRunning() const override;
    void processInput() override;
    void drawConsole() const override;
};