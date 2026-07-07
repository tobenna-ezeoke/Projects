#include "Globals.h"

std::vector<DebugLogEntry> debugLogs;
std::mutex logMutex;

void addLog(const std::string& logEntry, int colorPair) {
    std::lock_guard<std::mutex> lock(logMutex);
    DebugLogEntry entry { logEntry, colorPair };
    debugLogs.push_back(entry);
    if (debugLogs.size() > 100)
        debugLogs.erase(debugLogs.begin());
}