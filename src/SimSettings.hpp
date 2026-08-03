#pragma once

#include <map>

class FluidGrid;

enum class SimType {
    Tunnel,
    Box
};

struct SimSettings {
    float gravity;
    void (FluidGrid::*createWorld)();
    void (FluidGrid::*update)();
};

extern const std::map<SimType, SimSettings> SETTINGS;