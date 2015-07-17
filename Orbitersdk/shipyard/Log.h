#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <irrlicht.h>

#undef DEBUG

class Log
{
public:
    //L_DEBUG because windows global defines DEBUG :(
    enum LogLevel {ALL,L_DEBUG,INFO,WARN,ERR,FATAL,OFF};
    static void clearLog();
    static void setLogLevel(LogLevel level);

    //varadic write to log so its easier to use
    template<typename ... Types>
    //case that returns if messageLevel isn't enough to log
    static void writeToLog(LogLevel messageLevel, Types ... rest)
    {
        if (shouldLog(messageLevel))
        {
            std::ofstream logFile = std::ofstream("./StackEditor/StackEditor.log", std::ios::app);
            logFile << levelStrings[messageLevel];
            logFile.close();
            writeToLog(rest...);
        }
    }

    template<typename T, typename ... Types>
    //Generic case, prints out using << operator
    static void writeToLog(T first, Types ... rest)
    {
        std::ofstream logFile = std::ofstream("./StackEditor/StackEditor.log", std::ios::app);
        logFile << first;
        logFile.close();
        //recurse for other arguments
        writeToLog(rest...);
    }
    //base case, write out newline
    static void writeToLog();

    static void writeVectorToLog(const std::string& vectorName, irr::core::vector3df vec, LogLevel messageLevel);
private:
    static const char* levelStrings[];
    static bool shouldLog(LogLevel level);
    static LogLevel logLevel;
};