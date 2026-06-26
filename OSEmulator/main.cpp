#include "ConsoleManager.h"
#include "MainMenuConsole.h" // 1. Include the new Main Menu header
#include <thread>
#include <memory>

int main() {
    ConsoleManager manager;

    // 2. Register your console classes with the manager
    manager.registerConsole("main-menu", []() { return std::make_shared<MainMenuConsole>(); });
    //manager.registerConsole("emulator", []() { return std::make_shared<EmulatorConsole>(); });
    //manager.registerConsole("dummy", []() { return std::make_shared<DummyConsole>(); });

    // 3. Set the Main Menu as the boot/entry console
    manager.switchConsole("main-menu");

    // Core emulator engine loop running at ~30 FPS
    while (manager.isRunning()) {
        manager.processInput();
        manager.drawConsole();
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    return 0;
}