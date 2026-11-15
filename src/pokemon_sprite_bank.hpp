/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cctype>
#include <string>
#include <unordered_map>

namespace ash_horizon {

// Lazy SDL texture cache for individual Pokémon sprites and optional sprite sheets.
// Asset files are imported locally by tools/import_graphics.py; they are not
// duplicated into the source repository.
class PokemonSpriteBank {
public:
    struct SheetConfig {
        std::string path;
        int spriteWidth = 64;
        int spriteHeight = 64;
        int columns = 16;
        unsigned int firstDex = 1;
    };

    explicit PokemonSpriteBank(SDL_Renderer* renderer) : renderer_(renderer) {}
    PokemonSpriteBank(const PokemonSpriteBank&) = delete;
    PokemonSpriteBank& operator=(const PokemonSpriteBank&) = delete;

    ~PokemonSpriteBank() {
        for (auto& entry : textures_)
            if (entry.second) SDL_DestroyTexture(entry.second);
    }

    void configureSheet(const std::string& form, const std::string& view,
                        const std::string& path, int spriteWidth,
                        int spriteHeight, int columns, unsigned int firstDex = 1) {
        if (spriteWidth <= 0 || spriteHeight <= 0 || columns <= 0) return;
        const std::string key = form + "/" + view;
        eraseTexture(key);
        configs_[key] = {path, spriteWidth, spriteHeight, columns, firstDex};
    }

    bool drawPokemon(unsigned int nationalDex, const std::string& form,
                     const std::string& view, float x, float y,
                     float width, float height) {
        if (!renderer_ || nationalDex == 0 || nationalDex > 1025) return false;
        const std::string key = "sheet:" + form + "/" + view;
        SheetConfig config = defaultConfig(form, view);
        const auto configured = configs_.find(form + "/" + view);
        if (configured != configs_.end()) config = configured->second;
        if (nationalDex < config.firstDex) return false;
        const unsigned int index = nationalDex - config.firstDex;
        SDL_Texture* texture = loadTexture(key, config.path);
        if (!texture) return false;
        SDL_FRect source{
            static_cast<float>((index % static_cast<unsigned int>(config.columns)) * config.spriteWidth),
            static_cast<float>((index / static_cast<unsigned int>(config.columns)) * config.spriteHeight),
            static_cast<float>(config.spriteWidth), static_cast<float>(config.spriteHeight)};
        SDL_FRect destination{x, y, width, height};
        return SDL_RenderTexture(renderer_, texture, &source, &destination);
    }

    // Generation 9 Pack battle sprites are individual, species-named PNGs.
    bool drawPokemonByName(const std::string& speciesName, const std::string& view,
                           float x, float y, float width, float height) {
        if (view != "front" && view != "back") return false;
        const std::string filename = normalizeSpeciesName(speciesName) + ".png";
        return drawImage("assets/pokemon/battle/" + view + "/" + filename,
                         x, y, width, height);
    }

    // Downloads(1).zip overworld archive: one 32x32 image per Dex/form/direction.
    bool drawOverworldPokemon(unsigned int nationalDex, const std::string& direction,
                              float x, float y, float width, float height,
                              bool shiny = false) {
        if (nationalDex == 0 || nationalDex > 493) return false;
        if (direction != "down" && direction != "up" &&
            direction != "left" && direction != "right") return false;
        const std::string filename = std::to_string(nationalDex) + (shiny ? "s" : "") + ".png";
        return drawImage("assets/pokemon/overworld/" + direction + "/" + filename,
                         x, y, width, height);
    }

    // Generation 9 Pack follower images are named after the species.
    bool drawFollower(const std::string& speciesName, const std::string& direction,
                      float x, float y, float width, float height, bool shiny = false) {
        int row = 0;
        if (direction == "left") row = 1;
        else if (direction == "right") row = 2;
        else if (direction == "up") row = 3;
        else if (direction != "down") return false;
        const std::string folder = shiny ? "assets/pokemon/followers_shiny/" :
                                           "assets/pokemon/followers/";
        const std::string path = folder + normalizeSpeciesName(speciesName) + ".png";
        SDL_Texture* texture = loadTexture("follower:" + path, path);
        if (!texture) return false;
        const int frame = static_cast<int>((SDL_GetTicks() / 180u) % 4u);
        SDL_FRect source{static_cast<float>(frame * 64), static_cast<float>(row * 64),
                         64.0f, 64.0f};
        SDL_FRect destination{x, y, width, height};
        return SDL_RenderTexture(renderer_, texture, &source, &destination);
    }

    // Draw a frame from a horizontal sprite strip. Useful for custom move and
    // trainer animations; no animation assets are assumed to exist by default.
    bool drawAnimationStrip(const std::string& path, int frameWidth, int frameHeight,
                            int frameCount, Uint32 frameDurationMs,
                            float x, float y, float width, float height) {
        if (frameWidth <= 0 || frameHeight <= 0 || frameCount <= 0 ||
            frameDurationMs == 0) return false;
        SDL_Texture* texture = loadTexture("anim:" + path, path);
        if (!texture) return false;
        const int frame = static_cast<int>((SDL_GetTicks() / frameDurationMs) %
                                           static_cast<Uint32>(frameCount));
        SDL_FRect source{static_cast<float>(frame * frameWidth), 0.0f,
                         static_cast<float>(frameWidth), static_cast<float>(frameHeight)};
        SDL_FRect destination{x, y, width, height};
        return SDL_RenderTexture(renderer_, texture, &source, &destination);
    }

    bool drawImage(const std::string& path, float x, float y,
                   float width, float height) {
        if (!renderer_) return false;
        SDL_Texture* texture = loadTexture("file:" + path, path);
        if (!texture) return false;
        SDL_FRect destination{x, y, width, height};
        return SDL_RenderTexture(renderer_, texture, nullptr, &destination);
    }

private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, SDL_Texture*> textures_;
    std::unordered_map<std::string, SheetConfig> configs_;

    void eraseTexture(const std::string& key) {
        const auto old = textures_.find(key);
        if (old == textures_.end()) return;
        if (old->second) SDL_DestroyTexture(old->second);
        textures_.erase(old);
    }

    static std::string normalizeSpeciesName(const std::string& name) {
        std::string result;
        for (unsigned char ch : name) {
            if (std::isalnum(ch)) result.push_back(static_cast<char>(std::toupper(ch)));
            else if (ch == ' ' || ch == '-' || ch == '.') result.push_back('_');
        }
        return result;
    }

    static SheetConfig defaultConfig(const std::string& form, const std::string& view) {
        SheetConfig config;
        config.path = "assets/pokemon/sheets/" + form + "_" + view + ".png";
        return config;
    }

    SDL_Texture* loadTexture(const std::string& key, const std::string& path) {
        const auto found = textures_.find(key);
        if (found != textures_.end()) return found->second;
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            textures_.emplace(key, nullptr);
            return nullptr;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_DestroySurface(surface);
        textures_.emplace(key, texture);
        return texture;
    }
};

} // namespace ash_horizon
