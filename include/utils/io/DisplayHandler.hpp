#pragma once
#include <string>

class DisplayHandler {
public:
    virtual ~DisplayHandler() = default;

    virtual void printMessage(const std::string& message) = 0;
    virtual std::string getInput(const std::string& prompt) = 0;
};
