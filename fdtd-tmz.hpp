#pragma once

// TODO: simplify to set delta = deltax = deltay always ?! arguments for and against ?!

namespace TMz {

const double vacuum_permeability = 1.2566370621219 * 1.0e-6; // [N / A^2]
const double vacuum_permittivity = 8.854187812813 * 1.0e-12; // [F / m]
const double vacuum_impedance = std::sqrt(vacuum_permeability / vacuum_permittivity);
const double vacuum_velocity = 1.0 / std::sqrt(vacuum_permeability * vacuum_permittivity);

template <int NX, int NY>
class fdtdSolver
{
public:
  void initialize(double xmin, 
                  double deltax,
                  double ymin, 
                  double deltay)
  {
    for (int i = 0; i < NX; i++) {
      xgrid[i] = xmin + i * deltax;
    }

    for (int i = 0; i < NY; i++) {
      ygrid[i] = ymin + i * deltay;
    }

    zero();
    setUniformVacuum();
  }

  int getNX() const { return NX; }
  int getNY() const { return NY; }

  double getDeltaX() const { return xgrid[1] - xgrid[0]; }
  double getDeltaY() const { return ygrid[1] - ygrid[0]; }

  void zero() {
    std::memset(Hx, 0, NX * NY * sizeof(double));
    std::memset(Hy, 0, NX * NY * sizeof(double));
    std::memset(Ez, 0, NX * NY * sizeof(double));
  }

  void setUniformVacuum() {
    // ...
  }

  // TODO: set stepper coefficients given deltat etc..

  void updateEz() {
    // ... (pay attention to temp buffering)
  }

  void updateHxHy() {
    // ... (pay attention to temp buffering)
  }

  void update() {
    updateEz();
    updateHxHy();
  }

private:
  double xgrid[NX];
  double ygrid[NY];

  // Grid points (as-is) are for the Ez field
  // The grid for Hx is staggered by half deltay
  // The grid for Hy is staggered by half deltax

  double Hx[NX * NY]; // at t - 0.5 * deltat
  double Hy[NX * NY]; // at t - 0.5 * deltat
  double Ez[NX * NY]; // at t
};

}
