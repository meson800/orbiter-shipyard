#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <irrlicht.h>

class Log
{
public:
    enum LogLevel {ALL,DEBUG,INFO,WARN,ERROR,FATAL,OFF};
    static void clearLog();
    static void setLogLevel(LogLevel level);
    static void writeToLog(std::string &logMsg, LogLevel messageLevel);
    static void writeVectorToLog(const std::string& vectorName, irr::core::vector3df vec, LogLevel messageLevel);
private:
    static const char* levelStrings[];
    static bool shouldLog(LogLevel level);
    static LogLevel logLevel;
};