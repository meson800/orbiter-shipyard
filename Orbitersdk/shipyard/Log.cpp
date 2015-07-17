#include "Log.h"

Log::LogLevel Log::logLevel = Log::LogLevel::WARN;
const char *  Log::levelStrings[] = { "[ALL]", "[DEBUG]", "[INFO]", "[WARN]", "[ERROR]", "[FATAL]", "[OFF]" };

void Log::clearLog()
{
    std::ofstream logFile = std::ofstream("./StackEditor/StackEditor.log", std::ios::out);
    logFile.close();
}

void Log::setLogLevel(LogLevel level)
{
    Log::writeToLog("Changing log level to ", levelStrings[level]);
    logLevel = level;
}

bool Log::shouldLog(LogLevel level)
{
    return level >= logLevel;
}

void Log::writeToLog()
{
    std::ofstream logFile = std::ofstream("./StackEditor/StackEditor.log", std::ios::app);
    logFile << "\n";
    logFile.close();
}

void Log::writeVectorToLog(const std::string& vecName, irr::core::vector3df vec, LogLevel messageLevel)
{
    if (shouldLog(messageLevel))
    {
        std::ofstream logFile = std::ofstream("./StackEditor/StackEditor.log", std::ios::app);
        logFile << levelStrings[messageLevel] << vecName << "-X: " << vec.X << " Y: " << vec.Y << " Z: " << vec.Z << "\n";
        logFile.close();
    }
}