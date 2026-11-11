/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

namespace ash_horizon {

enum class ItemId : std::uint8_t {
    Potion, SuperPotion, HyperPotion, MaxPotion, FullRestore,
    Revive, MaxRevive, Antidote, ParalyzeHeal, BurnHeal, IceHeal, Awakening,
    PokeBall, GreatBall, UltraBall, MasterBall, ArceusBall,
    EscapeRope, Repel, SuperRepel, MaxRepel, RareCandy,
    Ether, MaxEther, Elixir, MaxElixir, EvolutionStone,
    Count
};

class Inventory {
public:
    static constexpr std::size_t ITEM_COUNT = static_cast<std::size_t>(ItemId::Count);
    static constexpr std::uint16_t MAX_STACK = 999;

    std::uint16_t quantity(ItemId item) const {
        return quantities_[index(item)];
    }

    bool set(ItemId item, std::uint16_t amount) {
        if (index(item) >= ITEM_COUNT || amount > MAX_STACK) return false;
        quantities_[index(item)] = amount;
        return true;
    }

    bool add(ItemId item, std::uint16_t amount) {
        const std::size_t i = index(item);
        if (i >= ITEM_COUNT) return false;
        quantities_[i] = static_cast<std::uint16_t>(
            std::min<unsigned>(MAX_STACK, quantities_[i] + amount));
        return true;
    }

    bool remove(ItemId item, std::uint16_t amount = 1) {
        const std::size_t i = index(item);
        if (i >= ITEM_COUNT || quantities_[i] < amount) return false;
        quantities_[i] = static_cast<std::uint16_t>(quantities_[i] - amount);
        return true;
    }

    void grantEverything(std::uint16_t amount = MAX_STACK) {
        quantities_.fill(std::min(amount, MAX_STACK));
    }

    const std::array<std::uint16_t, ITEM_COUNT>& quantities() const {
        return quantities_;
    }

private:
    std::array<std::uint16_t, ITEM_COUNT> quantities_{};
    static constexpr std::size_t index(ItemId item) {
        return static_cast<std::size_t>(item);
    }
};

struct ArceusBallState {
    bool unlimitedTime = false;
    std::uint32_t remainingMilliseconds = 30000;

    void update(std::uint32_t elapsedMilliseconds) {
        if (unlimitedTime || remainingMilliseconds == 0) return;
        remainingMilliseconds = elapsedMilliseconds >= remainingMilliseconds
            ? 0u : remainingMilliseconds - elapsedMilliseconds;
    }

    std::uint32_t displayedTime() const {
        return unlimitedTime ? 0xFFFFFFFFu : remainingMilliseconds;
    }
};

} // namespace ash_horizon
