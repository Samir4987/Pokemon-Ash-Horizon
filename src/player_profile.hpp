/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstdint>
#include <string>

namespace ash_horizon {

enum class PlayerGender : std::uint8_t {
    Unselected = 0,
    Male = 1,
    Female = 2
};

class PlayerProfile {
public:
    static constexpr std::size_t MAX_NAME_LENGTH = 12;

    const std::string& name() const { return name_; }
    PlayerGender gender() const { return gender_; }

    bool setName(const std::string& value) {
        if (value.empty() || value.size() > MAX_NAME_LENGTH) return false;
        for (unsigned char ch : value) {
            const bool allowed = (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
                ch == ' ' || ch == '-' || ch == '_';
            if (!allowed) return false;
        }
        name_ = value;
        return true;
    }

    bool setGender(PlayerGender value) {
        if (value != PlayerGender::Unselected &&
            value != PlayerGender::Male && value != PlayerGender::Female) return false;
        gender_ = value;
        return true;
    }

    bool restore(const std::string& name, unsigned genderValue) {
        if (!setName(name) || genderValue > static_cast<unsigned>(PlayerGender::Female))
            return false;
        gender_ = static_cast<PlayerGender>(genderValue);
        return true;
    }

private:
    std::string name_ = "Trainer";
    PlayerGender gender_ = PlayerGender::Unselected;
};

class PlayerWallet {
public:
    static constexpr std::uint32_t MAX_POKE_DOLLARS = 999999999u;
    static constexpr std::uint32_t MAX_HC = 999999u;
    static constexpr std::uint32_t MAX_SHARDS = 999999u;

    std::uint32_t pokeDollars() const { return pokeDollars_; }
    std::uint32_t hc() const { return hc_; }
    std::uint32_t shards() const { return shards_; }

    bool earnPokeDollars(std::uint32_t amount) {
        return add(pokeDollars_, amount, MAX_POKE_DOLLARS);
    }
    bool earnHC(std::uint32_t amount) {
        return add(hc_, amount, MAX_HC);
    }
    bool earnShards(std::uint32_t amount) {
        return add(shards_, amount, MAX_SHARDS);
    }

    bool spendPokeDollars(std::uint32_t amount) {
        return spend(pokeDollars_, amount);
    }
    bool spendHC(std::uint32_t amount) { return spend(hc_, amount); }
    bool spendShards(std::uint32_t amount) { return spend(shards_, amount); }

    bool restore(std::uint32_t pokeDollars, std::uint32_t hc,
                 std::uint32_t shards) {
        if (pokeDollars > MAX_POKE_DOLLARS || hc > MAX_HC ||
            shards > MAX_SHARDS) return false;
        pokeDollars_ = pokeDollars;
        hc_ = hc;
        shards_ = shards;
        return true;
    }

private:
    std::uint32_t pokeDollars_ = 0;
    std::uint32_t hc_ = 0;
    std::uint32_t shards_ = 0;

    static bool add(std::uint32_t& balance, std::uint32_t amount,
                    std::uint32_t maximum) {
        if (amount > maximum - balance) return false;
        balance += amount;
        return true;
    }

    static bool spend(std::uint32_t& balance, std::uint32_t amount) {
        if (balance < amount) return false;
        balance -= amount;
        return true;
    }
};

} // namespace ash_horizon
