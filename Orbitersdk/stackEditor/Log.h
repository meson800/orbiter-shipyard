//Copyright (c) 2015 Christopher Johnstone(meson800) and Benedict Haefeli(jedidia)
//The MIT License - See ../../LICENSE for more info
#pragma once

#include <mutex>
#include <string>
#include <iostream>
#include <fstream>
#include <irrlicht.h>
#include "Helpers.h"

#undef DEBUG

class Log
{
public:
    //L_DEBUG because windows global defines DEBUG :(
    enum LogLevel {ALL,L_DEBUG,INFO,WARN,ERR,FATAL,OFF};
    static void clearLog();
    static void setLogLevel(LogLevel level);

    //thread safe definition, to avoid recursive mutex
    template<typename ... Types>
    static void writeToLog(Types ... rest)
    {
        writeMutex.lock();
        writeToLogThreadUnsafe(rest...);
        writeMutex.unlock();
    }

    //varadic write to log so its easier to use
    template<typename ... Types>
    //case that returns if messageLevel isn't enough to log
    static void writeToLogThreadUnsafe(LogLevel messageLevel, Types ... rest)
    {
        if (shouldLog(messageLevel))
        {
            std::ofstream logFile = std::ofstream(Helpers::workingDirectory + "StackEditor/StackEditor.log", std::ios::app);
            logFile << levelStrings[messageLevel];
            logFile.close();
            writeToLogThreadUnsafe(rest...);
        }
    }

    template<typename T, typename ... Types>
    //Generic case, prints out using << operator
    static void writeToLogThreadUnsafe(T first, Types ... rest)
    {
        std::ofstream logFile = std::ofstream(Helpers::workingDirectory + "StackEditor/StackEditor.log", std::ios::app);
        logFile << first;
        logFile.close();
        //recurse for other arguments
        writeToLogThreadUnsafe(rest...);
    }
    //base case, write out newline
    static void writeToLogThreadUnsafe();

    static void writeVectorToLog(const std::string& vectorName, irr::core::vector3df vec, LogLevel messageLevel);
private:
    static std::mutex writeMutex;
    static const char* levelStrings[];
    static bool shouldLog(LogLevel level);
    static LogLevel logLevel;
};
