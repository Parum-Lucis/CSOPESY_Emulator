#include "ProcessScheduler.h"
#include "ConfigManager.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>

ProcessScheduler* ProcessScheduler::getInstance() {
    static ProcessScheduler instance;
    return &instance;
}

void ProcessScheduler::start() {
    isRunning = true;

    uint32_t numCPUs = ConfigManager::getInstance()->getNumCPU();
    for (uint32_t i = 0; i < numCPUs; ++i) {
        auto cpu = std::make_shared<CPU>(i);
        cpu->start();
        cpuWorkers.push_back(cpu);
    }

    generatorThread = std::thread(&ProcessScheduler::batchGeneratorLoop, this);
}

void ProcessScheduler::stop() {
    isRunning = false;

    if (generatorThread.joinable()) {
        generatorThread.detach();
    }

    for (auto& cpu : cpuWorkers) {
        cpu->stop();
    }
    cpuWorkers.clear();
}

void ProcessScheduler::addProcess(const std::shared_ptr<Process>& process) {
    std::lock_guard<std::mutex> lock(queueMutex);
    readyQueue.push(process);
}

std::shared_ptr<Process> ProcessScheduler::fetchNextProcess() {
    std::lock_guard<std::mutex> lock(queueMutex);

    if (readyQueue.empty()) {
        return nullptr;
    }

    auto process = readyQueue.front();
    readyQueue.pop();
    return process;
}

void ProcessScheduler::requeueProcess(const std::shared_ptr<Process>& process) {
    if (process->getState() != ProcessState::FINISHED) {
        addProcess(process);
    }
}

void ProcessScheduler::batchGeneratorLoop() {
    uint32_t batchFreq = ConfigManager::getInstance()->getBatchProcessFreq();
    uint32_t minIns = ConfigManager::getInstance()->getMinIns();
    uint32_t maxIns = ConfigManager::getInstance()->getMaxIns();

    size_t processCounter = 1;

    std::random_device rd;
    std::mt19937 gen(rd());

    uint32_t safeMin = minIns > 0 ? minIns : 100;
    uint32_t safeMax = maxIns >= safeMin ? maxIns : safeMin;

    std::uniform_int_distribution<size_t> distrib(safeMin, safeMax);

    while (isRunning) {
        size_t totalInstructions = distrib(gen);

        std::string processName = "p" + std::to_string(processCounter);

        auto newProcess = std::make_shared<Process>(processCounter, processName, totalInstructions);

        addProcess(newProcess);

        processCounter++;

        std::this_thread::sleep_for(std::chrono::milliseconds(batchFreq * 10));
    }
}

// TODO CPU REPORT
}
