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

namespace ash_horizon {

enum class MoveClass : std::uint8_t { Physical, Special, Status };
enum class BattlePhase : std::uint8_t { PlayerChoice, AttackAnimation, EnemyTurn, Finished };

struct BattleMove {
    const char* name;
    BattleType type;
    MoveClass category;
    int power;
    int accuracy;
};

struct BattleParticle {
    float x, y, vx, vy, life, maxLife;
    std::uint8_t r, g, b;
};

class BattleEngine {
public:
    static constexpr int MAX_HP = 100;
    static constexpr std::array<BattleMove, 4> MOVES{{
        {"Tackle", BattleType::Normal, MoveClass::Physical, 40, 100},
        {"Ember", BattleType::Fire, MoveClass::Special, 40, 100},
        {"Water Gun", BattleType::Water, MoveClass::Special, 40, 100},
        {"Thunder Shock", BattleType::Electric, MoveClass::Special, 40, 100}
    }};

    BattleEngine() : rng_(0xA5A2026u) { reset(); }

    void reset() {
        playerHp_ = enemyHp_ = MAX_HP;
        phase_ = BattlePhase::PlayerChoice;
        selectedMove_ = 0;
        animationTime_ = 0.0f;
        animationDuration_ = 0.0f;
        particleCount_ = 0;
        lastMessage_ = "Choose a move!";
    }

    int playerHp() const { return playerHp_; }
    int enemyHp() const { return enemyHp_; }
    int selectedMove() const { return selectedMove_; }
    BattlePhase phase() const { return phase_; }
    const char* message() const { return lastMessage_; }
    const std::array<BattleMove, 4>& moves() const { return MOVES; }
    int friendship() const { return friendship_; }

    // The first simple opponent is Pikachu, so its type is Electric.
    // Later, battle setup will read these types from the Pokédex.
    void setEnemyTypes(BattleType first, BattleType second, bool hasSecond = false) {
        enemyType1_ = first;
        enemyType2_ = second;
        enemyHasSecondType_ = hasSecond;
    }

    int refusalChance() const {
        if (friendship_ > 84) return 0;
        if (friendship_ >= 40) return 10;
        return 60;
    }

    void setFriendship(int value) {
        friendship_ = std::clamp(value, 0, 100);
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

        const BattleMove& move = MOVES[static_cast<std::size_t>(selectedMove_)];
        if (roll(rng_) > move.accuracy) {
            lastMessage_ = "The attack missed!";
            phase_ = BattlePhase::EnemyTurn;
            return;
        }

        const float multiplier = enemyHasSecondType_
            ? typeEffectiveness(move.type, enemyType1_, enemyType2_)
            : typeEffectiveness(move.type, enemyType1_);
        const int damage = multiplier == 0.0f ? 0 : std::max(1, static_cast<int>(
            (static_cast<float>(move.power) * multiplier) / 10.0f));
        enemyHp_ = std::max(0, enemyHp_ - damage);

        lastMessage_ = multiplier == 0.0f ? "It had no effect!" :
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
    int selectedMove_ = 0;
    int friendship_ = 100;
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

    void enemyAttack() {
        if (phase_ == BattlePhase::Finished) return;
        playerHp_ = std::max(0, playerHp_ - 8);
        lastMessage_ = playerHp_ == 0 ? "Your Pokemon fainted!" : "The opponent attacked!";
        phase_ = playerHp_ == 0 ? BattlePhase::Finished : BattlePhase::PlayerChoice;
    }
};

} // namespace ash_horizon
