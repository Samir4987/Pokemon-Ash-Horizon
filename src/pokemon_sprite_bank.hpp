#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cstdio>
#include <string>
#include <unordered_map>

namespace ash_horizon {

// Sprite sheets keep many Pokemon sprites in one image.
// Default layout: 64x64 cells, 16 columns, National Dex #1 at cell zero.
// Sheets are loaded only when first needed to keep memory use low.
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
        for (auto& entry : textures_) {
            if (entry.second) SDL_DestroyTexture(entry.second);
        }
    }

    // Optional: register a custom sheet layout before requesting its sprites.
    void configureSheet(const std::string& form, const std::string& view,
                        const std::string& path, int spriteWidth,
                        int spriteHeight, int columns,
                        unsigned int firstDex = 1) {
        if (spriteWidth <= 0 || spriteHeight <= 0 || columns <= 0) return;

        const std::string key = form + "/" + view;
        const auto old = textures_.find(key);
        if (old != textures_.end()) {
            if (old->second) SDL_DestroyTexture(old->second);
            textures_.erase(old);
        }

        configs_[key] = {path, spriteWidth, spriteHeight, columns, firstDex};
    }

    bool drawPokemon(unsigned int nationalDex, const std::string& form,
                     const std::string& view, float x, float y,
                     float width, float height) {
        if (!renderer_ || nationalDex == 0 || nationalDex > 1025) return false;

        const std::string key = form + "/" + view;
        SheetConfig config = defaultConfig(form, view);
        const auto configured = configs_.find(key);
        if (configured != configs_.end()) config = configured->second;

        if (nationalDex < config.firstDex) return false;
        const unsigned int index = nationalDex - config.firstDex;
        const int column = static_cast<int>(index % static_cast<unsigned int>(config.columns));
        const int row = static_cast<int>(index / static_cast<unsigned int>(config.columns));

        SDL_Texture* texture = loadSheet(key, config);
        if (!texture) return false;

        SDL_FRect source{
            static_cast<float>(column * config.spriteWidth),
            static_cast<float>(row * config.spriteHeight),
            static_cast<float>(config.spriteWidth),
            static_cast<float>(config.spriteHeight)
        };
        SDL_FRect destination{x, y, width, height};
        return SDL_RenderTexture(renderer_, texture, &source, &destination);
    }

private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, SDL_Texture*> textures_;
    std::unordered_map<std::string, SheetConfig> configs_;

    static SheetConfig defaultConfig(const std::string& form,
                                     const std::string& view) {
        SheetConfig config;
        config.path = "assets/pokemon/sheets/" + form + "_" + view + ".png";
        return config;
    }

    SDL_Texture* loadSheet(const std::string& key, const SheetConfig& config) {
        const auto found = textures_.find(key);
        if (found != textures_.end()) return found->second;

        SDL_Surface* surface = IMG_Load(config.path.c_str());
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
