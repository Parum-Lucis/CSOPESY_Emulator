#pragma once
#include <string>
#include <cstdint>

class ConfigManager {
public:
    static ConfigManager* getInstance();

    void initialize();

    [[nodiscard]] uint32_t getNumCPU() const { return numCPU; }
    [[nodiscard]] std::string getScheduler() const { return scheduler; }
    [[nodiscard]] uint32_t getQuantumCycles() const { return quantumCycles; }
    [[nodiscard]] uint32_t getBatchProcessFreq() const { return batchProcessFreq; }
    [[nodiscard]] uint32_t getMinIns() const { return minIns; }
    [[nodiscard]] uint32_t getMaxIns() const { return maxIns; }
    [[nodiscard]] uint32_t getDelayPerExec() const { return delayPerExec; }

private:
    ConfigManager() = default;

    static ConfigManager* instance;

    uint32_t numCPU = 4;
    std::string scheduler = "rr";
    uint32_t quantumCycles = 5;
    uint32_t batchProcessFreq = 1;
    uint32_t minIns = 1000;
    uint32_t maxIns = 2000;
    uint32_t delayPerExec = 0;
};