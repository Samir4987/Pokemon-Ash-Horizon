/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#pragma once

#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <random>
#include "type_chart.hpp"
#include "battle_stats.hpp"
#include "pokemon_party.hpp"
#include "move_data.hpp"

namespace ash_horizon {

enum class BattlePhase : std::uint8_t { PlayerChoice, AttackAnimation, EnemyTurn, Finished };

using BattleMove = MoveDefinition;

struct BattleParticle {
    float x, y, vx, vy, life, maxLife;
    std::uint8_t r, g, b;
};

class BattleEngine {
public:
    static constexpr int MAX_HP = 100;
    BattleEngine() : rng_(0xA5A2026u) { reset(); }

    void reset() {
        playerHp_ = MAX_HP;
        enemyHp_ = enemyMaxHp_;
        phase_ = BattlePhase::PlayerChoice;
        selectedMove_ = 0;
        animationTime_ = 0.0f;
        animationDuration_ = 0.0f;
        particleCount_ = 0;
        lastMessage_ = "Choose a move!";
    }

    int playerHp() const {
        const PartyPokemon* pokemon = party_ ? party_->active() : nullptr;
        return pokemon ? pokemon->currentHp : playerHp_;
    }
    int playerMaxHp() const {
        const PartyPokemon* pokemon = party_ ? party_->active() : nullptr;
        return pokemon ? pokemon->maxHp : MAX_HP;
    }
    const PartyPokemon* activePokemon() const {
        return party_ ? party_->active() : nullptr;
    }
    void setParty(PokemonParty* party) { party_ = party; }
    int enemyHp() const { return enemyHp_; }
    int enemyMaxHp() const { return enemyMaxHp_; }
    void setDatabase(const PokemonDatabase* database) { database_ = database; }

    void setEnemySpecies(std::uint16_t dex, std::uint8_t level) {
        enemyDex_ = dex;
        enemyLevel_ = std::clamp<int>(level, 1, 100);
        const SpeciesRecord* species = database_ ? database_->findSpecies(dex) : nullptr;
        if (!species) return;
        enemyType1_ = toBattleType(species->type1);
        enemyHasSecondType_ = species->type2 != PokemonType::None;
        enemyType2_ = enemyHasSecondType_ ? toBattleType(species->type2) : BattleType::Normal;
        enemyStats_ = calculateStats(species, enemyLevel_);
        enemyMaxHp_ = std::max(1, enemyStats_.hp);
    }
    int selectedMove() const { return selectedMove_; }
    BattlePhase phase() const { return phase_; }
    const char* message() const { return lastMessage_; }
    std::array<BattleMove, 4> moves() const {
        std::array<BattleMove, 4> result{};
        const PartyPokemon* pokemon = party_ ? party_->active() : nullptr;
        const MoveDefinition* fallback = findMoveDefinition(
            static_cast<std::uint16_t>(MoveId::Tackle));
        for (std::size_t i = 0; i < result.size(); ++i) {
            const MoveDefinition* definition = pokemon && i < pokemon->moveCount
                ? findMoveDefinition(pokemon->moveIds[i]) : nullptr;
            result[i] = definition ? *definition : *fallback;
        }
        return result;
    }
    int movePp(std::size_t index) const {
        const PartyPokemon* pokemon = party_ ? party_->active() : nullptr;
        return pokemon && index < pokemon->moveCount ? pokemon->movePp[index] : 0;
    }
    int friendship() const {
        const PartyPokemon* pokemon = party_ ? party_->active() : nullptr;
        return pokemon ? pokemon->friendship : friendship_;
    }

    // Fallback type setter for test battles without a species record.
    void setEnemyTypes(BattleType first, BattleType second, bool hasSecond = false) {
        enemyType1_ = first;
        enemyType2_ = second;
        enemyHasSecondType_ = hasSecond;
    }

    int refusalChance() const {
        const int currentFriendship = friendship();
        if (currentFriendship > 84) return 0;
        if (currentFriendship >= 40) return 10;
        return 60;
    }

