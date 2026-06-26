#include "ConfigManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

ConfigManager* ConfigManager::instance = nullptr;

ConfigManager* ConfigManager::getInstance() {
    if (instance == nullptr) {
        instance = new ConfigManager();
    }
    return instance;
}

void ConfigManager::initialize() {
    std::ifstream configFile("config.txt");

    if (!configFile.is_open()) {
        std::cerr << "Warning: config.txt not found. Generating a default configuration...\n";

        std::ofstream newConfigFile("config.txt");
        if (newConfigFile.is_open()) {
            newConfigFile << "num-cpu " << numCPU << "\n";
            newConfigFile << "scheduler \"" << scheduler << "\"\n";
            newConfigFile << "quantum-cycles " << quantumCycles << "\n";
            newConfigFile << "batch-process-freq " << batchProcessFreq << "\n";
            newConfigFile << "min-ins " << minIns << "\n";
            newConfigFile << "max-ins " << maxIns << "\n";
            newConfigFile << "delay-per-exec " << delayPerExec << "\n";
            newConfigFile.close();
            std::cout << "Default config.txt successfully initialized\n";
        } else {
            std::cerr << "Error: Could not generate config.txt. Check directory permissions.\n";
        }

        return;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);

        if (std::string key; iss >> key) {
            if (key == "num-cpu") {
                uint32_t val; iss >> val;
                numCPU = std::clamp(val, 1u, 128u);
            }
            else if (key == "scheduler") {
                std::string value;
                iss >> value;
                // Remove quotes
                value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
                // Validate algorithm
                if (value == "fcfs" || value == "rr") {
                    scheduler = value;
                } else {
                    scheduler = "fcfs";
                }
            }
            else if (key == "quantum-cycles") {
                uint32_t val; iss >> val;
                quantumCycles = std::max(1u, val);
            }
            else if (key == "batch-process-freq") {
                uint32_t val; iss >> val;
                batchProcessFreq = std::max(1u, val);
            }
            else if (key == "min-ins") {
                uint32_t val; iss >> val;
                minIns = std::max(1u, val);
            }
            else if (key == "max-ins") {
                uint32_t val; iss >> val;
                maxIns = std::max(minIns, val);
            }
            else if (key == "delay-per-exec") {
                uint32_t val; iss >> val;
                delayPerExec = val;
            }
        }
    }
    configFile.close();
}
