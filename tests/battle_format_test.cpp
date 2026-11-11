#include "../src/battle_format.hpp"
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    using namespace ash_horizon;

    const SaveBattleRules a = SaveBattleRules::create(12345u);
    const SaveBattleRules b = SaveBattleRules::create(12345u);
    assert(a.preferredStyle == b.preferredStyle);
    assert(a.seed == 12345u);

    // A save can never start a Double Battle while the player has only one
    // usable Pokémon, even if that save's randomized preference is Double.
    SaveBattleRules doubleSave{77u, BattleStyle::Double};
    assert(doubleSave.resolve(0) == BattleStyle::Single);
    assert(doubleSave.resolve(1) == BattleStyle::Single);
    assert(doubleSave.resolve(2) == BattleStyle::Double);
    assert(doubleSave.resolve(6) == BattleStyle::Double);
    assert(doubleSave.activePokemonPerSide(1) == 1);
    assert(doubleSave.activePokemonPerSide(6) == 2);

    SaveBattleRules singleSave{88u, BattleStyle::Single};
    assert(singleSave.resolve(6) == BattleStyle::Single);
    assert(singleSave.activePokemonPerSide(6) == 1);

    for (std::size_t count = 0; count <= 6; ++count)
        assert(doubleSave.opponentTeamSize(count) == count);
    assert(doubleSave.opponentTeamSize(9) == 6);

    SaveBattleRules loaded;
    assert(SaveBattleRules::deserialize(doubleSave.serialize(), loaded));
    assert(loaded.seed == doubleSave.seed);
    assert(loaded.preferredStyle == doubleSave.preferredStyle);
    assert(!SaveBattleRules::deserialize("not a save", loaded));

    std::cout << "Battle format tests passed.\n";
    return 0;
}
