#pragma once
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T>
class CardDeck {
private:
    std::vector<T*> cards;

public:
    CardDeck() = default;
    CardDeck(const CardDeck&) = delete;
    CardDeck& operator=(const CardDeck&) = delete;

    ~CardDeck() {
        clear();
    }

    bool empty() const {
        return cards.empty();
    }

    std::size_t size() const {
        return cards.size();
    }

    void addCard(T* card) {
        cards.push_back(card);
    }

    T* draw() {
        if (cards.empty()) {
            throw std::out_of_range("Deck is empty.");
        }

        T* drawn = cards.back();
        cards.pop_back();
        return drawn;
    }

    void putBack(T* card) {
        cards.insert(cards.begin(), card);
    }

    void shuffle() {
        if (cards.size() < 2) {
            return;
        }

        for (std::size_t i = cards.size() - 1; i > 0; --i) {
            std::size_t j = static_cast<std::size_t>(std::rand()) % (i + 1);
            std::swap(cards[i], cards[j]);
        }
    }

    const std::vector<T*>& getCards() const {
        return cards;
    }

    void clear() {
        for (std::size_t i = 0; i < cards.size(); ++i) {
            delete cards[i];
        }
        cards.clear();
    }
};
