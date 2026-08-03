#include "SimSettings.hpp"
#include "FluidGrid.hpp"

const std::map<SimType, SimSettings> SETTINGS{
    {SimType::Tunnel, {0.0f, &FluidGrid::createTunnel, &FluidGrid::updateTunnel}},
    {SimType::Box, {-9.81f, &FluidGrid::createBox, &FluidGrid::updateBox}}};