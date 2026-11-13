/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#include <SDL3/SDL.h>
#include "living_world.hpp"
#include "battle_system.hpp"
#include "pokemon_sprite_bank.hpp"
#include "pokemon_party.hpp"
#include "pokemon_data.hpp"
#include "private_cheat_menu.hpp"
#include "save_game.hpp"
#include "gameplay_system.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>

namespace {
constexpr int SCREEN_W = 240;
constexpr int SCREEN_H = 160;
constexpr int TILE = 16;
constexpr int MAP_W = 30;
constexpr int MAP_H = 20;
constexpr float PLAYER_SPEED = 75.0f;

enum class Mode { World, Battle, Party, Bag, Profile, Starter };

struct Player {
    float x = 5.0f * TILE;
    float y = 8.0f * TILE;
    int direction = 0; // 0 down, 1 up, 2 left, 3 right
};

SDL_FRect rect(float x, float y, float w, float h) {
    return SDL_FRect{x, y, w, h};
}

void fill(SDL_Renderer* r, float x, float y, float w, float h,
          Uint8 red, Uint8 green, Uint8 blue) {
    SDL_SetRenderDrawColor(r, red, green, blue, 255);
    SDL_FRect dst = rect(x, y, w, h);
    SDL_RenderFillRect(r, &dst);
}

int tileAt(int x, int y) {
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) return 1;
    // Water along the upper-right edge.
    if (x >= 22 && y <= 4) return 2;
    // A tree-lined boundary and a few obstacles.
    if (x == 0 || y == 0 || x == MAP_W - 1 || y == MAP_H - 1) return 1;
    if ((x == 11 && y >= 3 && y <= 6) ||
        (x == 17 && y >= 11 && y <= 15) ||
        (x == 4 && y >= 14 && y <= 17)) return 1;
    // Tall grass patches.
    if ((x >= 13 && x <= 20 && y >= 5 && y <= 9) ||
        (x >= 7 && x <= 12 && y >= 12 && y <= 17)) return 3;
    return 0;
}

bool blocked(float px, float py) {
    const int left = static_cast<int>(std::floor(px / TILE));
    const int right = static_cast<int>(std::floor((px + 11.0f) / TILE));
    const int top = static_cast<int>(std::floor(py / TILE));
    const int bottom = static_cast<int>(std::floor((py + 11.0f) / TILE));
    for (int ty = top; ty <= bottom; ++ty) {
        for (int tx = left; tx <= right; ++tx) {
            if (tileAt(tx, ty) == 1 || tileAt(tx, ty) == 2) return true;
        }
    }
    return false;
}

void drawMap(SDL_Renderer* r, int cameraX, int cameraY) {
    for (int y = 0; y < MAP_H; ++y) {
        for (int x = 0; x < MAP_W; ++x) {
            const int type = tileAt(x, y);
            const float sx = static_cast<float>(x * TILE - cameraX);
            const float sy = static_cast<float>(y * TILE - cameraY);
            if (sx < -TILE || sy < -TILE || sx > SCREEN_W || sy > SCREEN_H) continue;

            if (type == 0) {
                fill(r, sx, sy, TILE, TILE, 104, 184, 88);
                fill(r, sx + 3, sy + 5, 2, 2, 90, 166, 75);
            } else if (type == 1) {
                fill(r, sx, sy, TILE, TILE, 65, 125, 65);
                fill(r, sx + 2, sy + 2, 12, 10, 35, 105, 55);
                fill(r, sx + 5, sy + 11, 6, 5, 115, 78, 48);
            } else if (type == 2) {
                fill(r, sx, sy, TILE, TILE, 65, 145, 205);
                fill(r, sx + 2, sy + 5, 8, 2, 130, 195, 230);
            } else {
                fill(r, sx, sy, TILE, TILE, 65, 155, 70);
                fill(r, sx + 3, sy + 3, 2, 8, 35, 115, 55);
                fill(r, sx + 10, sy + 6, 2, 7, 35, 115, 55);
                fill(r, sx + 5, sy + 2, 2, 4, 145, 205, 85);
            }
        }
    }
}

