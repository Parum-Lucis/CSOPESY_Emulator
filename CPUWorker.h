#pragma once

#include <thread>
#include <memory>
#include <atomic>
#include "Process.h"

class CPU {
public:
    explicit CPU(int id);
    ~CPU();

    void start();

    void stop();

    [[nodiscard]] int getCoreID() const { return coreID; }
    [[nodiscard]] std::shared_ptr<Process> getCurrentProcess() const { return currentProcess; }

private:
    void runWorkerLoop();
    static void applyDelay();

    int coreID;
    std::shared_ptr<Process> currentProcess;

    std::thread workerThread;
    std::atomic<bool> isRunning;
};