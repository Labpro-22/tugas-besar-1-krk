#pragma once

#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

#include "models/cards/SkillCard.hpp"
#include "models/base/Property.hpp"
#include "models/player/Player.hpp"
#include "models/tiles/Street.hpp"

namespace GameManagerInternal
{

    inline std::string toUpper(std::string value)
    {
        for (std::size_t i = 0; i < value.size(); ++i)
        {
            value[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[i])));
        }
        return value;
    }

    inline std::string propertyStatusToString(PropertyStatus status)
    {
        switch (status)
        {
        case PropertyStatus::BANK:
            return "BANK";
        case PropertyStatus::OWNED:
            return "OWNED";
        case PropertyStatus::MORTGAGED:
            return "MORTGAGED";
        }
        return "BANK"; // Jaga2
    }

    inline std::string prettyName(const std::string &value)
    {
        std::string result = value;
        for (std::size_t i = 0; i < result.size(); ++i)
        {
            if (result[i] == '_')
            {
                result[i] = ' ';
            }
        }
        return result;
    }

    inline std::string upperPrettyName(const std::string &value)
    {
        return toUpper(prettyName(value));
    }

    inline std::string formatMoney(int amount)
    {
        bool negative = amount < 0;
        unsigned int value = static_cast<unsigned int>(negative ? -amount : amount);

        std::string digits = std::to_string(value);
        std::string grouped;
        for (std::size_t i = 0; i < digits.size(); ++i)
        {
            grouped += digits[i];
            std::size_t remaining = digits.size() - i - 1;
            if (remaining > 0 && remaining % 3 == 0)
            {
                grouped += '.';
            }
        }

        return std::string(negative ? "-M" : "M") + grouped;
    }

    inline std::string colorGroupDisplay(const std::string &colorGroup)
    {
        return upperPrettyName(colorGroup);
    }

    inline std::string propertyGroupLabel(const Property &property)
    {
        if (property.getPropertyType() == PropertyType::RAILROAD)
        {
            return "STASIUN";
        }
        if (property.getPropertyType() == PropertyType::UTILITY)
        {
            return "UTILITAS";
        }

        const Street *street = dynamic_cast<const Street *>(&property);
        return street != 0 ? colorGroupDisplay(street->getColorGroup()) : "PROPERTI";
    }

    inline std::string buildingLevelLabel(int level)
    {
        if (level <= 0)
        {
            return "Tanah kosong";
        }
        if (level >= 5)
        {
            return "Hotel";
        }
        return std::to_string(level) + " rumah";
    }

    inline std::string festivalStatusLabel(const Property &property)
    {
        if (property.getFestivalTurnsRemaining() <= 0 || property.getFestivalMultiplier() <= 1)
        {
            return "";
        }

        std::ostringstream oss;
        oss << "Festival aktif x" << property.getFestivalMultiplier()
            << " (" << property.getFestivalTurnsRemaining() << " giliran)";
        return oss.str();
    }

    inline std::string propertyStatusLabel(const Property &property)
    {
        if (property.getStatus() == PropertyStatus::BANK)
        {
            return "BANK";
        }

        std::ostringstream oss;
        oss << propertyStatusToString(property.getStatus());
        if (property.getStatus() == PropertyStatus::MORTGAGED)
        {
            oss << " [M]";
        }
        if (property.getOwner() != 0)
        {
            oss << " (" << property.getOwner()->getName() << ")";
        }
        return oss.str();
    }

    inline SkillCard *cloneSkillCardByName(const std::string &name)
    {
        if (name == "MoveCard")
        {
            return new SkillCard("MoveCard", "Move forward.", "MoveCard", (std::rand() % 12) + 1, 0, true);
        }
        if (name == "DiscountCard")
        {
            return new SkillCard("DiscountCard", "Discount for one turn.", "DiscountCard", 10 + (std::rand() % 41), 1, true);
        }
        if (name == "ShieldCard")
        {
            return new SkillCard("ShieldCard", "Ignore penalties for one turn.", "ShieldCard", 0, 1, true);
        }
        if (name == "TeleportCard")
        {
            return new SkillCard("TeleportCard", "Teleport to any tile.", "TeleportCard", 0, 0, true);
        }
        if (name == "LassoCard")
        {
            return new SkillCard("LassoCard", "Pull opponent.", "LassoCard", 0, 0, true);
        }
        return new SkillCard("DemolitionCard", "Demolish all buildings on one enemy property.", "DemolitionCard", 0, 0, true);
    }

    inline bool hasActiveShield(const Player &player)
    {
        return player.hasActiveShield();
    }

    inline int activeDiscountPercent(const Player &player)
    {
        return player.getActiveDiscountPercent();
    }

    inline int applyDiscount(const Player &player, int amount)
    {
        int percent = activeDiscountPercent(player);
        if (percent <= 0)
            return amount;
        int discounted = amount - ((amount * percent) / 100);
        return discounted < 0 ? 0 : discounted;
    }

} // namespace GameManagerInternal
