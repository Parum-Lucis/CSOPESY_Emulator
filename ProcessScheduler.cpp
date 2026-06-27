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
    schedulerThread = std::thread(&ProcessScheduler::schedulerLoop, this);
}

void ProcessScheduler::stop() {
    isRunning = false;
    isGeneratingDummy = false;

    if (generatorThread.joinable()) {
        generatorThread.join();
    }

    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }

    for (auto& cpu : cpuWorkers) {
        cpu->stop();
    }
    cpuWorkers.clear();
}

void ProcessScheduler::schedulerLoop() {
    while (isRunning) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);

            for (auto it = waitQueue.begin(); it != waitQueue.end(); ) {
                auto p = *it;

                p->decrementSleepTicks();

                if (p->getSleepTicks() <= 0) {
                    p->setState(ProcessState::READY);
                    readyQueue.push(p);
                    it = waitQueue.erase(it);
                } else {
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
    if (process->getState() == ProcessState::FINISHED) {
        return;
    }

    std::lock_guard<std::mutex> lock(queueMutex);

    if (process->getState() == ProcessState::WAITING) {
        waitQueue.push_back(process);
    } else {
        readyQueue.push(process);
    }
}

void ProcessScheduler::toggleDummyGeneration(bool state) {
    isGeneratingDummy = state;
}

static size_t generateRecursiveForLoop(const std::shared_ptr<Process>& process,
                                       int repeats,
                                       size_t currentInstructionCount,
                                       size_t maxInstructions,
                                       int currentDepth,
                                       std::mt19937& gen,
                                       const std::vector<std::string>& dummyVars) {

    if (repeats <= 0 || currentDepth > 3 || currentInstructionCount >= maxInstructions) {
        return 0;
    }

    size_t instructionsAddedThisLoop = 0;

    std::uniform_int_distribution<int> cmdDist(0, 5);
    std::uniform_int_distribution<size_t> varDist(0, dummyVars.size() - 1);
    std::uniform_int_distribution<int> valDist(0, 99);
    std::uniform_int_distribution<int> nestedLoopDist(2, 4);

    for (int i = 0; i < repeats; ++i) {
        if (currentInstructionCount + instructionsAddedThisLoop >= maxInstructions) {
            break;
        }

        int innerCmdType = cmdDist(gen);
        std::shared_ptr<ACommand> newCmd = nullptr;

        std::string randVar1 = dummyVars[varDist(gen)];
        std::string randVar2 = dummyVars[varDist(gen)];
        std::string randVal = std::to_string(valDist(gen));

        if (innerCmdType == 5 && currentDepth < 3) {
            int nestedRepeats = nestedLoopDist(gen);

            size_t nestedAdded = generateRecursiveForLoop(
                process, nestedRepeats,
                currentInstructionCount + instructionsAddedThisLoop,
                maxInstructions, currentDepth + 1, gen, dummyVars
            );

            instructionsAddedThisLoop += nestedAdded;

        } else {
            if (innerCmdType == 5) innerCmdType = 3;

            switch (innerCmdType) {
                case 0: newCmd = std::make_shared<DeclareCommand>(process->getLocalMemory(), randVar1, randVal); break;
                case 1: newCmd = std::make_shared<AddCommand>(process->getLocalMemory(), randVar1, randVar1, randVal); break;
                case 2: newCmd = std::make_shared<SubtractCommand>(process->getLocalMemory(), randVar1, randVar1, randVal); break;
                case 3: newCmd = std::make_shared<PrintCommand>(process->getLocalMemory(), randVar1); break;
                case 4: newCmd = std::make_shared<SleepCommand>(process, "10"); break;
            }

            if (newCmd != nullptr) {
                process->addCommand(newCmd);
                instructionsAddedThisLoop++;
            }
        }
    }

    return instructionsAddedThisLoop;
}

void ProcessScheduler::generateDummyProcess(const std::shared_ptr<Process>& newProcess, size_t totalInstructions) {
    std::vector<std::string> dummyVars = {"A", "B", "C", "X", "Y"};

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> cmdDist(0, 5);
    std::uniform_int_distribution<size_t> varDist(0, dummyVars.size() - 1);
    std::uniform_int_distribution<int> valDist(0, 99);
    std::uniform_int_distribution<int> loopDist(3, 8);

    size_t instructionsAdded = 0;

    while (instructionsAdded < totalInstructions) {
        int cmdType = cmdDist(gen);

        std::shared_ptr<ACommand> newCmd = nullptr;

        std::string randVar1 = dummyVars[varDist(gen)];
        std::string randVar2 = dummyVars[varDist(gen)];
        std::string randVal = std::to_string(valDist(gen));

        switch (cmdType) {
            case 0:
                newCmd = std::make_shared<DeclareCommand>(newProcess->getLocalMemory(), randVar1, randVal);
                instructionsAdded++;
                break;
            case 1:
                newCmd = std::make_shared<AddCommand>(newProcess->getLocalMemory(), randVar1, randVar1, randVal);
                instructionsAdded++;
                break;
            case 2:
                newCmd = std::make_shared<SubtractCommand>(newProcess->getLocalMemory(), randVar1, randVar1, randVal);
                instructionsAdded++;
                break;
            case 3:
                newCmd = std::make_shared<PrintCommand>(newProcess->getLocalMemory(), randVar1);
                instructionsAdded++;
                break;
            case 4:
                newCmd = std::make_shared<SleepCommand>(newProcess, "10");
                instructionsAdded++;
                break;
            case 5: {
                int repeats = loopDist(gen);

                size_t added = generateRecursiveForLoop(
                    newProcess, repeats, instructionsAdded, totalInstructions, 1, gen, dummyVars
                );

                instructionsAdded += added;
                break;
            }
        }

        if (newCmd != nullptr && cmdType != 5) {
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
