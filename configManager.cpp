#include "ConfigManager.h"
#include <iostream>
#include <fstream>
#include <sstream>

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
        std::cerr << "Warning: config.txt not found. Generating a default config.txt file...\n";

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
            std::cout << "Default config.txt successfully created in the current directory.\n";
        } else {
            std::cerr << "Critical Error: Could not generate config.txt. Please check directory permissions.\n";
        }

        return;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;

        if (iss >> key) {
            if (key == "num-cpu") iss >> numCPU;
            else if (key == "scheduler") {
                std::string value;
                iss >> value;
                if (!value.empty() && value.front() == '"') value.erase(0, 1);
                if (!value.empty() && value.back() == '"') value.pop_back();
                scheduler = value;
            }
            else if (key == "quantum-cycles") iss >> quantumCycles;
            else if (key == "batch-process-freq") iss >> batchProcessFreq;
            else if (key == "min-ins") iss >> minIns;
            else if (key == "max-ins") iss >> maxIns;
            else if (key == "delay-per-exec") iss >> delayPerExec;
        }
    }

    configFile.close();
}
