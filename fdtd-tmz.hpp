#pragma once

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
    resetUpdateCount();
  }

  int getNX() const { return NX; }
  int getNY() const { return NY; }
  int size() const { return NX * NY; }

  double getDelta() const { return xgrid[1] - xgrid[0]; }
  double getTimestep() const { return (getDelta() * courant_factor / vacuum_velocity); }
  int getUpdateCount() const { return updateCounter; }
  double getUpdateTime() const { return getUpdateCount() * getTimestep(); }
  void resetUpdateCount() { updateCounter = 0; }

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
        Ez[idx] = ceze[idx] * Ez[idx] + cezh[idx] * (dxhy - dyhx);
      }
    }
  }

  void updateSource(double ppw) {
    const double lambda = getDelta() * ppw;
    const double omega = (2.0 * M_PI) * vacuum_velocity / lambda;
    const double time = updateCounter * getTimestep();
    Ez[index(NX / 2, NY / 2)] = std::sin(omega * time);
  }

  void update() {
    updateHxHy();
    updateEz();
    updateSource(10.0);
    updateCounter++;
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

  // (xmin, ymin) : lower left corner (0, h - 1)
  // (xmax, ymax) : upper right corner (w - 1, 0)
  void rasterizeEz(uint32_t* imgdata, 
                   int w, 
                   int h,
                   double ezmin,
                   double ezmax,
                   double xmin,
                   double xmax,
                   double ymin,
                   double ymax) const
  {
    if (imgdata == nullptr) return;
    if (ezmin >= ezmax) return;

    const double delta = getDelta();
    const double crange = ezmax - ezmin;

    const double xupp = (xmax - xmin) / w; // x units per pixel
    const double yupp = (ymax - ymin) / h; // y units per pixel

    for (int i = 0; i < w; i++) {
      const double xi = xmin + i * xupp;
      for (int j = 0; j < h; j++) {
        const double yj = ymax - j * yupp;
        const double Ezij = interpolateEz(xi, yj, delta);
        imgdata[i + j * w] = rgb_viridis((Ezij - ezmin) / crange);
      }
    }
  }

  void rasterizeTestPattern(uint32_t* imgdata, 
                            int w, 
                            int h) const 
  {
    if (imgdata == nullptr) return;
    for (int i = 0; i < w; i++) {
      for (int j = 0; j < h; j++) {
        imgdata[i + j * w] = rgb_viridis((i + j + updateCounter) % 255);
      }
    }
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

  int updateCounter;

  int index(int ix, int iy) const {
    return NX * iy + ix;
  }

  double interpolateEz(double x, double y, double delta) const {
    const double xhat = (x - xgrid[0]) / delta;
    const double yhat = (y - ygrid[0]) / delta;
    const int xi = (int) xhat;
    const int yi = (int) yhat;
    const double etax = xhat - xi;
    const double etay = yhat - yi;

    const double v00 = Ez[index(xi, yi)];
    const double v01 = Ez[index(xi, yi + 1)];
    const double v10 = Ez[index(xi + 1, yi)];
    const double v11 = Ez[index(xi + 1, yi + 1)];

    const double w00 = (1.0 - etax) * (1.0 - etay);
    const double w01 = (1.0 - etax) * etay;
    const double w10 = etax * (1.0 - etay);
    const double w11 = etax * etay;

    return w00 * v00 + w01 * v01 + w10 * v10 + w11 * v11;
  }
};

}
