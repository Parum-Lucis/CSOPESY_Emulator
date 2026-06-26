#pragma once

class AConsole {
public:
    // A virtual destructor is required for abstract base classes to ensure 
    // proper cleanup of derived classes.
    virtual ~AConsole() = default;

    // Pure virtual functions (the "= 0" makes them abstract)
    virtual bool isRunning() const = 0;
    virtual void processInput() = 0;
    virtual void drawConsole() const = 0;
};