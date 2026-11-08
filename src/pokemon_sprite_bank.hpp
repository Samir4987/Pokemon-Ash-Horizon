#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <cstdio>
#include <string>
#include <unordered_map>

namespace ash_horizon {

// Loads sprites lazily to keep memory use modest on low-end PCs.
// Preferred layout: assets/pokemon/0001/normal/front.<supported-image-extension>
// Supported formats depend on the SDL3_image build; common formats include
// PNG, BMP, JPEG, GIF, WebP, TGA, TIFF, QOI, and several others.
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
        if (!renderer_ || nationalDex == 0 || nationalDex > 1025) return nullptr;

        char number[5];
        std::snprintf(number, sizeof(number), "%04u", nationalDex);
        const std::string key = std::string(number) + "/" + form + "/" + view;
        const auto found = textures_.find(key);
        if (found != textures_.end()) return found->second;

        // Try common sprite formats in a predictable order. BMP remains supported
        // for backwards compatibility with the original asset layout.
        static const char* extensions[] = {
            ".png", ".bmp", ".webp", ".gif", ".jpg", ".jpeg",
            ".tga", ".qoi", ".tif", ".tiff", ".avif", ".jxl",
            ".svg", ".pnm", ".pcx", ".lbm", ".xpm", ".xcf", ".xv"
        };

        SDL_Texture* texture = nullptr;
        for (const char* extension : extensions) {
            const std::string path =
                std::string("assets/pokemon/") + key + extension;
            SDL_Surface* surface = IMG_Load(path.c_str());
            if (!surface) continue;

            texture = SDL_CreateTextureFromSurface(renderer_, surface);
            SDL_DestroySurface(surface);
            if (texture) break;
        }

        // Cache missing assets too, avoiding repeated disk checks every frame.
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
