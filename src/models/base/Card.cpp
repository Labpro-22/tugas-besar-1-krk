#include "models/base/Card.hpp"

Card::Card(const std::string& name, const std::string& description)
    : name(name), description(description) {}

const std::string& Card::getName() const {
    return name;
}

const std::string& Card::getDescription() const {
    return description;
}
