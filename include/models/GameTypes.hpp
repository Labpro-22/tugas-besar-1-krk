#ifndef GAMETYPES_HPP
#define GAMETYPES_HPP
#include <string>
#include <map>
#include <vector>
using namespace std;

enum class PropertyType
{
    STREET,
    RAILROAD,
    UTILITIY
};

enum class PropertyStatus
{
    BANK,
    OWNED,
    MORTGAGED
};

enum class PlayerStatus
{
    ACTIVE,
    JAILED,
    BANKRUPT
};

enum class ActionType
{
    CHANCE,
    COMMUNITY_CHEST,
    FESTIVAL,
    TAX_PPH,
    TAX_PBM
};

enum class SpecialType
{
    GO,
    JAIL,
    FREE_PARKING,
    GO_TO_JAIL
};

enum class CardType
{
    CHANCE,
    COMMUNITY_CHEST,
    SKILL
};

enum class CardEffectType
{
    RECEIVE_MONEY,
    PAY_MONEY,
    MOVE_RELATIVE,
    MOVE_TO_POSITION,
    GO_TO_JAIL,
    GET_OUT_OF_JAIL,
    PAY_EACH_PLAYER,
    RECEIVE_FROM_EACH_PLAYER
};

class PropertyDefinition
{
public:
    int id;
    string code;
    string name;
    PropertyType type;
    string colorGroup;
    int purchasePrice;
    int mortgageValue;
    int houseCost;
    int hotelCost;
    int colorGroupSize;
    vector<int> rentTable;

    PropertyDefinition();
};

class TaxConfig
{
public:
    int pphFlat;
    int pphPercentage;
    int pbmFlat;

    TaxConfig();
};

class SpecialConfig
{
public:
    int goSalary;
    int jailFine;

    SpecialConfig();
};

class MiscConfig
{
public:
    int maxTurn;
    int startingBalance;

    MiscConfig();
};

class GameConfig
{
public:
    vector<PropertyDefinition> properties;
    map<int, int> railroadRentByCount;
    map<int, int> utilityMultiplierByCount;
    TaxConfig tax;
    SpecialConfig special;
    MiscConfig misc;

    GameConfig();
};

class SavedSkillCardState
{
public:
    std::string cardName;
    int value;
    int remainingDuration;

    SavedSkillCardState();
};

class SavedPropertyState
{
public:
    string code;
    string ownerName;
    PropertyType propertyType;
    PropertyStatus status;
    int buildingLevel;
    int festivalMultiplier;
    int festivalTurnsRemaining;

    SavedPropertyState();
};

class SavedPlayerState
{
public:
    string name;
    int money;
    string tileCode;
    PlayerStatus status;
    int jailTurnsSpent;
    vector<SavedSkillCardState> skillCards;

    SavedPlayerState();
};

class SavedDeckState
{
public:
    vector<std::string> skillDeckCardNames;

    SavedDeckState();
};

class LogEntry
{
public:
    int turnNumber;
    string username;
    string actionName;
    string detail;

    LogEntry();
    LogEntry(int turnNumber, const string &username, const string &actionName, const string &detail);
};

class GameSaveData
{
public:
    int currentTurn;
    int maxTurn;
    vector<SavedPlayerState> players;
    vector<std::string> turnOrderNames;
    int currentPlayerIndex;
    vector<SavedPropertyState> properties;
    SavedDeckState skillDeckState;
    vector<LogEntry> logEntries;

    GameSaveData();
};

#endif