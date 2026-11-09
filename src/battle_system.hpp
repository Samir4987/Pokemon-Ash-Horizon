#pragma once

#include <SDL3/SDL.h>
#include "battle_stats.hpp"
#include "type_chart.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <random>

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

struct BattleCombatant {
    StatInputs inputs{};
    BattleStats stats{};
    BattleType primaryType = BattleType::Normal;
    BattleType secondaryType = BattleType::Normal;
    bool hasSecondaryType = false;
    int hp = 1;

    void recalculate() {
        stats = calculateBattleStats(inputs);
        hp = std::clamp(hp, 0, stats.hp);
    }

    void restore() {
        stats = calculateBattleStats(inputs);
        hp = stats.hp;
    }
};

class BattleEngine {
public:
    static constexpr std::array<BattleMove, 4> MOVES{{
        {"Tackle", BattleType::Normal, MoveClass::Physical, 40, 100},
        {"Ember", BattleType::Fire, MoveClass::Special, 40, 100},
        {"Water Gun", BattleType::Water, MoveClass::Special, 40, 100},
        {"Thunder Shock", BattleType::Electric, MoveClass::Special, 40, 100}
    }};

    BattleEngine() : rng_(0xA5A2026u) {
        // Temporary encounter data until wild encounters load Pokédex records.
        player_.inputs.level = 20;
        player_.inputs.baseHp = 45;
        player_.inputs.baseAttack = 49;
        player_.inputs.baseDefense = 49;
        player_.inputs.baseSpecialAttack = 65;
        player_.inputs.baseSpecialDefense = 65;
        player_.inputs.baseSpeed = 45;
        player_.inputs.ivs = {{31, 31, 31, 31, 31, 31}};
        player_.primaryType = BattleType::Grass;
        player_.secondaryType = BattleType::Poison;
        player_.hasSecondaryType = true;

        enemy_.inputs.level = 20;
        enemy_.inputs.baseHp = 35;
        enemy_.inputs.baseAttack = 55;
        enemy_.inputs.baseDefense = 40;
        enemy_.inputs.baseSpecialAttack = 50;
        enemy_.inputs.baseSpecialDefense = 50;
        enemy_.inputs.baseSpeed = 90;
        enemy_.inputs.ivs = {{31, 31, 31, 31, 31, 31}};
        enemy_.primaryType = BattleType::Electric;

        player_.restore();
        enemy_.restore();
        reset();
    }

    void reset() {
        player_.restore();
        enemy_.restore();
        phase_ = BattlePhase::PlayerChoice;
        selectedMove_ = 0;
        animationTime_ = 0.0f;
        animationDuration_ = 0.0f;
        particleCount_ = 0;
        lastMessage_ = "Choose a move!";
    }

    int playerHp() const { return player_.hp; }
    int enemyHp() const { return enemy_.hp; }
    int playerMaxHp() const { return player_.stats.hp; }
    int enemyMaxHp() const { return enemy_.stats.hp; }
    int playerHpBar() const { return hpBar(player_); }
    int enemyHpBar() const { return hpBar(enemy_); }
    int selectedMove() const { return selectedMove_; }
    BattlePhase phase() const { return phase_; }
    const char* message() const { return lastMessage_; }
    const std::array<BattleMove, 4>& moves() const { return MOVES; }
    int friendship() const { return friendship_; }

    int refusalChance() const {
        if (friendship_ > 84) return 0;
        if (friendship_ >= 40) return 10;
        return 60;
    }

    void setFriendship(int value) {
        friendship_ = std::clamp(value, 0, 100);
    }

    // Encounter code supplies real Pokédex stats and types through these setters.
    void setPlayerPokemon(const StatInputs& inputs, BattleType primary,
                          BattleType secondary = BattleType::Normal,
                          bool hasSecondary = false) {
        player_.inputs = inputs;
        player_.primaryType = primary;
        player_.secondaryType = secondary;
        player_.hasSecondaryType = hasSecondary;
        player_.restore();
    }

