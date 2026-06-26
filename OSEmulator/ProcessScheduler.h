#pragma once

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include "Process.h"
#include "CPUWorker.h"

class ProcessScheduler {
public:
    static ProcessScheduler* getInstance();

    void start();
    void stop();

    void addProcess(const std::shared_ptr<Process>& process);
    std::shared_ptr<Process> fetchNextProcess();
    void requeueProcess(const std::shared_ptr<Process>& process);

    static void generateReportUtil();

    // --- NEW ADDITIONS FOR TESTING ---
    [[nodiscard]] size_t getReadyQueueSize() const;
    [[nodiscard]] std::shared_ptr<Process> getFirstProcess() const;
    [[nodiscard]] std::shared_ptr<Process> getLatestProcess() const;

    // ADD THIS GETTER
    [[nodiscard]] bool isGeneratorRunning() const { return isRunning; }

    // --- UI/REPORT REPORTING ADDITIONS ---
    [[nodiscard]] std::vector<std::shared_ptr<Process>> getAllProcesses() const;
    [[nodiscard]] size_t getCoresUsed() const;

private:
    ProcessScheduler() : isRunning(false) {}
    ~ProcessScheduler() = default;

    // To keep track of the next PID to assign to a new process
    size_t globalProcessCounter = 1;

    std::queue<std::shared_ptr<Process>> readyQueue;
    std::vector<std::shared_ptr<Process>> allProcessList; // Master record of all processes

    std::atomic<uint64_t> totalSystemTicks{ 0 };
    std::atomic<uint64_t> activeCpuTicks{ 0 };

    mutable std::mutex queueMutex;

    std::vector<std::shared_ptr<CPU>> cpuWorkers{};

    std::thread generatorThread;
    std::atomic<bool> isRunning;
    void batchGeneratorLoop();

    // Stats Tracking
    std::shared_ptr<Process> firstProcess;
    std::shared_ptr<Process> latestProcess;
    mutable std::mutex statsMutex; // Reused to protect allProcessList as well
};