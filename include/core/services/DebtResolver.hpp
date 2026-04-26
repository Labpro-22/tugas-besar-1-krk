#pragma once

#include <string>

class GameManager;
class Player;
class Property;

struct PaymentRequest {
    int amount;
    Player* payee;
    Player* creditor;
    bool allowDiscount;
    bool allowShield;
    bool allowLiquidation;
    std::string actionName;
    std::string detail;
    std::string blockDetail;

    PaymentRequest();
};

class DebtResolver {
private:
    GameManager& game;

    bool autoLiquidate(Player& player, int requiredAmount);
    bool interactiveLiquidate(Player& player, int requiredAmount);

public:
    explicit DebtResolver(GameManager& game);

    int adjustedAmount(const Player& player, int amount, bool allowDiscount) const;
    int bankSaleValue(const Property& property) const;
    int maxLiquidationValue(const Player& player) const;
    bool sellPropertyToBank(Player& player, Property& property);
    bool mortgageProperty(Player& player, Property& property);
    bool liquidate(Player& player, int requiredAmount, Player* creditor);
    bool collect(Player& payer, const PaymentRequest& request);
    void declareBankruptcy(Player& player, Player* creditor);
};