    void setEnemyPokemon(const StatInputs& inputs, BattleType primary,
                         BattleType secondary = BattleType::Normal,
                         bool hasSecondary = false) {
        enemy_.inputs = inputs;
        enemy_.primaryType = primary;
        enemy_.secondaryType = secondary;
        enemy_.hasSecondaryType = hasSecondary;
        enemy_.restore();
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
        if (roll(rng_) > std::clamp(move.accuracy, 1, 100)) {
            lastMessage_ = "The attack missed!";
            phase_ = BattlePhase::EnemyTurn;
            return;
        }

        const float multiplier = effectiveness(move.type, enemy_);
        std::uniform_int_distribution<int> randomRoll(85, 100);
        const float randomFactor = static_cast<float>(randomRoll(rng_)) / 100.0f;
        std::uniform_int_distribution<int> criticalRoll(1, 24);
        const bool critical = criticalRoll(rng_) == 1;
        const int damage = resolveDamage(move, player_, enemy_, multiplier,
                                         critical, randomFactor);
        enemy_.hp = std::max(0, enemy_.hp - damage);

        if (multiplier == 0.0f) lastMessage_ = "It had no effect!";
        else if (critical) lastMessage_ = "A critical hit!";
        else if (multiplier > 1.0f) lastMessage_ = "It's super effective!";
        else if (multiplier < 1.0f) lastMessage_ = "It's not very effective...";
        else lastMessage_ = "Direct hit!";

        if (damage > 0) startAnimation(move.type);
        if (enemy_.hp == 0) {
            phase_ = BattlePhase::Finished;
            lastMessage_ = "You won the battle!";
        } else {
            phase_ = damage > 0 ? BattlePhase::AttackAnimation : BattlePhase::EnemyTurn;
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
    BattleCombatant player_{};
    BattleCombatant enemy_{};
    int selectedMove_ = 0;
    int friendship_ = 100;
    BattlePhase phase_ = BattlePhase::PlayerChoice;
    float animationTime_ = 0.0f;
    float animationDuration_ = 0.55f;
    const char* lastMessage_ = "Choose a move!";
    std::array<BattleParticle, 48> particles_{};
    std::size_t particleCount_ = 0;
    std::mt19937 rng_;

    static int hpBar(const BattleCombatant& pokemon) {
        if (pokemon.stats.hp <= 0) return 0;
        return std::clamp(pokemon.hp * 20 / pokemon.stats.hp, 0, 20);
    }

    static float effectiveness(BattleType attack, const BattleCombatant& defender) {
        return typeEffectiveness(attack, defender.primaryType,
                                 defender.secondaryType, defender.hasSecondaryType);
    }

    static int resolveDamage(const BattleMove& move,
                             const BattleCombatant& attacker,
                             const BattleCombatant& defender,
                             float multiplier, bool critical = false,
                             float randomFactor = 1.0f) {
        if (move.category == MoveClass::Status || move.power <= 0 || multiplier <= 0.0f)
            return 0;
        const bool stab = move.type == attacker.primaryType ||
            (attacker.hasSecondaryType && move.type == attacker.secondaryType);
        const int attack = move.category == MoveClass::Physical
            ? attacker.stats.attack : attacker.stats.specialAttack;
        const int defense = move.category == MoveClass::Physical
            ? defender.stats.defense : defender.stats.specialDefense;
        return calculateDamage(attacker.inputs.level, move.power, attack, defense,
                               stab, multiplier, randomFactor, critical);
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
        static const BattleMove enemyMove{
            "Thunder Shock", BattleType::Electric, MoveClass::Special, 40, 100
        };
        std::uniform_int_distribution<int> accuracyRoll(1, 100);
        if (accuracyRoll(rng_) > enemyMove.accuracy) {
            lastMessage_ = "The opponent's attack missed!";
            phase_ = BattlePhase::PlayerChoice;
            return;
        }

        const float multiplier = effectiveness(enemyMove.type, player_);
        std::uniform_int_distribution<int> randomRoll(85, 100);
        const float randomFactor = static_cast<float>(randomRoll(rng_)) / 100.0f;
        std::uniform_int_distribution<int> criticalRoll(1, 24);
        const bool critical = criticalRoll(rng_) == 1;
        const int damage = resolveDamage(enemyMove, enemy_, player_, multiplier,
                                         critical, randomFactor);
        player_.hp = std::max(0, player_.hp - damage);

        if (multiplier == 0.0f) lastMessage_ = "Your Pokemon was unaffected!";
        else if (critical) lastMessage_ = "A critical hit!";
        else lastMessage_ = "The opponent attacked!";

        phase_ = player_.hp == 0 ? BattlePhase::Finished : BattlePhase::PlayerChoice;
        if (phase_ == BattlePhase::Finished) lastMessage_ = "Your Pokemon fainted!";
    }
};

} // namespace ash_horizon