void drawPlayer(SDL_Renderer* r, float x, float y) {
    // Simple placeholder sprite, drawn from rectangles (no image files).
    fill(r, x + 2, y, 8, 5, 45, 45, 65);
    fill(r, x + 1, y + 5, 10, 5, 220, 175, 135);
    fill(r, x, y + 10, 12, 5, 55, 85, 190);
    fill(r, x + 1, y + 14, 4, 2, 45, 45, 55);
    fill(r, x + 7, y + 14, 4, 2, 45, 45, 55);
}

void drawWildCreature(SDL_Renderer* r, const ash_horizon::WildCreature& c,
                      int cameraX, int cameraY) {
    const float x = c.x - cameraX;
    const float y = c.y - cameraY;
    Uint8 red = 105, green = 185, blue = 95;
    if (c.species == 1) { red = 220; green = 105; blue = 65; }
    if (c.species == 2) { red = 75; green = 155; blue = 215; }

    const float bob = (c.mood == ash_horizon::WildMood::Rest)
        ? 0.0f : std::sin(c.animationTime * 8.0f) * 1.2f;
    fill(r, x + 2, y + 9 + bob, 12, 5, 45, 65, 55);
    fill(r, x + 3, y + 3 + bob, 10, 9, red, green, blue);
    fill(r, x + 5, y + 1 + bob, 6, 4, red, green, blue);
    fill(r, x + 5, y + 6 + bob, 2, 2, 250, 250, 225);
    fill(r, x + 10, y + 6 + bob, 2, 2, 250, 250, 225);
    if (c.mood == ash_horizon::WildMood::Play) {
        fill(r, x + 1, y + bob, 2, 2, 245, 225, 110);
        fill(r, x + 14, y + 2 + bob, 2, 2, 245, 225, 110);
    }
}

void drawProfileSetup(SDL_Renderer* r, const std::string& name, int gender) {
    fill(r, 0, 0, SCREEN_W, SCREEN_H, 28, 52, 72);
    fill(r, 8, 8, 224, 144, 238, 240, 220);
    fill(r, 8, 8, 224, 22, 35, 65, 82);
    SDL_RenderDebugText(r, 18, 15, "NEW TRAINER PROFILE");
    SDL_RenderDebugText(r, 18, 43, "NAME (12 CHARACTERS MAX):");
    fill(r, 18, 58, 204, 18, 255, 255, 245);
    SDL_RenderDebugText(r, 24, 63, name.empty() ? "_" : name.c_str());
    SDL_RenderDebugText(r, 18, 88, "CHOOSE YOUR TRAINER:");
    fill(r, 18, 103, 96, 20, gender == 0 ? 185 : 218, gender == 0 ? 220 : 222, gender == 0 ? 185 : 210);
    fill(r, 122, 103, 100, 20, gender == 1 ? 185 : 218, gender == 1 ? 220 : 222, gender == 1 ? 185 : 210);
    SDL_RenderDebugText(r, 43, 109, "MALE");
    SDL_RenderDebugText(r, 147, 109, "FEMALE");
    SDL_RenderDebugText(r, 18, 133, "TYPE NAME  TAB: GENDER  ENTER: OK");
}

void drawWallet(SDL_Renderer* r, const ash_horizon::PlayerProfile& profile,
                const ash_horizon::PlayerWallet& wallet) {
    fill(r, 3, 3, 126, 40, 25, 45, 60);
    SDL_RenderDebugText(r, 7, 6, profile.name().c_str());
    const std::string money = "P$ " + std::to_string(wallet.pokeDollars());
    const std::string hc = "HC " + std::to_string(wallet.hc());
    const std::string shards = "SH " + std::to_string(wallet.shards());
    SDL_RenderDebugText(r, 7, 15, money.c_str());
    SDL_RenderDebugText(r, 7, 24, hc.c_str());
    SDL_RenderDebugText(r, 7, 33, shards.c_str());
}

