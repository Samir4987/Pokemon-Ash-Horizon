#pragma once

#include <cstdint>

namespace ash_horizon {

enum class BattleType : std::uint8_t {
    Normal, Fire, Water, Electric, Grass, Ice, Fighting, Poison, Ground,
    Flying, Psychic, Bug, Rock, Ghost, Dragon, Dark, Steel, Fairy
};

// Generation VI onward type effectiveness. The returned value is the
// multiplier for one defending type; dual types multiply two results.
inline float singleTypeEffectiveness(BattleType attack, BattleType defend) {
    switch (attack) {
    case BattleType::Normal:
        if (defend == BattleType::Rock || defend == BattleType::Steel) return 0.5f;
        if (defend == BattleType::Ghost) return 0.0f;
        break;
    case BattleType::Fire:
        if (defend == BattleType::Grass || defend == BattleType::Ice ||
            defend == BattleType::Bug || defend == BattleType::Steel) return 2.0f;
        if (defend == BattleType::Fire || defend == BattleType::Water ||
            defend == BattleType::Rock || defend == BattleType::Dragon) return 0.5f;
        break;
    case BattleType::Water:
        if (defend == BattleType::Fire || defend == BattleType::Ground ||
            defend == BattleType::Rock) return 2.0f;
        if (defend == BattleType::Water || defend == BattleType::Grass ||
            defend == BattleType::Dragon) return 0.5f;
        break;
    case BattleType::Electric:
        if (defend == BattleType::Water || defend == BattleType::Flying) return 2.0f;
        if (defend == BattleType::Electric || defend == BattleType::Grass ||
            defend == BattleType::Dragon) return 0.5f;
        if (defend == BattleType::Ground) return 0.0f;
        break;
    case BattleType::Grass:
        if (defend == BattleType::Water || defend == BattleType::Ground ||
            defend == BattleType::Rock) return 2.0f;
        if (defend == BattleType::Fire || defend == BattleType::Grass ||
            defend == BattleType::Poison || defend == BattleType::Flying ||
            defend == BattleType::Bug || defend == BattleType::Dragon ||
            defend == BattleType::Steel) return 0.5f;
        break;
    case BattleType::Ice:
        if (defend == BattleType::Grass || defend == BattleType::Ground ||
            defend == BattleType::Flying || defend == BattleType::Dragon) return 2.0f;
        if (defend == BattleType::Fire || defend == BattleType::Water ||
            defend == BattleType::Ice || defend == BattleType::Steel) return 0.5f;
        break;
    case BattleType::Fighting:
        if (defend == BattleType::Normal || defend == BattleType::Ice ||
            defend == BattleType::Rock || defend == BattleType::Dark ||
            defend == BattleType::Steel) return 2.0f;
        if (defend == BattleType::Poison || defend == BattleType::Flying ||
            defend == BattleType::Psychic || defend == BattleType::Bug ||
            defend == BattleType::Fairy) return 0.5f;
        if (defend == BattleType::Ghost) return 0.0f;
        break;
    case BattleType::Poison:
        if (defend == BattleType::Grass || defend == BattleType::Fairy) return 2.0f;
        if (defend == BattleType::Poison || defend == BattleType::Ground ||
            defend == BattleType::Rock || defend == BattleType::Ghost) return 0.5f;
        if (defend == BattleType::Steel) return 0.0f;
        break;
    case BattleType::Ground:
        if (defend == BattleType::Fire || defend == BattleType::Electric ||
            defend == BattleType::Poison || defend == BattleType::Rock ||
            defend == BattleType::Steel) return 2.0f;
        if (defend == BattleType::Grass || defend == BattleType::Bug) return 0.5f;
        if (defend == BattleType::Flying) return 0.0f;
        break;
    case BattleType::Flying:
        if (defend == BattleType::Grass || defend == BattleType::Fighting ||
            defend == BattleType::Bug) return 2.0f;
        if (defend == BattleType::Electric || defend == BattleType::Rock ||
            defend == BattleType::Steel) return 0.5f;
        break;
    case BattleType::Psychic:
        if (defend == BattleType::Fighting || defend == BattleType::Poison) return 2.0f;
        if (defend == BattleType::Psychic || defend == BattleType::Steel) return 0.5f;
        if (defend == BattleType::Dark) return 0.0f;
        break;
    case BattleType::Bug:
        if (defend == BattleType::Grass || defend == BattleType::Psychic ||
            defend == BattleType::Dark) return 2.0f;
        if (defend == BattleType::Fire || defend == BattleType::Fighting ||
            defend == BattleType::Poison || defend == BattleType::Flying ||
            defend == BattleType::Ghost || defend == BattleType::Steel ||
            defend == BattleType::Fairy) return 0.5f;
        break;
    case BattleType::Rock:
        if (defend == BattleType::Fire || defend == BattleType::Ice ||
            defend == BattleType::Flying || defend == BattleType::Bug) return 2.0f;
        if (defend == BattleType::Fighting || defend == BattleType::Ground ||
            defend == BattleType::Steel) return 0.5f;
        break;
    case BattleType::Ghost:
        if (defend == BattleType::Psychic || defend == BattleType::Ghost) return 2.0f;
        if (defend == BattleType::Dark) return 0.5f;
        if (defend == BattleType::Normal) return 0.0f;
        break;
    case BattleType::Dragon:
        if (defend == BattleType::Dragon) return 2.0f;
        if (defend == BattleType::Steel) return 0.5f;
        if (defend == BattleType::Fairy) return 0.0f;
        break;
    case BattleType::Dark:
        if (defend == BattleType::Psychic || defend == BattleType::Ghost) return 2.0f;
        if (defend == BattleType::Fighting || defend == BattleType::Dark ||
            defend == BattleType::Fairy) return 0.5f;
        break;
    case BattleType::Steel:
        if (defend == BattleType::Ice || defend == BattleType::Rock ||
            defend == BattleType::Fairy) return 2.0f;
        if (defend == BattleType::Fire || defend == BattleType::Water ||
            defend == BattleType::Electric || defend == BattleType::Steel) return 0.5f;
        break;
    case BattleType::Fairy:
        if (defend == BattleType::Fighting || defend == BattleType::Dragon ||
            defend == BattleType::Dark) return 2.0f;
        if (defend == BattleType::Fire || defend == BattleType::Poison ||
            defend == BattleType::Steel) return 0.5f;
        break;
    }
    return 1.0f;
}

inline float typeEffectiveness(BattleType attack, BattleType firstDefender,
                               BattleType secondDefender,
                               bool hasSecondType) {
    const float first = singleTypeEffectiveness(attack, firstDefender);
    if (!hasSecondType || secondDefender == firstDefender) return first;
    return first * singleTypeEffectiveness(attack, secondDefender);
}

} // namespace ash_horizon
