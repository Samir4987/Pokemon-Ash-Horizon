#pragma once

#include <SDL3/SDL.h>
#include <cstdio>
#include <string>
#include <unordered_map>

namespace ash_horizon {

// Loads sprites lazily so a 2 GB machine only keeps requested images in memory.
// Asset layout: assets/pokemon/0001/normal/front.bmp, back.bmp, overworld.bmp.
class PokemonSpriteBank {
public:
    explicit PokemonSpriteBank(SDL_Renderer* renderer) : renderer_(renderer) {}

    PokemonSpriteBank(const PokemonSpriteBank&) = delete;
    PokemonSpriteBank& operator=(const PokemonSpriteBank&) = delete;

    ~PokemonSpriteBank() {
        for (auto& entry : textures_) {
            if (entry.second) SDL_DestroyTexture(entry.second);
        }
    }

    SDL_Texture* get(unsigned int nationalDex, const std::string& form,
                     const std::string& view) {
        if (nationalDex == 0 || nationalDex > 1025) return nullptr;

        char number[5];
        std::snprintf(number, sizeof(number), "%04u", nationalDex);
        const std::string key = std::string(number) + "/" + form + "/" + view;
        const auto found = textures_.find(key);
        if (found != textures_.end()) return found->second;

        const std::string path = std::string("assets/pokemon/") + key + ".bmp";
        SDL_Surface* surface = SDL_LoadBMP(path.c_str());
        if (!surface) {
            // Cache missing assets too, avoiding repeated disk checks every frame.
            textures_.emplace(key, nullptr);
            return nullptr;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_DestroySurface(surface);
        textures_.emplace(key, texture);
        return texture;
    }

    void draw(SDL_Texture* texture, float x, float y, float width, float height) const {
        if (!renderer_ || !texture) return;
        SDL_FRect destination{x, y, width, height};
        SDL_RenderTexture(renderer_, texture, nullptr, &destination);
    }

private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, SDL_Texture*> textures_;
};

} // namespace ash_horizon
