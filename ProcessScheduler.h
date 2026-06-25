//
// Created by Denzel Macayan on 6/25/2026.
//

#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include "Process.h"
#include "CPU.h"

class ProcessScheduler {
public:
    static ProcessScheduler* getInstance();

    void start();
    void stop();

    void addProcess(const std::shared_ptr<Process>& process);
    std::shared_ptr<Process> fetchNextProcess();
    void requeueProcess(std::shared_ptr<Process> process);

    static void generateReportUtil() ;

private:
    ProcessScheduler() : isRunning(false) {}
    ~ProcessScheduler() = default;

    std::queue<std::shared_ptr<Process>> readyQueue;

    mutable std::mutex queueMutex;

    std::vector<std::shared_ptr<CPU>> cpuWorkers{};

    std::thread generatorThread;
    std::atomic<bool> isRunning;
    void batchGeneratorLoop();
};
