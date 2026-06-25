cmake_minimum_required(VERSION 4.2)
project(CSOPESY_Emulator)

set(CMAKE_CXX_STANDARD 26)

add_executable(CSOPESY_Emulator quattro.cpp
        configManager.h
        configManager.cpp
        OSEmulator.h
        OSEmulator.cpp
        Console/ConsoleManager.cpp
        Console/ConsoleManager.h
        Console/AConsole.cpp
        Console/AConsole.h
        Console/MainMenuConsole.cpp
        Console/MainMenuConsole.h
        Console/SchedulingConsole.cpp
        Console/SchedulingConsole.h
        Console/MarqueeConsole.cpp
        Console/MarqueeConsole.h
        Console/MemorySimulationConsole.cpp
        Console/MemorySimulationConsole.h
        Process.cpp
        Process.h
        Instruction/AInstruction.cpp
        Instruction/AInstruction.h
        Instruction/PrintCommand.cpp
        Instruction/PrintCommand.h
        Instruction/DeclareCommand.cpp
        Instruction/DeclareCommand.h
        Instruction/AddCommand.cpp
        Instruction/AddCommand.h
        Instruction/SubtractCommand.cpp
        Instruction/SubtractCommand.h
        Instruction/SleepCommand.cpp
        Instruction/SleepCommand.h
        CPUWorker.cpp
        CPUWorker.h
        ProcessScheduler.cpp
        ProcessScheduler.h
)
