#include "models/player/Inventory.hpp"
#include <stdexcept>
#include "models/cards/SkillCard.hpp"

Inventory::~Inventory() {
    clear();
}

bool Inventory::isSkillCardFull() const {
    return skillCards.size() >= MAX_SKILL_CARDS;
}

std::size_t Inventory::getSkillCardCount() const {
    return skillCards.size();
}

void Inventory::addSkillCard(SkillCard* card) {
    if (card == 0) {
        return;
    }
    if (isSkillCardFull()) {
        throw std::runtime_error("Skill card inventory is full.");
    }
    skillCards.push_back(card);
}

SkillCard* Inventory::takeSkillCard(std::size_t index) {
    if (index >= skillCards.size()) {
        throw std::out_of_range("Invalid skill card index.");
    }
    SkillCard* card = skillCards[index];
    skillCards.erase(skillCards.begin() + static_cast<long>(index));
    return card;
}

void Inventory::removeSkillCard(std::size_t index) {
    SkillCard* card = takeSkillCard(index);
    delete card;
}

const std::vector<SkillCard*>& Inventory::getSkillCards() const {
    return skillCards;
}

void Inventory::clear() {
    for (std::size_t i = 0; i < skillCards.size(); ++i) {
        delete skillCards[i];
    }
    skillCards.clear();
}
