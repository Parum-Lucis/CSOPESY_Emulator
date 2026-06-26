#include "ConsoleManager.h"
#include "MainMenuConsole.h" // 1. Include the new Main Menu header
#include <thread>
#include <memory>

int main() {
    //  Get the global Singleton instance pointer
    auto* manager = ConsoleManager::getInstance();

    //  Use the arrow operator (->) to access its methods
    manager->registerConsole("main-menu", []() { return std::make_shared<MainMenuConsole>(); });
    manager->switchConsole("main-menu");

    while (manager->isRunning()) {
        manager->processInput();
        manager->drawConsole();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    return 0;
}