    void setFriendship(int value) {
        const int bounded = std::clamp(value, 0, 100);
        if (party_ && party_->active()) {
            party_->active()->friendship = static_cast<std::uint8_t>(bounded);
        } else {
            friendship_ = bounded;
        }
    }

    void captureSucceeded(const char* message) {
        lastMessage_ = message;
        phase_ = BattlePhase::Finished;
    }

    void captureFailed() {
        lastMessage_ = "The Pokemon broke free!";
        phase_ = BattlePhase::EnemyTurn;
    }

    void captureUnavailable(const char* message) {
        lastMessage_ = message;
    }

    void switchTurn() {
        if (phase_ != BattlePhase::PlayerChoice) return;
        lastMessage_ = "You changed Pokemon!";
        phase_ = BattlePhase::EnemyTurn;
    }

    void runAway() {
        if (phase_ != BattlePhase::PlayerChoice) return;
        lastMessage_ = "You got away safely! Press E.";
        phase_ = BattlePhase::Finished;
    }

    void moveSelection(int direction) {
        if (phase_ != BattlePhase::PlayerChoice) return;
        selectedMove_ = (selectedMove_ + direction + 4) % 4;
    }

    void chooseMove() {
        if (phase_ != BattlePhase::PlayerChoice) return;
        std::uniform_int_distribution<int> roll(1, 100);
        if (refusalChance() > 0 && roll(rng_) <= refusalChance()) {
            lastMessage_ = "Your Pokemon refused to attack!";
            phase_ = BattlePhase::EnemyTurn;
            return;
        }

        PartyPokemon* attacker = party_ ? party_->active() : nullptr;
        const std::array<BattleMove, 4> availableMoves = moves();
        const BattleMove& move = availableMoves[static_cast<std::size_t>(selectedMove_)];
        if (attacker && (static_cast<std::size_t>(selectedMove_) >= attacker->moveCount ||
                         attacker->movePp[static_cast<std::size_t>(selectedMove_)] == 0)) {
            lastMessage_ = "No PP left!";
            return;
        }
        if (attacker)
            --attacker->movePp[static_cast<std::size_t>(selectedMove_)];
        if (roll(rng_) > move.accuracy) {
            lastMessage_ = "The attack missed!";
            phase_ = BattlePhase::EnemyTurn;
            return;
        }

        const PartyPokemon* attacker = party_ ? party_->active() : nullptr;
        const SpeciesRecord* attackerSpecies = attacker && database_
            ? database_->findSpecies(attacker->nationalDex) : nullptr;
        const BattleStats attackerStats = calculateStats(
            attackerSpecies, attacker ? attacker->level : 5);
        const float multiplier = enemyHasSecondType_
            ? typeEffectiveness(move.type, enemyType1_, enemyType2_)
            : typeEffectiveness(move.type, enemyType1_);
        const int attackStat = move.category == MoveClass::Physical
            ? attackerStats.attack : attackerStats.specialAttack;
        const int defenseStat = move.category == MoveClass::Physical
            ? enemyStats_.defense : enemyStats_.specialDefense;
        const bool stab = attackerSpecies &&
            (toBattleType(attackerSpecies->type1) == move.type ||
             (attackerSpecies->type2 != PokemonType::None &&
              toBattleType(attackerSpecies->type2) == move.type));
        std::uniform_real_distribution<float> damageRoll(0.85f, 1.0f);
        std::uniform_int_distribution<int> criticalRoll(1, 24);
        const bool critical = criticalRoll(rng_) == 1;
        const int damage = calculateDamage(attacker ? attacker->level : 5,
            move.power, attackStat, defenseStat, stab, multiplier,
            damageRoll(rng_), critical);
        enemyHp_ = std::max(0, enemyHp_ - damage);

        lastMessage_ = multiplier == 0.0f ? "It had no effect!" :
                       critical ? "A critical hit!" :
                       multiplier > 1.0f ? "It's super effective!" :
                       multiplier < 1.0f ? "It's not very effective..." : "Direct hit!";
        startAnimation(move.type);
        if (enemyHp_ == 0) {
            phase_ = BattlePhase::Finished;
            lastMessage_ = "You won the battle!";
        } else {
            phase_ = BattlePhase::AttackAnimation;
        }
    }

