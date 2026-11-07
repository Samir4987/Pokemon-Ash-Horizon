#include <SDL3/SDL.h>
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

enum class Mode { World, Battle };

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

void drawBattle(SDL_Renderer* r, int hp, int enemyHp, int selected) {
    fill(r, 0, 0, SCREEN_W, SCREEN_H, 190, 220, 235);
    // distant hills
    fill(r, 0, 65, SCREEN_W, 35, 135, 190, 115);
    fill(r, 0, 100, SCREEN_W, 60, 95, 170, 90);
    // enemy platform and creature placeholder
    fill(r, 142, 48, 72, 12, 115, 160, 105);
    fill(r, 162, 25, 28, 24, 225, 125, 75);
    fill(r, 168, 31, 5, 5, 255, 245, 210);
    fill(r, 180, 31, 5, 5, 255, 245, 210);
    // player platform and creature placeholder
    fill(r, 18, 105, 85, 13, 115, 160, 105);
    fill(r, 45, 76, 30, 28, 70, 115, 205);
    fill(r, 51, 82, 5, 5, 255, 255, 255);
    fill(r, 64, 82, 5, 5, 255, 255, 255);

    // UI panels / HP bars
    fill(r, 8, 8, 94, 30, 248, 248, 225);
    fill(r, 8, 8, 94, 2, 45, 65, 65);
    fill(r, 142, 88, 90, 28, 248, 248, 225);
    fill(r, 150, 100, 72, 5, 70, 70, 70);
    fill(r, 150, 100, 72 * std::max(0, enemyHp) / 20, 5, 70, 190, 90);
    fill(r, 16, 23, 72, 5, 70, 70, 70);
    fill(r, 16, 23, 72 * std::max(0, hp) / 20, 5, 70, 190, 90);

    fill(r, 8, 123, 224, 33, 30, 45, 75);
    fill(r, 10, 125, 220, 29, 245, 245, 230);
    // Menu cursor and two selectable rows.
    fill(r, 20, selected == 0 ? 133 : 145, 5, 5, 40, 60, 80);
    fill(r, 34, 133, 65, 4, 55, 75, 95);
    fill(r, 34, 145, 65, 4, 55, 75, 95);
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

    Player player;
    Mode mode = Mode::World;
    int playerHp = 20;
    int enemyHp = 20;
    int menuSelection = 0;
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
            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                const SDL_Keycode key = event.key.key;
                if (key == SDLK_ESCAPE) running = false;

                if (mode == Mode::Battle && now - lastAction > 160) {
                    if (key == SDLK_UP || key == SDLK_DOWN) {
                        menuSelection = 1 - menuSelection;
                    } else if (key == SDLK_RETURN || key == SDLK_SPACE) {
                        lastAction = now;
                        if (menuSelection == 0) {
                            enemyHp = std::max(0, enemyHp - 5);
                            if (enemyHp > 0) playerHp = std::max(0, playerHp - 3);
                        } else {
                            mode = Mode::World;
                            enemyHp = 20;
                            lastEncounter = now;
                        }
                        if (enemyHp <= 0) {
                            mode = Mode::World;
                            enemyHp = 20;
                            playerHp = 20;
                            lastEncounter = now;
                        }
                        if (playerHp <= 0) {
                            playerHp = 20;
                            enemyHp = 20;
                            mode = Mode::World;
                            lastEncounter = now;
                        }
                    }
                }
            }
        }

        if (mode == Mode::World) {
            const bool* keys = SDL_GetKeyboardState(nullptr);
            float dx = 0.0f, dy = 0.0f;
            if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) { dx -= 1; player.direction = 2; }
            if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) { dx += 1; player.direction = 3; }
            if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) { dy -= 1; player.direction = 1; }
            if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) { dy += 1; player.direction = 0; }

            if (dx != 0.0f || dy != 0.0f) {
                const float length = std::sqrt(dx * dx + dy * dy);
                dx = dx / length * PLAYER_SPEED * dt;
                dy = dy / length * PLAYER_SPEED * dt;

                if (!blocked(player.x + dx, player.y)) player.x += dx;
                if (!blocked(player.x, player.y + dy)) player.y += dy;

                const int tx = static_cast<int>((player.x + 6) / TILE);
                const int ty = static_cast<int>((player.y + 6) / TILE);
                if (tileAt(tx, ty) == 3 && encounterReady && now - lastEncounter > 900) {
                    if (encounterRoll(rng) < 2) {
                        mode = Mode::Battle;
                        menuSelection = 0;
                        encounterReady = false;
                    }
                }
            }
            if (now - lastEncounter > 1200) encounterReady = true;
        }

        if (mode == Mode::World) {
            const int cameraX = std::clamp(static_cast<int>(player.x) - SCREEN_W / 2,
                                           0, MAP_W * TILE - SCREEN_W);
            const int cameraY = std::clamp(static_cast<int>(player.y) - SCREEN_H / 2,
                                           0, MAP_H * TILE - SCREEN_H);
            drawMap(renderer, cameraX, cameraY);
            drawPlayer(renderer, player.x - cameraX, player.y - cameraY);
        } else {
            drawBattle(renderer, playerHp, enemyHp, menuSelection);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
