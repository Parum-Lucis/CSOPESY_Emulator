#pragma once

#include <string>
#include <vector>
#include <atomic>

class OSEmulator {
public:
    OSEmulator();
    ~OSEmulator();

    void run();

private:
    bool isRunning;
    bool isInitialized;

    static void printHeader() ;
    void handleCommand(const std::string& commandLine);
    static std::vector<std::string> tokenize(const std::string& str, char delimiter) ;

    void handleInitialize();
    void handleScreen(const std::vector<std::string>& args) const;
    void handleSchedulerStart() const;
    void handleSchedulerStop() const;
    void handleReportUtil() const;
};
