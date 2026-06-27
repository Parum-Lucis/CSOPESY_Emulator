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

    ProcessScheduler(const ProcessScheduler&) = delete;
    ProcessScheduler& operator=(const ProcessScheduler&) = delete;

    void start();
    void stop();

    void addProcess(const std::shared_ptr<Process>& process);
    std::shared_ptr<Process> fetchNextProcess();
    void requeueProcess(const std::shared_ptr<Process>& process);

    void toggleDummyGeneration(bool state);

    static void generateReportUtil();

private:
    ProcessScheduler() : isRunning(false), isGeneratingDummy(false) {}
    ~ProcessScheduler() = default;

    std::queue<std::shared_ptr<Process>> readyQueue;
    std::vector<std::shared_ptr<Process>> waitQueue;

    std::atomic<uint64_t> totalSystemTicks{0};
    std::atomic<uint64_t> activeCpuTicks{0};

    mutable std::mutex queueMutex;

    std::vector<std::shared_ptr<CPU>> cpuWorkers{};

    std::thread generatorThread;
    std::thread schedulerThread;

    std::atomic<bool> isRunning;
    std::atomic<bool> isGeneratingDummy;

    void schedulerLoop();
    void dummyGenerationLoop();

    static void generateDummyProcess(const std::shared_ptr<Process>& newProcess, size_t totalInstructions);
};
