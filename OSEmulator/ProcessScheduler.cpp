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

void ProcessScheduler::initializeSystem() {
    if (systemRunning) return;
    systemRunning = true;

    // Spawn CPU threads once on startup
    if (cpuWorkers.empty()) {
        uint32_t numCPUs = ConfigManager::getInstance()->getNumCPU();
        for (uint32_t i = 0; i < numCPUs; ++i) {
            auto cpu = std::make_shared<CPU>(i);
            cpu->start();
            cpuWorkers.push_back(cpu);
        }
    }

    // Start the waitQueue manager thread immediately
    schedulerThread = std::thread([this]() {
        while (systemRunning) {

            // 1. Create an explicit scope block just for the lock
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                for (auto it = waitQueue.begin(); it != waitQueue.end(); ) {
                    auto p = *it;
                    p->decrementSleepTicks();
                    if (p->getSleepTicks() <= 0) {
                        p->setState(ProcessState::READY);
                        readyQueue.push(p);
                        it = waitQueue.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            } // 2. Lock goes out of scope and is safely released HERE!

            // 3. Sleep freely without blocking the CPU workers
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 1 tick = 100ms
        }
    });
}

void ProcessScheduler::shutdownSystem() {
    systemRunning = false;
    stop(); // Ensure generator stops too

    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }
    for (auto& cpu : cpuWorkers) {
        cpu->stop();
    }
    cpuWorkers.clear();
}

void ProcessScheduler::moveToWaitQueue(const std::shared_ptr<Process>& process) {
    std::lock_guard<std::mutex> lock(queueMutex);
    waitQueue.push_back(process);
}

// UPDATE start() and stop() to ONLY manage isRunning and the generatorThread
void ProcessScheduler::start() {
    if (isRunning) return;
    isRunning = true;

    {
        std::lock_guard<std::mutex> lock(statsMutex);
        firstProcess = nullptr;
        latestProcess = nullptr;
        allProcessList.clear();
    }
    generatorThread = std::thread(&ProcessScheduler::batchGeneratorLoop, this);
}

void ProcessScheduler::stop() {
    if (isRunning) {
        isRunning = false;
        if (generatorThread.joinable()) {
            generatorThread.join();
        }
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
    }
    else {
        readyQueue.push(process);
    }
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

    // CHANGED: Use a weighted discrete distribution instead of a uniform one.
    // Weights: Declare (19%), Add (29%), Subtract (29%), Print (17%), Sleep (1%), For Loop (5%)
    std::discrete_distribution<int> cmdDist({ 19, 29, 29, 17, 1, 5 });
    std::uniform_int_distribution<size_t> varDist(0, dummyVars.size() - 1);
    std::uniform_int_distribution<int> valDist(0, 99);
    std::uniform_int_distribution<int> nestedLoopDist(2, 4);

    // 1. Decide how many lines of code are inside this loop block (e.g., 1 to 3 lines)
    std::uniform_int_distribution<int> linesDist(1, 3);
    int linesInLoop = linesDist(gen);

    // 2. Save the starting state of the random number generator
    std::mt19937 savedGenState = gen;
    std::mt19937 finalGenState;

    // The Outer Repetition Loop
    for (int i = 0; i < repeats; ++i) {
        if (currentInstructionCount + instructionsAddedThisLoop >= maxInstructions) {
            break;
        }

        // 3. REWIND: Reset the dice to the saved state at the start of every repetition.
        // Because the RNG is reset, it will roll the exact same commands and variables!
        gen = savedGenState;

        // The Inner Loop Body Generation
        for (int line = 0; line < linesInLoop; ++line) {
            if (currentInstructionCount + instructionsAddedThisLoop >= maxInstructions) break;

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
            }
            else {
                if (innerCmdType == 5) innerCmdType = 3; // Fallback to print

                switch (innerCmdType) {
                case 0: newCmd = std::make_shared<DeclareCommand>(process->getLocalMemory(), randVar1, randVal); break;
                case 1: newCmd = std::make_shared<AddCommand>(process->getLocalMemory(), randVar1, randVar1, randVal); break;
                case 2: newCmd = std::make_shared<SubtractCommand>(process->getLocalMemory(), randVar1, randVar1, randVal); break;
                case 3: newCmd = std::make_shared<PrintCommand>(process->getLocalMemory(), "Hello world from " + process->getName()); break;
                case 4: newCmd = std::make_shared<SleepCommand>(process, "10"); break;
                }

                if (newCmd != nullptr) {
                    process->addCommand(newCmd);
                    instructionsAddedThisLoop++;
                }
            }
        }

        // 4. Save the RNG state after the FIRST full iteration of the loop body
        if (i == 0) {
            finalGenState = gen;
        }
    }

    // 5. FAST-FORWARD: Advance the RNG so subsequent outer code doesn't replicate the loop's rolls
    gen = finalGenState;

    return instructionsAddedThisLoop;
}

