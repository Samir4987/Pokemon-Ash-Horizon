#pragma once

#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <random>

namespace ash_horizon {

enum class BattleType : std::uint8_t {
    Normal, Fire, Water, Electric, Grass, Ice, Fighting, Poison, Ground,
    Flying, Psychic, Bug, Rock, Ghost, Dragon, Dark, Steel, Fairy
};

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
        particles_.clear();
        lastMessage_ = "Choose a move!";
    }

    int playerHp() const { return playerHp_; }
    int enemyHp() const { return enemyHp_; }
    int selectedMove() const { return selectedMove_; }
    BattlePhase phase() const { return phase_; }
    const char* message() const { return lastMessage_; }
    const std::array<BattleMove, 4>& moves() const { return MOVES; }

    void moveSelection(int direction) {
        if (phase_ != BattlePhase::PlayerChoice) return;
        selectedMove_ = (selectedMove_ + direction + 4) % 4;
    }

    void chooseMove() {
        if (phase_ != BattlePhase::PlayerChoice) return;
        const BattleMove& move = MOVES[static_cast<std::size_t>(selectedMove_)];
        std::uniform_int_distribution<int> roll(1, 100);
        if (roll(rng_) > move.accuracy) {
            lastMessage_ = "The attack missed!";
            phase_ = BattlePhase::EnemyTurn;
            return;
        }
        const float multiplier = effectiveness(move.type, BattleType::Normal);
        const int damage = std::max(1, static_cast<int>(
            (static_cast<float>(move.power) * multiplier) / 10.0f));
        enemyHp_ = std::max(0, enemyHp_ - damage);
        lastMessage_ = multiplier > 1.0f ? "It's super effective!" :
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
        for (const auto& p : particles_) {
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
    BattlePhase phase_ = BattlePhase::PlayerChoice;
    float animationTime_ = 0.0f;
    float animationDuration_ = 0.55f;
    const char* lastMessage_ = "Choose a move!";
    std::array<BattleParticle, 48> particles_{};
    std::size_t particleCount_ = 0;
    std::mt19937 rng_;

    static float effectiveness(BattleType attack, BattleType defend) {
        // First-pass core chart; expanded to the complete 18-type matrix next.
        if (defend == BattleType::Normal) {
            if (attack == BattleType::Fighting) return 2.0f;
            if (attack == BattleType::Ghost) return 0.0f;
        }
        if (defend == BattleType::Fire) {
            if (attack == BattleType::Water || attack == BattleType::Ground ||
                attack == BattleType::Rock) return 2.0f;
            if (attack == BattleType::Fire || attack == BattleType::Grass ||
                attack == BattleType::Ice || attack == BattleType::Bug ||
                attack == BattleType::Steel || attack == BattleType::Fairy) return 0.5f;
        }
        if (defend == BattleType::Water) {
            if (attack == BattleType::Electric || attack == BattleType::Grass) return 2.0f;
            if (attack == BattleType::Fire || attack == BattleType::Water ||
                attack == BattleType::Ice || attack == BattleType::Steel) return 0.5f;
        }
        if (defend == BattleType::Grass) {
            if (attack == BattleType::Fire || attack == BattleType::Ice ||
                attack == BattleType::Poison || attack == BattleType::Flying ||
                attack == BattleType::Bug) return 2.0f;
            if (attack == BattleType::Water || attack == BattleType::Electric ||
                attack == BattleType::Grass || attack == BattleType::Ground) return 0.5f;
        }
        if (defend == BattleType::Ground && attack == BattleType::Electric) return 0.0f;
        if (defend == BattleType::Flying && attack == BattleType::Ground) return 0.0f;
        if (defend == BattleType::Electric) {
            if (attack == BattleType::Ground) return 2.0f;
            if (attack == BattleType::Electric || attack == BattleType::Flying ||
                attack == BattleType::Steel) return 0.5f;
        }
        return 1.0f;
    }

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
