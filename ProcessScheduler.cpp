#include "ProcessScheduler.h"
#include "ConfigManager.h"
#include "PrintCommand.h"
#include "AddCommand.h"
#include "SubtractCommand.h"
#include "DeclareCommand.h"
#include "SleepCommand.h"
#include <vector>
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

    generatorThread = std::thread(&ProcessScheduler::dummyGenerationLoop, this);
}

void ProcessScheduler::stop() {
    isRunning = false;
    isGeneratingDummy = false;

    if (generatorThread.joinable()) {
        generatorThread.join();
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

void ProcessScheduler::toggleDummyGeneration(bool state) {
    isGeneratingDummy = state;
}

#include <random> // Ensure this is at the top of your file!

static void ProcessScheduler::generateDummyProcess(const std::shared_ptr<Process>& newProcess, size_t totalInstructions) {
    std::vector<std::string> dummyVars = {"A", "B", "C", "X", "Y"};

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
                    newProcess->getMemoryMap(), randVar1, randVal
                );
                break;

            case 1:
                newCmd = std::make_shared<AddCommand>(
                    newProcess->getMemoryMap(), randVar1, randVar1, randVal
                );
                break;

            case 2:
                newCmd = std::make_shared<SubtractCommand>(
                    newProcess->getMemoryMap(), randVar1, randVar1, randVal
                );
                break;

            case 3:
                newCmd = std::make_shared<PrintCommand>(
                    newProcess->getMemoryMap(), randVar1
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

void ProcessScheduler::dummyGenerationLoop() {
    size_t processCounter = 1;
    std::random_device rd;
    std::mt19937 gen(rd());

    while (isRunning) {
        if (isGeneratingDummy) {
            uint32_t batchFreq = ConfigManager::getInstance()->getBatchProcessFreq();
            uint32_t minIns = ConfigManager::getInstance()->getMinIns();
            uint32_t maxIns = ConfigManager::getInstance()->getMaxIns();

            uint32_t safeMin = minIns > 0 ? minIns : 100;
            uint32_t safeMax = maxIns >= safeMin ? maxIns : safeMin;

            std::uniform_int_distribution<size_t> distrib(safeMin, safeMax);
            size_t totalInstructions = distrib(gen);

            std::string processName = "p" + std::to_string(processCounter);

            auto newProcess = std::make_shared<Process>(processCounter, processName, totalInstructions);

            generateDummyProcess(newProcess, totalInstructions);

            addProcess(newProcess);

            processCounter++;

            std::this_thread::sleep_for(std::chrono::milliseconds(batchFreq * 10));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
