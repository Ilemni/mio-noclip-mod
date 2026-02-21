#pragma once

#include <string>
#include <vector>

#include "modding_api.h"

typedef struct teleport_location {
    const std::string name;
    f32x3 location;
} teleport_location;

inline std::vector<teleport_location> teleportLocations;

void ReadLocations();
void PrevTeleport(f32x3* loc);
void NextTeleport(f32x3* loc);