void drawStarter(SDL_Renderer* r, int selected) {
    fill(r, 0, 0, SCREEN_W, SCREEN_H, 28, 52, 72);
    fill(r, 8, 8, 224, 144, 238, 240, 220);
    fill(r, 8, 8, 224, 22, 35, 65, 82);
    SDL_RenderDebugText(r, 18, 15, "CHOOSE YOUR FIRST POKEMON");
    const char* names[3] = {"BULBASAUR - GRASS", "CHARMANDER - FIRE", "SQUIRTLE - WATER"};
    for (int i = 0; i < 3; ++i) {
        const int y = 45 + i * 28;
        if (i == selected) fill(r, 20, y - 4, 200, 22, 185, 220, 185);
        SDL_RenderDebugText(r, 32, y, i == selected ? ">" : " ");
        SDL_RenderDebugText(r, 48, y, names[i]);
    }
    SDL_RenderDebugText(r, 18, 132, "W/S SELECT    E CONFIRM");
}

void drawBag(SDL_Renderer* r, const ash_horizon::Inventory& bag, int selected) {
    fill(r, 0, 0, SCREEN_W, SCREEN_H, 42, 65, 83);
    fill(r, 5, 5, 230, 150, 238, 240, 220);
    fill(r, 5, 5, 230, 18, 35, 65, 82);
    SDL_RenderDebugText(r, 14, 11, "BAG - W/S SELECT, E USE, T BACK");
    const ash_horizon::ItemId items[3] = {ash_horizon::ItemId::Potion,
        ash_horizon::ItemId::SuperPotion, ash_horizon::ItemId::Revive};
    const char* names[3] = {"Potion", "Super Potion", "Revive"};
    for (int i = 0; i < 3; ++i) {
        const int y = 42 + i * 28;
        if (i == selected) fill(r, 14, y - 4, 210, 22, 190, 225, 190);
        SDL_RenderDebugText(r, 20, y, i == selected ? ">" : " ");
        SDL_RenderDebugText(r, 36, y, names[i]);
        const std::string count = "x" + std::to_string(bag.quantity(items[i]));
        SDL_RenderDebugText(r, 174, y, count.c_str());
    }
    SDL_RenderDebugText(r, 14, 133, "Use items on your active Pokemon");
}

void drawParty(SDL_Renderer* r, const ash_horizon::PokemonParty& party,
               int selectedIndex) {
    fill(r, 0, 0, SCREEN_W, SCREEN_H, 42, 65, 83);
    fill(r, 5, 5, 230, 150, 238, 240, 220);
    fill(r, 5, 5, 230, 16, 35, 65, 82);
    SDL_RenderDebugText(r, 12, 10, "POKEMON PARTY  -  W/S SELECT");
    for (std::size_t i = 0; i < ash_horizon::PokemonParty::MAX_SIZE; ++i) {
        const int y = 25 + static_cast<int>(i) * 20;
        const auto* pokemon = party.member(i);
        if (!pokemon) {
            fill(r, 12, y, 216, 17, 218, 222, 210);
            SDL_RenderDebugText(r, 22, y + 5, "-- EMPTY SLOT --");
            continue;
        }
        const bool active = i == party.activeIndex();
        fill(r, 12, y, 216, 17, active ? 190 : 218, active ? 225 : 222,
             active ? 190 : 210);
        if (static_cast<int>(i) == selectedIndex)
            SDL_RenderDebugText(r, 15, y + 5, ">");
        if (active) SDL_RenderDebugText(r, 27, y + 5, "*");
        const std::string& label = pokemon->nickname.empty()
            ? pokemon->speciesName : pokemon->nickname;
        SDL_RenderDebugText(r, active ? 39 : 29, y + 5, label.c_str());
        const std::string stats = "Lv" + std::to_string(pokemon->level) +
            "  HP " + std::to_string(pokemon->currentHp) + "/" +
            std::to_string(pokemon->maxHp);
        SDL_RenderDebugText(r, 145, y + 5, stats.c_str());
    }
    fill(r, 12, 146, 216, 8, 35, 65, 82);
    SDL_RenderDebugText(r, 17, 146, "E: SWITCH    T: BACK");
}

