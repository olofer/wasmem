#pragma once

// TODO: implement the *basic stepper* without any boundary treatment; once that works, branch!
// TODO: utility based on points-per-wavelength, to determine delta, deltat etc.. Courant number
// TODO: bring in a viridis colormap array 256*3 ints; ...

namespace TMz {

const double vacuum_permeability = 1.2566370621219 * 1.0e-6; // [N / A^2]
const double vacuum_permittivity = 8.854187812813 * 1.0e-12; // [F / m]
const double vacuum_impedance = std::sqrt(vacuum_permeability / vacuum_permittivity);
const double vacuum_velocity = 1.0 / std::sqrt(vacuum_permeability * vacuum_permittivity);
const double courant_factor = 1.0 / std::sqrt(2.0);

template <int NX, int NY>
class fdtdSolver
{
public:
  void initialize(double xmin, 
                  double ymin, 
                  double delta)
  {
    for (int i = 0; i < NX; i++) {
      xgrid[i] = xmin + i * delta;
    }

    for (int i = 0; i < NY; i++) {
      ygrid[i] = ymin + i * delta;
    }

    zero();
    setUniformVacuum();
  }

  int getNX() const { return NX; }
  int getNY() const { return NY; }

  double getDelta() const { return xgrid[1] - xgrid[0]; }
  double getTimestep() const { return (getDelta() * courant_factor / vacuum_velocity); }

  double getXmin() const { return xgrid[0]; }
  double getXmax() const { return xgrid[NX - 1]; }
  double getYmin() const { return ygrid[0]; }
  double getYmax() const { return ygrid[NY - 1]; }

  void zero() {
    std::memset(Hx, 0, NX * NY * sizeof(double));
    std::memset(Hy, 0, NX * NY * sizeof(double));
    std::memset(Ez, 0, NX * NY * sizeof(double));
  }

  void setUniformVacuum() {
    // ...
    // TODO: zero conductivities, vacuum perms
    // ...
  }

  void updateHxHy() {
    // ... (pay attention to temp buffering)
  }

  void updateEz() {
    // ... (pay attention to temp buffering)
  }

  void update() {
    updateHxHy();
    updateEz();
  }

  void rasterize(uint32_t* imgdata, 
                 int w, 
                 int h) const
  {
    // TODO: render the Ez field to pixel data using bilinear interpolation
    // (a viewport is rendered onto the image; not necessary to rasterize the full field; zoom in possible)
    // default is full xrange onto 0..w-1, full yrange onto 0..h-1 but reversed so that pos. axis is upwards
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

  int index(int ix, int iy) const {
    return NX * iy + ix;
  }
};

}
