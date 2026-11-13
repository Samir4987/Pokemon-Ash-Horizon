/*
 * Pokémon: Ash Horizon
 * Copyright (C) 2026 Pokémon: Ash Horizon contributors
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 only. See the LICENSE file for details.
 */

#pragma once

#include <algorithm>

namespace ash_horizon {

// Friendship is stored on a 0-100 scale. Keep this rule independent from SDL
// and the battle renderer so it can be tested on every supported build.
inline constexpr int attackRefusalChance(int friendship) {
    const int boundedFriendship = std::clamp(friendship, 0, 100);
    if (boundedFriendship > 84) return 0;
    if (boundedFriendship >= 40) return 10;
    return 60;
}

} // namespace ash_horizon
