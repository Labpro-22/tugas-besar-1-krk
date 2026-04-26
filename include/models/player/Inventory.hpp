#pragma once
#include <cstddef>
#include <vector>

class SkillCard;

class Inventory {
private:
    std::vector<SkillCard*> skillCards;
    static const std::size_t MAX_SKILL_CARDS = 3;

public:
    Inventory() = default;
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;
    ~Inventory();

    bool isSkillCardFull() const;
    std::size_t getSkillCardCount() const;

    void addSkillCard(SkillCard* card);
    SkillCard* takeSkillCard(std::size_t index);
    void removeSkillCard(std::size_t index);

    const std::vector<SkillCard*>& getSkillCards() const;
    void clear();
};
