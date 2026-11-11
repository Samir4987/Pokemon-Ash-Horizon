/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#include "../src/pokemon_data.hpp"
#include <cassert>
#include <sstream>
#include <string>

int main() {
    using namespace ash_horizon;
    PokemonDatabase db;
    std::istringstream source(
        "ASHDEX\t1\n"
        "1\tBulbasaur\tgrass\tpoison\t45\t49\t49\t65\t65\t45\t1\n"
        "2\tIvysaur\tgrass\tpoison\t60\t62\t63\t80\t80\t60\t1\n"
        "3\tVenusaur\tgrass\tpoison\t80\t82\t83\t100\t100\t80\t1\n");
    std::string error;
    assert(db.loadSpeciesTsv(source, &error));
    assert(db.species().size() == 3);
    assert(db.findSpecies(1) != nullptr);
    assert(db.findSpecies(1)->baseStats.hp == 45);
    assert(db.findSpecies(1)->generation == 1);
    assert(db.findSpecies(99) == nullptr);

    FormRecord regional;
    regional.speciesDex = 1;
    regional.formId = "example";
    regional.displayName = "Bulbasaur Example Form";
    regional.region = "Example";
    regional.type1 = PokemonType::Grass;
    assert(db.addForm(regional));
    assert(db.findForm(1, "example") != nullptr);
    assert(!db.addForm(regional));

    EvolutionRecord evolution;
    evolution.fromDex = 1;
    evolution.toDex = 2;
    evolution.method = EvolutionMethod::Level;
    evolution.minimumLevel = 16;
    assert(db.addEvolution(evolution));
    assert(!db.addEvolution(evolution));
    EvolutionContext context;
    context.level = 15;
    assert(!db.canEvolve(evolution, context));
    context.level = 16;
    assert(db.canEvolve(evolution, context));

    EvolutionRecord friendship;
    friendship.fromDex = 2;
    friendship.toDex = 3;
    friendship.method = EvolutionMethod::Friendship;
    assert(db.addEvolution(friendship));
    context.level = 20;
    context.friendship = 159;
    assert(!db.canEvolve(friendship, context));
    context.friendship = 160;
    assert(db.canEvolve(friendship, context));

    PokemonDatabase malformed;
    std::istringstream bad("ASHDEX\t1\n1\tMissing\tgrass\tpoison\t45\t49\t49\t65\t65\n");
    assert(!malformed.loadSpeciesTsv(bad, &error));
    assert(!error.empty());

    PokemonDatabase duplicate;
    std::istringstream repeated(
        "ASHDEX\t1\n"
        "1\tBulbasaur\tgrass\tpoison\t45\t49\t49\t65\t65\t45\n"
        "1\tDuplicate\tgrass\tpoison\t45\t49\t49\t65\t65\t45\n");
    assert(!duplicate.loadSpeciesTsv(repeated, &error));
    return 0;
}
