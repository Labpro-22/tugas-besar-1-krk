#pragma once
#include <string>
#include <vector>
#include "models/base/GameTypes.hpp"

class TransactionLogger {
private:
    std::vector<LogEntry> entries;

public:
    TransactionLogger() = default;

    void log(const LogEntry& entry);
    void log(int turnNumber,
             const std::string& username,
             const std::string& actionName,
             const std::string& detail);

    const std::vector<LogEntry>& getEntries() const;
    std::vector<std::string> toDisplayLines() const;
    void setEntries(const std::vector<LogEntry>& newEntries);
    void clear();
};