void drawBattle(SDL_Renderer* r, const ash_horizon::BattleEngine& battle,
                ash_horizon::PokemonSpriteBank& sprites, int enemyDex,
                const char* enemyName) {
    fill(r, 0, 0, SCREEN_W, SCREEN_H, 190, 220, 235);
    // distant hills
    fill(r, 0, 65, SCREEN_W, 35, 135, 190, 115);
    fill(r, 0, 100, SCREEN_W, 60, 95, 170, 90);
    // Platforms and sprite-backed battle creatures (temporary fallbacks if files are absent).
    fill(r, 142, 48, 72, 12, 115, 160, 105);
    if (!sprites.drawPokemon(enemyDex, "normal", "front", 157, 14, 40, 38)) {
        fill(r, 162, 25, 28, 24, 225, 125, 75);
        fill(r, 168, 31, 5, 5, 255, 245, 210);
        fill(r, 180, 31, 5, 5, 255, 245, 210);
    }
    fill(r, 18, 105, 85, 13, 115, 160, 105);
    const auto* activePokemon = battle.activePokemon();
    const int playerDex = activePokemon ? activePokemon->nationalDex : 1;
    if (!sprites.drawPokemon(playerDex, "normal", "back", 38, 67, 44, 42)) {
        fill(r, 45, 76, 30, 28, 70, 115, 205);
        fill(r, 51, 82, 5, 5, 255, 255, 255);
        fill(r, 64, 82, 5, 5, 255, 255, 255);
    }

    // Readable status panels: keep the UI inside the 240x160 GBA canvas.
    const int playerHp = battle.playerHp();
    const int enemyHp = battle.enemyHp();
    fill(r, 8, 8, 100, 31, 248, 248, 225);
    fill(r, 8, 8, 100, 2, 45, 65, 65);
    SDL_RenderDebugText(r, 14, 12, enemyName);
    SDL_RenderDebugText(r, 14, 23, "HP");
    fill(r, 32, 25, 66, 5, 70, 70, 70);
    fill(r, 32, 25, 66 * std::max(0, enemyHp) / std::max(1, battle.enemyMaxHp()),
         5, 70, 190, 90);

    fill(r, 132, 84, 100, 34, 248, 248, 225);
    fill(r, 132, 84, 100, 2, 45, 65, 65);
    const char* playerName = activePokemon ? activePokemon->speciesName.c_str() : "YOUR POKEMON";
    SDL_RenderDebugText(r, 138, 88, playerName);
    SDL_RenderDebugText(r, 138, 99, "HP");
    fill(r, 156, 101, 66, 5, 70, 70, 70);
    fill(r, 156, 101, 66 * std::max(0, playerHp) / std::max(1, battle.playerMaxHp()),
         5, 70, 190, 90);

    // Battle feedback and a real 2x2 move menu.
    fill(r, 8, 113, 224, 9, 30, 45, 75);
    SDL_RenderDebugText(r, 11, 114, battle.message());
    fill(r, 8, 123, 224, 33, 30, 45, 75);
    fill(r, 10, 125, 220, 29, 245, 245, 230);

    const auto& moves = battle.moves();
    const int selected = battle.selectedMove();
    const float moveX[4] = {22.0f, 126.0f, 22.0f, 126.0f};
    const float moveY[4] = {130.0f, 130.0f, 143.0f, 143.0f};
    for (int i = 0; i < 4; ++i) {
        if (i == selected) {
            SDL_RenderDebugText(r, moveX[i] - 10.0f, moveY[i], ">");
        }
        const std::size_t slot = static_cast<std::size_t>(i);
        const std::string moveLabel = std::string(moves[slot].name) + " " +
            std::to_string(battle.movePp(slot)) + "/" +
            std::to_string(moves[slot].maxPp);
        SDL_RenderDebugText(r, moveX[i], moveY[i], moveLabel.c_str());
    }
}

} // namespace

