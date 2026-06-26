#include "ProcessScheduler.h"
#include "ConfigManager.h"
#include "PrintCommand.h"
#include "AddCommand.h"
#include "SubtractCommand.h"
#include "DeclareCommand.h"
#include "SleepCommand.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>

ProcessScheduler* ProcessScheduler::getInstance() {
    static ProcessScheduler instance;
    return &instance;
}

void ProcessScheduler::start() {
    // ADDED: Safety guard to prevent crashing if start is called twice
    if (isRunning) {
        return;
    }

    isRunning = true;

    // FOR TESTING
    // Reset tracking for new runs
    {
        std::lock_guard<std::mutex> lock(statsMutex);
        firstProcess = nullptr;
        latestProcess = nullptr;
        allProcessList.clear(); // Clear master list for a fresh start
    }

    // ADDED: Only spawn CPU threads if they don't already exist!
    if (cpuWorkers.empty()) {
        uint32_t numCPUs = ConfigManager::getInstance()->getNumCPU();
        for (uint32_t i = 0; i < numCPUs; ++i) {
            auto cpu = std::make_shared<CPU>(i);
            cpu->start();
            cpuWorkers.push_back(cpu);
        }
    }

    generatorThread = std::thread(&ProcessScheduler::batchGeneratorLoop, this);
}

void ProcessScheduler::stop() {
    if (isRunning)
    {
        isRunning = false; // This will flag the batchGeneratorLoop to stop

        if (generatorThread.joinable()) {
            generatorThread.join();
        }

        // REMOVED: The loop that calls cpu->stop() and cpuWorkers.clear()
        // The CPU threads will continue running in the background to finish the readyQueue
    }
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

void ProcessScheduler::generateDummyProcess(const std::shared_ptr<Process>& newProcess, size_t totalInstructions) {
    std::vector<std::string> dummyVars = { "A", "B", "C", "X", "Y" };

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> cmdDist(0, 4);
    std::uniform_int_distribution<size_t> varDist(0, dummyVars.size() - 1);
    std::uniform_int_distribution<int> valDist(0, 99);

    for (size_t i = 0; i < totalInstructions; i++) {
        int cmdType = cmdDist(gen);

        std::shared_ptr<ACommand> newCmd = nullptr;

        std::string randVar1 = dummyVars[varDist(gen)];
        std::string randVar2 = dummyVars[varDist(gen)];
        std::string randVal = std::to_string(valDist(gen));

        switch (cmdType) {
        case 0:
            newCmd = std::make_shared<DeclareCommand>(
                newProcess->getLocalMemory(), randVar1, randVal
            );
            break;

        case 1:
            newCmd = std::make_shared<AddCommand>(
                newProcess->getLocalMemory(), randVar1, randVar1, randVal
            );
            break;

        case 2:
            newCmd = std::make_shared<SubtractCommand>(
                newProcess->getLocalMemory(), randVar1, randVar1, randVal
            );
            break;

        case 3:
            newCmd = std::make_shared<PrintCommand>(
                newProcess->getLocalMemory(), randVar1
            );
            break;

        case 4:
            newCmd = std::make_shared<SleepCommand>("10");
            break;
        }

        if (newCmd != nullptr) {
            newProcess->addCommand(newCmd);
        }
    }
}

void ProcessScheduler::batchGeneratorLoop() {
    uint32_t batchFreq = ConfigManager::getInstance()->getBatchProcessFreq();
    uint32_t minIns = ConfigManager::getInstance()->getMinIns();
    uint32_t maxIns = ConfigManager::getInstance()->getMaxIns();

    std::random_device rd;
    std::mt19937 gen(rd());

    uint32_t safeMin = minIns > 0 ? minIns : 100;
    uint32_t safeMax = maxIns >= safeMin ? maxIns : safeMin;

    std::uniform_int_distribution<size_t> distrib(safeMin, safeMax);

    while (isRunning) {
        size_t totalInstructions = distrib(gen);

        // Instantiate Process dynamically
        auto newProcess = std::make_shared<Process>(globalProcessCounter, "p" + std::to_string(globalProcessCounter), totalInstructions);
        
        generateDummyProcess(newProcess, totalInstructions);
        
        addProcess(newProcess);

        // Track stats dynamically FOR TESTING & reporting
        {
            std::lock_guard<std::mutex> lock(statsMutex);
            if (!firstProcess) {
                firstProcess = newProcess;
            }
            latestProcess = newProcess;

            // Push into the master list so 'screen -ls' can find it later
            allProcessList.push_back(newProcess);
        }

        globalProcessCounter++;

        // Wait representing CPU ticks before generating next batch
        std::this_thread::sleep_for(std::chrono::milliseconds(batchFreq * 10));
    }
}

void ProcessScheduler::generateReportUtil() {
    // Left unimplemented intentionally as the logic was ported directly 
    // into MainMenuConsole.cpp for shared formatting across screen -ls and report-util.
}

// --- GETTERS FOR TESTING AND UI REPORTING ---

size_t ProcessScheduler::getReadyQueueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return readyQueue.size();
}

std::shared_ptr<Process> ProcessScheduler::getFirstProcess() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return firstProcess;
}

std::shared_ptr<Process> ProcessScheduler::getLatestProcess() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return latestProcess;
}

std::vector<std::shared_ptr<Process>> ProcessScheduler::getAllProcesses() const {
    std::lock_guard<std::mutex> lock(statsMutex);
    return allProcessList;
}

size_t ProcessScheduler::getCoresUsed() const {
    size_t count = 0;

    // CPU workers are static/constant while running, but their currentProcess changes.
    // Iterating over the array is safe. 
    for (const auto& cpu : cpuWorkers) {
        if (cpu->getCurrentProcess() != nullptr) {
            count++;
        }
    }

    return count;
}