#pragma once

#include <array>
#include <cmath>
#include <cstdint>

namespace ash_horizon {

enum class WildMood : std::uint8_t {
    Wander,
    Rest,
    Curious,
    Flee,
    Play
};

struct WildCreature {
    float x;
    float y;
    float homeX;
    float homeY;
    float vx;
    float vy;
    float stateTime;
    float animationTime;
    std::uint8_t species;
    WildMood mood;
};

class LivingWorld {
public:
    LivingWorld()
        : creatures_{{
            {238.0f, 112.0f, 238.0f, 112.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0, WildMood::Wander},
            {286.0f, 128.0f, 286.0f, 128.0f, 0.0f, 0.0f, 1.8f, 0.2f, 1, WildMood::Wander},
            {334.0f, 144.0f, 334.0f, 144.0f, 0.0f, 0.0f, 2.2f, 0.4f, 2, WildMood::Rest},
            {142.0f, 208.0f, 142.0f, 208.0f, 0.0f, 0.0f, 1.4f, 0.1f, 0, WildMood::Wander},
            {174.0f, 240.0f, 174.0f, 240.0f, 0.0f, 0.0f, 2.0f, 0.3f, 2, WildMood::Rest},
            {206.0f, 256.0f, 206.0f, 256.0f, 0.0f, 0.0f, 1.6f, 0.5f, 1, WildMood::Wander}
        }} {}

    void update(float frameDelta, float playerX, float playerY,
                bool (*isBlocked)(float, float)) {
        // A low-frequency simulation tick keeps the nearby ecosystem inexpensive.
        accumulator_ += frameDelta;
        if (accumulator_ > 0.5f) accumulator_ = 0.5f;
        while (accumulator_ >= TICK_SECONDS) {
            tick(playerX, playerY, isBlocked);
            accumulator_ -= TICK_SECONDS;
        }
        for (auto& creature : creatures_) creature.animationTime += frameDelta;
    }

    const std::array<WildCreature, 6>& creatures() const { return creatures_; }

private:
    static constexpr float TICK_SECONDS = 0.12f;
    std::array<WildCreature, 6> creatures_;
    float accumulator_ = 0.0f;
    std::uint32_t randomState_ = 0xA51A2026u;

    float randomUnit() {
        randomState_ = randomState_ * 1664525u + 1013904223u;
        return static_cast<float>((randomState_ >> 8) & 0x00FFFFFFu) / 16777215.0f;
    }

    static float distanceSquared(float ax, float ay, float bx, float by) {
        const float dx = ax - bx;
        const float dy = ay - by;
        return dx * dx + dy * dy;
    }

    void chooseDirection(WildCreature& c, float speed) {
        const float angle = randomUnit() * 6.28318530718f;
        c.vx = std::cos(angle) * speed;
        c.vy = std::sin(angle) * speed;
    }

    void tick(float playerX, float playerY, bool (*isBlocked)(float, float)) {
        // First resolve proximity reactions and social play.
        for (std::size_t i = 0; i < creatures_.size(); ++i) {
            auto& c = creatures_[i];
            const float playerDistance = distanceSquared(c.x, c.y, playerX, playerY);
            if (playerDistance < 38.0f * 38.0f && c.species != 2) {
                c.mood = WildMood::Flee;
                c.stateTime = 1.2f + randomUnit() * 0.8f;
                const float dx = c.x - playerX;
                const float dy = c.y - playerY;
                const float length = (std::sqrt(dx * dx + dy * dy) > 0.01f)
                    ? std::sqrt(dx * dx + dy * dy) : 1.0f;
                c.vx = dx / length * 34.0f;
                c.vy = dy / length * 34.0f;
                continue;
            }
            if (playerDistance < 58.0f * 58.0f && c.species == 2 &&
                c.mood != WildMood::Flee && c.mood != WildMood::Play) {
                c.mood = WildMood::Curious;
                c.stateTime = 1.0f;
                const float dx = playerX - c.x;
                const float dy = playerY - c.y;
                const float length = std::sqrt(dx * dx + dy * dy) + 0.001f;
                c.vx = dx / length * 12.0f;
                c.vy = dy / length * 12.0f;
            }
        }

        for (std::size_t i = 0; i < creatures_.size(); ++i) {
            auto& a = creatures_[i];
            if (a.mood == WildMood::Flee) continue;
            for (std::size_t j = i + 1; j < creatures_.size(); ++j) {
                auto& b = creatures_[j];
                if (a.species == b.species &&
                    distanceSquared(a.x, a.y, b.x, b.y) < 22.0f * 22.0f &&
                    a.mood != WildMood::Flee && b.mood != WildMood::Flee) {
                    a.mood = b.mood = WildMood::Play;
                    a.stateTime = b.stateTime = 1.5f;
                    a.vx = -12.0f; a.vy = 8.0f;
                    b.vx = 12.0f; b.vy = -8.0f;
                }
            }
        }

        for (auto& c : creatures_) {
            c.stateTime -= TICK_SECONDS;
            if (c.stateTime <= 0.0f) {
                const float roll = randomUnit();
                if (roll < 0.22f) {
                    c.mood = WildMood::Rest;
                    c.stateTime = 1.0f + randomUnit() * 2.0f;
                    c.vx = c.vy = 0.0f;
                } else {
                    c.mood = WildMood::Wander;
                    c.stateTime = 1.0f + randomUnit() * 2.5f;
                    chooseDirection(c, 8.0f + randomUnit() * 8.0f);
                }
            }

            const float nextX = c.x + c.vx * TICK_SECONDS;
            const float nextY = c.y + c.vy * TICK_SECONDS;
            if (nextX < 18.0f || nextX > 450.0f ||
                nextY < 18.0f || nextY > 290.0f ||
                (isBlocked && isBlocked(nextX, nextY))) {
                c.vx = -c.vx;
                c.vy = -c.vy;
                c.stateTime = 0.0f;
            } else {
                c.x = nextX;
                c.y = nextY;
            }
        }
    }
};

} // namespace ash_horizon
