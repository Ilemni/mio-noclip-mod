#include "teleport_list.h"

#include <filesystem>
#include <fstream>
#include <sstream>

const auto location_config_dir = "modconfig/noclip/";
const auto location_config_path = "modconfig/noclip/locations.txt";

size_t teleportIndex;

static void Trim(std::string &str) {
    const size_t start = str.find_first_not_of(' ');
    const size_t end = str.find_last_not_of(' ');

    if (start == std::string::npos)
        str.clear();  // String contains only whitespace
    else
        str = str.substr(start, end - start + 1);
}

void CreateLocationsFile() {
    teleportLocations.clear();

    std::filesystem::create_directories(location_config_dir);
    std::ofstream config(location_config_path);
    if (!config.is_open()) {
        LogMessage("Failed to create keybind configs");
        return;
    }

    config << "# Here you can specify a list of coordinate, and a description, to teleport to." << std::endl;
    config << "# Values are written as <x> <y> [description]" << std::endl << std::endl;

    config.close();

    LogMessage("Created default locations config");
}

void ReadLocations() {
    std::ifstream config(location_config_path);
    if (!config.is_open()) {
        LogMessage("No locations.txt found, creating defaults.");
        CreateLocationsFile();
        return;
    }

    teleportLocations.clear();

    std::string line;
    while (std::getline(config, line)) {
        Trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        float x, y;
        std::string name;
        if (!(iss >> x >> y)) {
            printf("Invalid line in locations config: %s\n", line.c_str());
            continue; // Invalid line
        }

        std::getline(iss, name);
        Trim(name);
        teleportLocations.push_back({name, make_f32x3(x, y, 0)});
        printf("Added teleport location: %s: %.2f, %.2f\n", name.c_str(), x, y);
    }
}

void Teleport(f32x3 *loc) {
    auto [name, location] = teleportLocations[teleportIndex];
    loc->x = location.x;
    loc->y = location.y;
    printf("Teleported to %s: %.2f, %.2f\n", name.c_str(), location.x, location.y);
}

void PrevTeleport(f32x3 *loc) {
    if (teleportLocations.empty()) {
        printf("No teleport locations set.\n");
        return;
    }

    if (teleportIndex == 0)
        teleportIndex = teleportLocations.size() - 1;
    else
        --teleportIndex;

    Teleport(loc);
}

void NextTeleport(f32x3 *loc) {
    if (teleportLocations.empty()) {
        printf("No teleport locations set.\n");
        return;
    }

    ++teleportIndex;
    if (teleportIndex >= teleportLocations.size())
        teleportIndex = 0;

    Teleport(loc);
}
