#pragma once

// TODO: implement resterizer member to view Ez state
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
    for (int ix = 0; ix < NX; ix++) {
      xgrid[ix] = xmin + ix * delta;
    }

    for (int iy = 0; iy < NY; iy++) {
      ygrid[iy] = ymin + iy * delta;
    }

    zeroField();
    setUniformMedium(1.0, 1.0);
  }

  int getNX() const { return NX; }
  int getNY() const { return NY; }
  int size() const { return NX * NY; }

  double getDelta() const { return xgrid[1] - xgrid[0]; }
  double getTimestep() const { return (getDelta() * courant_factor / vacuum_velocity); }

  double getXmin() const { return xgrid[0]; }
  double getXmax() const { return xgrid[NX - 1]; }
  double getYmin() const { return ygrid[0]; }
  double getYmax() const { return ygrid[NY - 1]; }

  void zeroField() {
    std::memset(Hx, 0, NX * NY * sizeof(double));
    std::memset(Hy, 0, NX * NY * sizeof(double));
    std::memset(Ez, 0, NX * NY * sizeof(double));
  }

  // assumes sigma = sigmam = 0.0 (TODO: generalize)
  void setUniformMedium(double mur, 
                        double epr)
  {
    for (int ix = 0; ix < NX; ix++) {
      for (int iy = 0; iy < NY; iy++) {
        const int idx = index(ix, iy);

        chxh[idx] = 1.0;
        chxe[idx] = courant_factor / (mur * vacuum_impedance);

        chyh[idx] = 1.0;
        chye[idx] = courant_factor / (mur * vacuum_impedance);

        cezh[idx] = vacuum_impedance * courant_factor / epr;
        ceze[idx] = 1.0;
      }
    }
  }

  void updateHxHy() {
    for (int ix = 0; ix < NX; ix++) {
      for (int iy = 0; iy < NY - 1; iy++) {
        const int idx = index(ix, iy);
        Hx[idx] = chxh[idx] * Hx[idx] - chxe[idx] * (Ez[index(ix, iy + 1)] - Ez[idx]);
      }
    }

    for (int ix = 0; ix < NX - 1; ix++) {
      for (int iy = 0; iy < NY; iy++) {
        const int idx = index(ix, iy);
        Hy[idx] = chyh[idx] * Hy[idx] + chye[idx] * (Ez[index(ix + 1, iy)] - Ez[idx]);
      }
    }
  }

  void updateEz() {
    for (int ix = 1; ix < NX - 1; ix++) {
      for (int iy = 1; iy < NY - 1; iy++) {
        const int idx = index(ix, iy);
        const double dxhy = Hy[idx] - Hy[index(ix - 1, iy)];
        const double dyhx = Hx[idx] - Hx[index(ix, iy - 1)];
        Ez[idx] = ceze[idx] * Ez[idx] - cezh[idx] * (dxhy - dyhx);
      }
    }
  }

  void update() {
    updateHxHy();
    updateEz();
  }

  void rasterizeEz(uint32_t* imgdata, 
                   int w, 
                   int h) const
  {
    // TODO: render the Ez field to pixel data using bilinear interpolation
    // (a viewport is rendered onto the image; not necessary to rasterize the full field; zoom in possible)
    // default is full xrange onto 0..w-1, full yrange onto 0..h-1 but reversed so that pos. axis is upwards
  }

  double minimumEz() const {
    double val = Ez[0];
    for (int i = 1; i < size(); i++) {
      if (Ez[i] < val) val = Ez[i];
    }
    return val;
  }

  double maximumEz() const {
    double val = Ez[0];
    for (int i = 1; i < size(); i++) {
      if (Ez[i] > val) val = Ez[i];
    }
    return val;
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

  // update coefficient arrays
  double chxh[NX * NY];
  double chxe[NX * NY];

  double chyh[NX * NY];
  double chye[NX * NY];

  double ceze[NX * NY];
  double cezh[NX * NY];

  int index(int ix, int iy) const {
    return NX * iy + ix;
  }
};

}
