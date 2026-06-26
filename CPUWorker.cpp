#include "CPUWorker.h"
#include "ProcessScheduler.h"
#include "ConfigManager.h"
#include <iostream>

CPU::CPU(int id) : coreID(id), currentProcess(nullptr), isRunning(false) {}

CPU::~CPU() {
    stop();
}

void CPU::start() {
    isRunning = true;
    workerThread = std::thread(&CPU::runWorkerLoop, this);
}

void CPU::stop() {
    isRunning = false;
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void CPU::runWorkerLoop() {
    std::string schedulerAlgo = ConfigManager::getInstance()->getSchedulerAlgorithm();
    uint32_t timeQuantum = ConfigManager::getInstance()->getTimeQuantum();

    while (isRunning) {
        currentProcess = ProcessScheduler::getInstance()->fetchNextProcess();

        if (currentProcess != nullptr) {

            currentProcess->setState(ProcessState::RUNNING);

            if (schedulerAlgo == "fcfs") {
                while (currentProcess->getState() != ProcessState::FINISHED) {
                    currentProcess->executeNextInstruction();
                    applyDelay();
                }
                currentProcess = nullptr;
            }

            else if (schedulerAlgo == "rr") {
                for (uint32_t i = 0; i < timeQuantum; ++i) {
                    if (currentProcess->getState() == ProcessState::FINISHED) {
                        break; // Drop immediately if finished before quantum ends
                    }
                    currentProcess->executeNextInstruction();
                    applyDelay();
                }

                if (currentProcess->getState() == ProcessState::FINISHED) {
                    currentProcess = nullptr;
                } else {
                    currentProcess->setState(ProcessState::READY);

                    ProcessScheduler::getInstance()->requeueProcess(currentProcess);
                    currentProcess = nullptr;
                }
            }

        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

void CPU::applyDelay() {
    uint32_t delayCycles = ConfigManager::getInstance()->getDelayPerExec();

    if (delayCycles > 0) {
        volatile uint32_t dummyCounter = 0;
        for (uint32_t i = 0; i < delayCycles * 1000; ++i) {
            dummyCounter++;
        }
    }
}