int main(int, char**) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;

    SDL_Window* window = SDL_CreateWindow(
        "Pokemon: Ash Horizon", SCREEN_W * 3, SCREEN_H * 3, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderLogicalPresentation(renderer, SCREEN_W, SCREEN_H,
                                     SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);

#ifdef _WIN32
    // Private builds add a native desktop Cheats menu outside the game canvas.
    ash_horizon::PrivateCheatMenu privateCheatMenu(window);
    privateCheatMenu.attach();
    bool privateCheatApplied = false;
#endif

    Player player;
    ash_horizon::LivingWorld livingWorld;
    ash_horizon::PokemonSpriteBank spriteBank(renderer);

    // First playable Pokédex slice: the three starter species.
    ash_horizon::PokemonDatabase pokemonDatabase;
    const auto addDemoSpecies = [&pokemonDatabase](
        std::uint16_t dex, const char* name, ash_horizon::PokemonType type,
        int hp, int attack, int defense, int specialAttack, int specialDefense, int speed) {
        ash_horizon::SpeciesRecord species;
        species.nationalDex = dex;
        species.name = name;
        species.type1 = type;
        species.baseStats = {
            static_cast<std::uint16_t>(hp), static_cast<std::uint16_t>(attack),
            static_cast<std::uint16_t>(defense), static_cast<std::uint16_t>(specialAttack),
            static_cast<std::uint16_t>(specialDefense), static_cast<std::uint16_t>(speed)
        };
        species.generation = 1;
        pokemonDatabase.addSpecies(species);
    };
    addDemoSpecies(1, "Bulbasaur", ash_horizon::PokemonType::Grass, 45, 49, 49, 65, 65, 45);
    addDemoSpecies(4, "Charmander", ash_horizon::PokemonType::Fire, 39, 52, 43, 60, 50, 65);
    addDemoSpecies(7, "Squirtle", ash_horizon::PokemonType::Water, 44, 48, 65, 50, 64, 43);

    ash_horizon::PokemonParty party;

    // One persistent slot for this prototype. SDL creates the per-user folder.
    char* preferencePath = SDL_GetPrefPath("FanMadeSensei", "PokemonAshHorizon");
    const std::string savePath = preferencePath
        ? std::string(preferencePath) + "save1.ahs" : "save1.ahs";
    if (preferencePath) SDL_free(preferencePath);

    ash_horizon::SaveGameData saveData;
    const bool loadedExistingSave =
        ash_horizon::SaveGameStore::load(savePath, saveData, party, pokemonDatabase);
    if (loadedExistingSave) {
        player.x = saveData.playerX;
        player.y = saveData.playerY;
    } else {
        const Uint64 seedEntropy = SDL_GetTicksNS() ^ SDL_GetPerformanceCounter();
        const std::uint32_t newSaveSeed = static_cast<std::uint32_t>(
            seedEntropy ^ (seedEntropy >> 32));
        saveData.battleRules = ash_horizon::SaveBattleRules::create(newSaveSeed);
        saveData.playerX = player.x;
        saveData.playerY = player.y;
    }
    // Persist the new save immediately so its random battle preference survives crashes.
    if (!loadedExistingSave) {
        ash_horizon::SaveGameStore::save(savePath, saveData, party);
    }
#ifdef _WIN32
    privateCheatApplied = saveData.privateSamfr2xActive;
    privateCheatMenu.setSamfr2xActivated(saveData.privateSamfr2xActive);
#endif

    Mode mode = saveData.profile.gender() == ash_horizon::PlayerGender::Unselected
        ? Mode::Profile : (party.empty() ? Mode::Starter : Mode::World);
    std::string profileName = saveData.profile.gender() == ash_horizon::PlayerGender::Unselected
        ? std::string() : saveData.profile.name();
    int selectedGender = 0;
    if (mode == Mode::Profile) SDL_StartTextInput(window);
    ash_horizon::BattleEngine battle;
    battle.setParty(&party);
    battle.setDatabase(&pokemonDatabase);
    int selectedPartyIndex = 0;
    int selectedStarter = 0;
    int selectedBag = 0;
    int selectedBall = 0;
    std::uint16_t wildDex = 1;
    std::string wildName = "Wild Pokemon";
    bool running = true;
    bool encounterReady = true;
    std::mt19937 rng(static_cast<unsigned int>(SDL_GetTicks()));
    std::uniform_int_distribution<int> encounterRoll(0, 99);
    Uint64 lastEncounter = 0;
    Uint64 lastAction = 0;
    Uint64 previous = SDL_GetTicks();

    while (running) {
        const Uint64 now = SDL_GetTicks();
        const float dt = std::min(0.05f, static_cast<float>(now - previous) / 1000.0f);
        previous = now;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_TEXT_INPUT && mode == Mode::Profile) {
                for (const char* ch = event.text.text; *ch; ++ch) {
                    const unsigned char value = static_cast<unsigned char>(*ch);
                    const bool allowed = (value >= 'A' && value <= 'Z') ||
                        (value >= 'a' && value <= 'z') ||
                        (value >= '0' && value <= '9') || value == ' ' ||
                        value == '-' || value == '_';
                    if (allowed && profileName.size() < ash_horizon::PlayerProfile::MAX_NAME_LENGTH)
                        profileName.push_back(static_cast<char>(value));
                }
            }
            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                const SDL_Scancode key = event.key.scancode;

                if (mode == Mode::Profile && now - lastAction > 130) {
                    if (key == SDL_SCANCODE_BACKSPACE && !profileName.empty()) {
                        profileName.pop_back();
                        lastAction = now;
                    } else if (key == SDL_SCANCODE_TAB) {
                        selectedGender = 1 - selectedGender;
                        lastAction = now;
                    } else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER) {
                        const auto gender = selectedGender == 0
                            ? ash_horizon::PlayerGender::Male
                            : ash_horizon::PlayerGender::Female;
                        if (saveData.profile.setName(profileName) &&
                            saveData.profile.setGender(gender) &&
                            ash_horizon::SaveGameStore::save(savePath, saveData, party)) {
                            SDL_StopTextInput(window);
                            mode = party.empty() ? Mode::Starter : Mode::World;
                        }
                        lastAction = now;
                    }
                } else if (mode == Mode::Starter && now - lastAction > 130) {
                    if (key == SDL_SCANCODE_W) { selectedStarter = (selectedStarter + 2) % 3; lastAction = now; }
                    else if (key == SDL_SCANCODE_S) { selectedStarter = (selectedStarter + 1) % 3; lastAction = now; }
                    else if (key == SDL_SCANCODE_E) {
                        if (ash_horizon::StarterSelection::choose(static_cast<std::size_t>(selectedStarter), pokemonDatabase, party)) {
                            saveData.inventory.add(ash_horizon::ItemId::PokeBall, 5);
                            saveData.inventory.add(ash_horizon::ItemId::Potion, 3);
                            saveData.playerX = player.x; saveData.playerY = player.y;
                            ash_horizon::SaveGameStore::save(savePath, saveData, party);
                            mode = Mode::World;
                        }
                        lastAction = now;
                    }
                } else if (mode == Mode::World && key == SDL_SCANCODE_I) {
                    mode = Mode::Bag;
                    selectedBag = 0;
                } else if (mode == Mode::World && key == SDL_SCANCODE_T) {
                    mode = Mode::Party;
                    selectedPartyIndex = static_cast<int>(party.activeIndex());
                } else if (mode == Mode::Bag && now - lastAction > 130) {
                    if (key == SDL_SCANCODE_T) { mode = Mode::World; lastAction = now; }
                    else if (key == SDL_SCANCODE_W) { selectedBag = (selectedBag + 2) % 3; lastAction = now; }
                    else if (key == SDL_SCANCODE_S) { selectedBag = (selectedBag + 1) % 3; lastAction = now; }
                    else if (key == SDL_SCANCODE_E) {
                        const ash_horizon::ItemId items[3] = {ash_horizon::ItemId::Potion,
                            ash_horizon::ItemId::SuperPotion, ash_horizon::ItemId::Revive};
                        if (party.active()) {
                            if (selectedBag == 2)
                                ash_horizon::ItemUseSystem::useRevive(items[selectedBag], saveData.inventory, party, party.activeIndex());
                            else
                                ash_horizon::ItemUseSystem::useHealingItem(items[selectedBag], saveData.inventory, party, party.activeIndex());
                        }
                        lastAction = now;
                    }
                } else if (mode == Mode::Party && now - lastAction > 130) {
                    if (key == SDL_SCANCODE_T) {
                        mode = Mode::World;
                        lastAction = now;
                    } else if (key == SDL_SCANCODE_W) {
                        selectedPartyIndex = std::max(0, selectedPartyIndex - 1);
                        lastAction = now;
                    } else if (key == SDL_SCANCODE_S) {
                        selectedPartyIndex = std::min(
                            static_cast<int>(ash_horizon::PokemonParty::MAX_SIZE) - 1,
                            selectedPartyIndex + 1);
                        lastAction = now;
                    } else if (key == SDL_SCANCODE_E) {
                        party.switchTo(static_cast<std::size_t>(selectedPartyIndex));
                        lastAction = now;
                    }
                } else if (mode == Mode::Battle && now - lastAction > 130) {
                    if (key == SDL_SCANCODE_W) { battle.moveSelection(-2); lastAction = now; }
                    else if (key == SDL_SCANCODE_S) { battle.moveSelection(2); lastAction = now; }
                    else if (key == SDL_SCANCODE_A) { battle.moveSelection(-1); lastAction = now; }
                    else if (key == SDL_SCANCODE_D) { battle.moveSelection(1); lastAction = now; }
                    else if (key == SDL_SCANCODE_T && battle.phase() == ash_horizon::BattlePhase::PlayerChoice) {
                        for (std::size_t offset = 1; offset <= party.size(); ++offset) {
                            const std::size_t candidate = (party.activeIndex() + offset) % party.size();
                            if (party.switchTo(candidate)) { battle.switchTurn(); break; }
                        }
                        lastAction = now;
                    }
                    else if (key == SDL_SCANCODE_Z && battle.phase() == ash_horizon::BattlePhase::PlayerChoice) {
                        selectedBall = (selectedBall + 1) % 5;
                        lastAction = now;
                    }
                    else if (key == SDL_SCANCODE_G && battle.phase() == ash_horizon::BattlePhase::PlayerChoice) {
                        battle.runAway();
                        lastAction = now;
                    }
                    else if (key == SDL_SCANCODE_F && battle.phase() == ash_horizon::BattlePhase::PlayerChoice) {
                        lastAction = now;
                        const ash_horizon::ItemId captureBalls[5] = {
                            ash_horizon::ItemId::PokeBall, ash_horizon::ItemId::GreatBall,
                            ash_horizon::ItemId::UltraBall, ash_horizon::ItemId::MasterBall,
                            ash_horizon::ItemId::ArceusBall};
                        const auto result = ash_horizon::CaptureSystem::attempt(
                            captureBalls[selectedBall], saveData.inventory, saveData.arceusBall,
                            party, pokemonDatabase, wildDex, battle.enemyHp(),
                            battle.enemyMaxHp(), encounterRoll(rng));
                        if (result == ash_horizon::CaptureResult::Caught)
                            battle.captureSucceeded("Pokemon caught! Press E.");
                        else if (result == ash_horizon::CaptureResult::Failed)
                            battle.captureFailed();
                        else if (result == ash_horizon::CaptureResult::NoBall)
                            battle.captureUnavailable("No Poke Balls available.");
                        else if (result == ash_horizon::CaptureResult::PartyFull)
                            battle.captureUnavailable("Party is full!");
                        else if (result == ash_horizon::CaptureResult::TimerExpired)
                            battle.captureUnavailable("Arceus Ball timer expired.");
                    }
                    else if (key == SDL_SCANCODE_E) {
                        lastAction = now;
                        if (battle.phase() == ash_horizon::BattlePhase::Finished) {
                            if (battle.enemyHp() == 0 && party.active())
                                ash_horizon::ProgressionSystem::award(party, party.activeIndex(), pokemonDatabase, 35);
                            battle.reset();
                            mode = Mode::World;
                            lastEncounter = now;
                            ash_horizon::SaveGameStore::save(savePath, saveData, party);
                        } else battle.chooseMove();
                    }
                }            }
        }