void ProcessScheduler::generateDummyProcess(const std::shared_ptr<Process>& newProcess, size_t totalInstructions) {
    std::vector<std::string> dummyVars = { "A", "B", "C", "X", "Y" };

    std::random_device rd;
    std::mt19937 gen(rd());

    // CHANGED: Use a weighted discrete distribution instead of a uniform one.
	// Weights: Declare (19%), Add (29%), Subtract (29%), Print (17%), Sleep (1%), For Loop (5%)
    std::discrete_distribution<int> cmdDist({ 19, 29, 29, 17, 1, 5 });

    std::uniform_int_distribution<size_t> varDist(0, dummyVars.size() - 1);
    std::uniform_int_distribution<int> valDist(0, 99);

    // Distribution for sleep duration between 10 and 50 ticks to keep them snappy
    std::uniform_int_distribution<int> sleepDist(10, 50);

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
            newCmd = std::make_shared<DeclareCommand>(
                newProcess->getLocalMemory(), randVar1, randVal
            );
            instructionsAdded++;
            break;

        case 1:
            newCmd = std::make_shared<AddCommand>(
                newProcess->getLocalMemory(), randVar1, randVar1, randVal
            );
            instructionsAdded++;
            break;

        case 2:
            newCmd = std::make_shared<SubtractCommand>(
                newProcess->getLocalMemory(), randVar1, randVar1, randVal
            );
            instructionsAdded++;
            break;

        case 3:
            newCmd = std::make_shared<PrintCommand>(
                newProcess->getLocalMemory(), "Hello world from " + newProcess->getName()
            );
            instructionsAdded++;
            break;

        case 4:
            newCmd = std::make_shared<SleepCommand>(newProcess, std::to_string(sleepDist(gen)));
            instructionsAdded++;
            break;
        case 5:
            int repeats = loopDist(gen);

            size_t added = generateRecursiveForLoop(
                newProcess, repeats, instructionsAdded, totalInstructions, 1, gen, dummyVars
            );

            instructionsAdded += added;
            break;
        }


        if (newCmd != nullptr && cmdType != 5) {
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

size_t ProcessScheduler::getWaitQueueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return waitQueue.size();
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

bool ProcessScheduler::createManualProcess(const std::string& processName) {
    std::lock_guard<std::mutex> lock(statsMutex);

    // 1. Ensure the process name is unique
    for (const auto& proc : allProcessList) {
        if (proc && proc->getName() == processName) {
            return false;
        }
    }

    // 2. Ensure CPU cores are ACTUALLY spawned and running to process this manual request!
    // (This fixes the issue if the user hasn't run the global start() command yet)
    if (cpuWorkers.empty()) {
        uint32_t numCPUs = ConfigManager::getInstance()->getNumCPU();
        for (uint32_t i = 0; i < numCPUs; ++i) {
            auto cpu = std::make_shared<CPU>(i);
            cpu->start();
            cpuWorkers.push_back(cpu);
        }
    }

    // 3. Determine random instruction counts safely matching config limits
    uint32_t minIns = ConfigManager::getInstance()->getMinIns();
    uint32_t maxIns = ConfigManager::getInstance()->getMaxIns();
    uint32_t safeMin = minIns > 0 ? minIns : 100;
    uint32_t safeMax = maxIns >= safeMin ? maxIns : safeMin;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> distrib(safeMin, safeMax);
    size_t totalInstructions = distrib(gen);

    // 4. Instantiate the process shell
    auto newProcess = std::make_shared<Process>(globalProcessCounter, processName, totalInstructions);

    // 5. Generate commands and track stats
    generateDummyProcess(newProcess, totalInstructions);

    if (!firstProcess) {
        firstProcess = newProcess;
    }
    latestProcess = newProcess;
    allProcessList.push_back(newProcess);

    globalProcessCounter++;

    // 6. Push to ready queue—now a live CPU core thread will instantly grab it!
    addProcess(newProcess);

    return true;
}