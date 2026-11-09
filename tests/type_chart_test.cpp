#include "../src/type_chart.hpp"
#include <cassert>
#include <cmath>

static bool near(float a, float b) { return std::fabs(a - b) < 0.0001f; }

int main() {
    using namespace ash_horizon;

    assert(near(singleTypeEffectiveness(BattleType::Fire, BattleType::Grass), 2.0f));
    assert(near(singleTypeEffectiveness(BattleType::Electric, BattleType::Ground), 0.0f));
    assert(near(singleTypeEffectiveness(BattleType::Normal, BattleType::Ghost), 0.0f));
    assert(near(singleTypeEffectiveness(BattleType::Fighting, BattleType::Normal), 2.0f));
    assert(near(singleTypeEffectiveness(BattleType::Fairy, BattleType::Dragon), 2.0f));
    assert(near(singleTypeEffectiveness(BattleType::Poison, BattleType::Steel), 0.0f));

    // Dual types multiply: Fire is 2x against Grass and 0.5x against Water.
    assert(near(typeEffectiveness(BattleType::Fire, BattleType::Grass,
                                  BattleType::Water, true), 1.0f));
    // Ground is immune to Electric even when paired with a type weak to it.
    assert(near(typeEffectiveness(BattleType::Electric, BattleType::Ground,
                                  BattleType::Flying, true), 0.0f));
    // A duplicated secondary type must not apply effectiveness twice.
    assert(near(typeEffectiveness(BattleType::Water, BattleType::Fire,
                                  BattleType::Fire, true), 2.0f));
    return 0;
}