    void update(float dt) {
        if (phase_ == BattlePhase::AttackAnimation) {
            animationTime_ += dt;
            updateParticles(dt);
            if (animationTime_ >= animationDuration_) {
                phase_ = BattlePhase::EnemyTurn;
                enemyAttack();
            }
        } else if (phase_ == BattlePhase::EnemyTurn) {
            enemyAttack();
        }
    }

    void drawEffects(SDL_Renderer* renderer) const {
        for (std::size_t i = 0; i < particleCount_; ++i) {
            const auto& p = particles_[i];
            if (p.maxLife <= 0.0f) continue;
            const float alpha = std::clamp(p.life / p.maxLife, 0.0f, 1.0f);
            SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b,
                                   static_cast<Uint8>(255.0f * alpha));
            SDL_FRect pixel{p.x, p.y, 3.0f, 3.0f};
            SDL_RenderFillRect(renderer, &pixel);
        }
        if (phase_ == BattlePhase::AttackAnimation && animationTime_ < 0.12f) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 110);
            SDL_FRect flash{0, 0, 240, 160};
            SDL_RenderFillRect(renderer, &flash);
        }
    }

private:
    int playerHp_ = MAX_HP;
    int enemyHp_ = MAX_HP;
    int enemyMaxHp_ = MAX_HP;
    std::uint16_t enemyDex_ = 0;
    int enemyLevel_ = 5;
    const PokemonDatabase* database_ = nullptr;
    BattleStats enemyStats_{50, 50, 50, 50, 50, 50};
    int selectedMove_ = 0;
    int friendship_ = 100;\n    int enemyAttackStage_ = 0;\n    int enemyDefenseStage_ = 0;
    PokemonParty* party_ = nullptr;
    BattleType enemyType1_ = BattleType::Electric;
    BattleType enemyType2_ = BattleType::Normal;
    bool enemyHasSecondType_ = false;
    BattlePhase phase_ = BattlePhase::PlayerChoice;
    float animationTime_ = 0.0f;
    float animationDuration_ = 0.55f;
    const char* lastMessage_ = "Choose a move!";
    std::array<BattleParticle, 48> particles_{};
    std::size_t particleCount_ = 0;
    std::mt19937 rng_;

    void startAnimation(BattleType type) {
        animationTime_ = 0.0f;
        animationDuration_ = 0.62f;
        particleCount_ = particles_.size();
        Uint8 r = 235, g = 220, b = 150;
        if (type == BattleType::Fire) { r = 255; g = 105; b = 35; }
        if (type == BattleType::Water) { r = 65; g = 165; b = 255; }
        if (type == BattleType::Electric) { r = 255; g = 240; b = 55; }
        if (type == BattleType::Grass) { r = 90; g = 210; b = 85; }
        for (std::size_t i = 0; i < particleCount_; ++i) {
            const float angle = static_cast<float>(i) * 6.2831853f /
                                static_cast<float>(particleCount_);
            const float speed = 22.0f + static_cast<float>(i % 7) * 5.0f;
            particles_[i] = {174.0f, 43.0f,
                std::cos(angle) * speed, std::sin(angle) * speed,
                0.62f, 0.62f, r, g, b};
        }
    }

    void updateParticles(float dt) {
        for (std::size_t i = 0; i < particleCount_; ++i) {
            auto& p = particles_[i];
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.life = std::max(0.0f, p.life - dt);
        }
    }

    static BattleType toBattleType(PokemonType type) {
        if (type == PokemonType::None) return BattleType::Normal;
        return static_cast<BattleType>(static_cast<unsigned>(type) - 1u);
    }

    static BattleStats calculateStats(const SpeciesRecord* species, int level) {
        StatInputs input;
        input.level = level;
        if (species) {
            input.baseHp = species->baseStats.hp;
            input.baseAttack = species->baseStats.attack;
            input.baseDefense = species->baseStats.defense;
            input.baseSpecialAttack = species->baseStats.specialAttack;
            input.baseSpecialDefense = species->baseStats.specialDefense;
            input.baseSpeed = species->baseStats.speed;
        } else {
            input.baseHp = 45; input.baseAttack = 49; input.baseDefense = 49;
            input.baseSpecialAttack = 65; input.baseSpecialDefense = 65;
            input.baseSpeed = 45;
        }
        return calculateBattleStats(input);
    }

    void enemyAttack() {
        if (phase_ == BattlePhase::Finished) return;
        const PartyPokemon* defender = party_ ? party_->active() : nullptr;
        const SpeciesRecord* defenderSpecies = defender && database_
            ? database_->findSpecies(defender->nationalDex) : nullptr;
        const SpeciesRecord* attackerSpecies = database_
            ? database_->findSpecies(enemyDex_) : nullptr;
        const BattleStats defenderStats = calculateStats(
            defenderSpecies, defender ? defender->level : 5);
        const auto enemyMoveIds = defaultMoveIds(enemyDex_);
        const MoveDefinition* selectedEnemyMove = nullptr;
        for (std::size_t attempt = 0; attempt < enemyMoveIds.size(); ++attempt) {
            const std::size_t slot = static_cast<std::size_t>(rng_() % enemyMoveIds.size());
            const MoveDefinition* candidate = findMoveDefinition(enemyMoveIds[slot]);
            if (candidate && candidate->category != MoveClass::Status) {
                selectedEnemyMove = candidate;
                break;
            }
        }
        if (!selectedEnemyMove) selectedEnemyMove = findMoveDefinition(
            static_cast<std::uint16_t>(MoveId::Tackle));
        const BattleMove& move = *selectedEnemyMove;
        const int attackStat = move.category == MoveClass::Physical
            ? applyStatStage(enemyStats_.attack, 0) : enemyStats_.specialAttack;
        const int defenseStat = move.category == MoveClass::Physical
            ? defenderStats.defense : defenderStats.specialDefense;
        const BattleType defend1 = defenderSpecies
            ? toBattleType(defenderSpecies->type1) : BattleType::Normal;
        const bool hasSecond = defenderSpecies &&
            defenderSpecies->type2 != PokemonType::None;
        const BattleType defend2 = hasSecond
            ? toBattleType(defenderSpecies->type2) : BattleType::Normal;
        const float multiplier = hasSecond
            ? typeEffectiveness(move.type, defend1, defend2)
            : typeEffectiveness(move.type, defend1);
        const bool stab = attackerSpecies &&
            (toBattleType(attackerSpecies->type1) == move.type ||
             (attackerSpecies->type2 != PokemonType::None &&
              toBattleType(attackerSpecies->type2) == move.type));
        std::uniform_real_distribution<float> damageRoll(0.85f, 1.0f);
        std::uniform_int_distribution<int> criticalRoll(1, 24);
        const bool critical = criticalRoll(rng_) == 1;
        const int damage = calculateDamage(enemyLevel_, move.power, attackStat,
            defenseStat, stab, multiplier, damageRoll(rng_), critical);
        if (party_ && defender) party_->damageActive(damage);
        else playerHp_ = std::max(0, playerHp_ - damage);
        lastMessage_ = playerHp() == 0 ? "Your Pokemon fainted!" :
            critical ? "The opponent scored a critical hit!" : "The opponent attacked!";
        phase_ = playerHp() == 0 ? BattlePhase::Finished : BattlePhase::PlayerChoice;
    }
};

} // namespace ash_horizon
