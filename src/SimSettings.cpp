#include <Solver/GaussSeidel.hpp>
#include <SimSettings.hpp>
#include <FluidGrid.hpp>

SimSettings::SimSettings(float gravity, void (FluidGrid::*createWorld)(), void (FluidGrid::*update)())
: gravity{gravity},  createWorld{createWorld}, update{update}
{
  solver = Solver::GaussSeidel();
}

const std::map<SimType, SimSettings> SETTINGS{
  {SimType::Tunnel, 
    SimSettings(0.0f, &FluidGrid::createTunnel, &FluidGrid::updateTunnel)},
  {SimType::Box, 
    SimSettings(-9.81f, &FluidGrid::createBox, &FluidGrid::updateBox)}
};