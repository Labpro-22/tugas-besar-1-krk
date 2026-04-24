#pragma once
#include "utils/io/DisplayHandler.hpp"

class CLIHandler : public DisplayHandler {
public:
    CLIHandler() = default;

    void printMessage(const std::string& message) override;
    std::string getInput(const std::string& prompt) override;
};