#ifdef _WIN32
        // Apply the private code once; its effects are saved with this save slot.
        if (!privateCheatApplied && privateCheatMenu.samfr2xActivated()) {
            saveData.inventory.grantEverything(ash_horizon::Inventory::MAX_STACK);
            saveData.arceusBall.unlimitedTime = true;
            saveData.privateSamfr2xActive = true;
            privateCheatApplied = true;
            ash_horizon::SaveGameStore::save(savePath, saveData, party);
        }
#endif

        if (mode == Mode::World) {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            float dx = 0.0f, dy = 0.0f;
            if (keys[SDL_SCANCODE_A]) { dx -= 1; player.direction = 2; }
            if (keys[SDL_SCANCODE_D]) { dx += 1; player.direction = 3; }
            if (keys[SDL_SCANCODE_W]) { dy -= 1; player.direction = 1; }
            if (keys[SDL_SCANCODE_S]) { dy += 1; player.direction = 0; }

            if (dx != 0.0f || dy != 0.0f) {
                const float length = std::sqrt(dx * dx + dy * dy);
                dx = dx / length * PLAYER_SPEED * dt;
                dy = dy / length * PLAYER_SPEED * dt;

                if (!blocked(player.x + dx, player.y)) player.x += dx;
                if (!blocked(player.x, player.y + dy)) player.y += dy;

                const int tx = static_cast<int>((player.x + 6) / TILE);
                const int ty = static_cast<int>((player.y + 6) / TILE);
                if (tileAt(tx, ty) == 3 && encounterReady && now - lastEncounter > 900) {
                    if (encounterRoll(rng) < 2 && party.usableCount() > 0) {
                        mode = Mode::Battle;
                        const int wildPick = encounterRoll(rng) % 3;
                        wildDex = ash_horizon::StarterSelection::OPTIONS[wildPick].dex;
                        const auto* wildSpecies = pokemonDatabase.findSpecies(wildDex);
                        if (wildSpecies) {
                            wildName = wildSpecies->name;
                            battle.setEnemySpecies(wildDex, 5);
                        }
                        battle.reset();
                        encounterReady = false;
                    }
                }
            }
            if (now - lastEncounter > 1200) encounterReady = true;
        }

        if (mode == Mode::Battle) {
            battle.update(dt);
            saveData.arceusBall.update(static_cast<std::uint32_t>(dt * 1000.0f));
        }

        if (mode == Mode::Profile) {
            drawProfileSetup(renderer, profileName, selectedGender);
        } else if (mode == Mode::Starter) {
            drawStarter(renderer, selectedStarter);
        } else if (mode == Mode::World) {
            livingWorld.update(dt, player.x, player.y, blocked);
            const int cameraX = std::clamp(static_cast<int>(player.x) - SCREEN_W / 2,
                                           0, MAP_W * TILE - SCREEN_W);
            const int cameraY = std::clamp(static_cast<int>(player.y) - SCREEN_H / 2,
                                           0, MAP_H * TILE - SCREEN_H);
            drawMap(renderer, cameraX, cameraY);
            for (const auto& creature : livingWorld.creatures()) {
                drawWildCreature(renderer, creature, cameraX, cameraY);
            }
            drawPlayer(renderer, player.x - cameraX, player.y - cameraY);
            drawWallet(renderer, saveData.profile, saveData.wallet);
        } else if (mode == Mode::Battle) {
            drawBattle(renderer, battle, spriteBank, wildDex, wildName.c_str());
            battle.drawEffects(renderer);
        } else if (mode == Mode::Party) {
            drawParty(renderer, party, selectedPartyIndex);
        } else if (mode == Mode::Bag) {
            drawBag(renderer, saveData.inventory, selectedBag);
        }

        SDL_RenderPresent(renderer);
    }

    saveData.playerX = player.x;
    saveData.playerY = player.y;
    const bool saveWritten = ash_horizon::SaveGameStore::save(savePath, saveData, party);
    (void)saveWritten;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
