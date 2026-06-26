#include "ConsoleManager.h"
#include "MainMenuConsole.h"
#include <thread>
#include <memory>
#include <chrono>

int main() {
    // 1. Get the global Singleton instance pointer
    auto* manager = ConsoleManager::getInstance();

    // 2. Register via a factory lambda (matches ConsoleFactory definition)
    manager->registerConsole("main-menu", []() {
        return std::make_shared<MainMenuConsole>();
        });
    manager->switchConsole("main-menu");

    // 3. Core engine refresh loop running at ~30 FPS
    while (manager->isRunning()) {
        manager->processInput();
        manager->drawConsole();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    return 0;
}