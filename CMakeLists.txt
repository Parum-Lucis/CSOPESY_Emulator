cmake_minimum_required(VERSION 4.2)
project(CSOPESY_Emulator)

set(CMAKE_CXX_STANDARD 26)

add_executable(CSOPESY_Emulator quattro.cpp
        configManager.h
        configManager.cpp
        OSEmulator.h
        OSEmulator.cpp
        Process.cpp
        Process.h
        Command/ACommand.cpp
        Command/ACommand.h
        Command/PrintCommand.cpp
        Command/PrintCommand.h
        Command/DeclareCommand.cpp
        Command/DeclareCommand.h
        Command/AddCommand.cpp
        Command/AddCommand.h
        Command/SubtractCommand.cpp
        Command/SubtractCommand.h
        Command/SleepCommand.cpp
        Command/SleepCommand.h
        CPUWorker.cpp
        CPUWorker.h
        ProcessScheduler.cpp
        ProcessScheduler.h
        Console/MainMenuConsole.cpp
        Console/MainMenuConsole.h
        Console/SchedulingConsole.cpp
        Console/SchedulingConsole.h
)
