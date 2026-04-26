#include "utils/data/TransactionLogger.hpp"
#include <sstream>

void TransactionLogger::log(const LogEntry& entry) {
    entries.push_back(entry);
}

void TransactionLogger::log(int turnNumber,
                            const std::string& username,
                            const std::string& actionName,
                            const std::string& detail) {
    entries.push_back(LogEntry(turnNumber, username, actionName, detail));
}

const std::vector<LogEntry>& TransactionLogger::getEntries() const {
    return entries;
}

std::vector<std::string> TransactionLogger::toDisplayLines() const {
    std::vector<std::string> lines;
    for (std::size_t i = 0; i < entries.size(); ++i) {
        std::ostringstream oss;
        oss << "[Turn " << entries[i].turnNumber << "] "
            << entries[i].username << " | "
            << entries[i].actionName << " | "
            << entries[i].detail;
        lines.push_back(oss.str());
    }
    return lines;
}

void TransactionLogger::setEntries(const std::vector<LogEntry>& newEntries) {
    entries = newEntries;
}

void TransactionLogger::clear() {
    entries.clear();
}
