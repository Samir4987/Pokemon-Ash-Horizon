/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#include "../src/pokemon_party.hpp"
#include <cassert>

int main() {
    using namespace ash_horizon;
    PokemonDatabase database;
    SpeciesRecord bulbasaur;
    bulbasaur.nationalDex = 1;
    bulbasaur.name = "Bulbasaur";
    bulbasaur.type1 = PokemonType::Grass;
    bulbasaur.type2 = PokemonType::Poison;
    bulbasaur.baseStats.hp = 45;
    assert(database.addSpecies(bulbasaur));

    SpeciesRecord charmander = bulbasaur;
    charmander.nationalDex = 4;
    charmander.name = "Charmander";
    charmander.type1 = PokemonType::Fire;
    charmander.type2 = PokemonType::None;
    charmander.baseStats.hp = 39;
    assert(database.addSpecies(charmander));

    PokemonParty party;
    assert(party.empty());
    assert(!party.add(database, 999, 5));
    assert(!party.add(database, 1, 0));
    assert(party.add(database, 1, 5, "Leaf"));
    assert(party.add(database, 4, 5));
    assert(party.size() == 2);
    assert(party.active() != nullptr);
    assert(party.active()->nationalDex == 1);
    assert(party.active()->nickname == "Leaf");
    assert(party.active()->level == 5);
    assert(party.active()->maxHp == 19);
    assert(party.active()->currentHp == 19);
    assert(party.battleStats(0, database).hp == 19);
    const std::array<std::uint8_t, 6> ivs{{31, 31, 31, 31, 31, 31}};
    const std::array<std::uint16_t, 6> evs{{0, 252, 0, 0, 0, 252}};
    assert(party.restoreGenetics(0, ivs, evs, static_cast<std::uint8_t>(NatureId::Jolly)));
    assert(party.member(0)->ivs == ivs);
    assert(party.member(0)->evs == evs);
    assert(party.member(0)->nature == NatureId::Jolly);
    const BattleStats calculated = party.battleStats(0, database);
    assert(calculated.hp == 19 && calculated.attack > 0 && calculated.speed > 0);
    assert(!party.restoreGenetics(0, {{32, 0, 0, 0, 0, 0}}, evs, 0));
    assert(!party.restoreGenetics(0, ivs, {{252, 252, 7, 0, 0, 0}}, 0));

    assert(!party.switchTo(0));
    assert(party.switchTo(1));
    assert(party.activeIndex() == 1);
    assert(party.active()->nationalDex == 4);

    party.damageActive(7);
    assert(party.active()->currentHp == party.active()->maxHp - 7);
    party.damageActive(999);
    assert(party.active()->currentHp == 0);
    assert(party.active()->fainted());
    assert(!party.switchTo(1));
    assert(party.switchTo(0));
    party.damageActive(-5);
    assert(party.active()->currentHp == party.active()->maxHp);
    party.damageActive(3);
    party.healAll();
    assert(party.member(0)->currentHp == party.member(0)->maxHp);
    assert(party.member(1)->currentHp == party.member(1)->maxHp);

    for (int i = 0; i < 4; ++i) assert(party.add(database, 1, 5));
    assert(party.size() == PokemonParty::MAX_SIZE);
    assert(!party.add(database, 1, 5));
    assert(party.member(6) == nullptr);
    return 0;
}
