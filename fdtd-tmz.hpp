#pragma once

namespace TMz {

const double vacuum_permeability = 1.2566370621219 * 1.0e-6; // [N / A^2]
const double vacuum_permittivity = 8.854187812813 * 1.0e-12; // [F / m]
const double vacuum_impedance = std::sqrt(vacuum_permeability / vacuum_permittivity);
const double vacuum_velocity = 1.0 / std::sqrt(vacuum_permeability * vacuum_permittivity);
const double courant_factor = 1.0 / std::sqrt(2.0);

enum fdtdSourceType {
  NoSource = 0,
  Monochromatic = 1,
  RickerPulse = 2,
};

struct fdtdSource {
  fdtdSourceType type;
  double ppw;
  double x;
  double y;
  double amp;

  void initDefault() {
    type = fdtdSourceType::Monochromatic;
    ppw = 30.0;
    x = 0.05;
    y = 0.05;
    amp = 1.0;
  }

  // FIXME: the source calc should be implemented as a member of this struct also!
};

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

    setPeriodicX(true);
    setPeriodicY(true);

    setUniformMedium(1.0, 1.0, 0.0, 0.0);
    reset();

    source.initDefault();
  }

  void reset() {
    zeroField();
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

  bool isPeriodicX() const { return periodicAlongX; }
  bool isPeriodicY() const { return periodicAlongY; }

  void zeroField() {
    std::memset(Hx, 0, NX * NY * sizeof(double));
    std::memset(Hy, 0, NX * NY * sizeof(double));
    std::memset(Ez, 0, NX * NY * sizeof(double));
  }

  // NOTE: conductivities are not yet tested
  void setUniformMedium(double mur, 
                        double epr,
                        double sigmam,
                        double sigma)
  {
    relativePermeability = mur; // stash these for energy accounting (TBD)
    relativePermittivity = epr;
    magneticConductivity = sigmam;
    electricConductivity = sigma;

    const double CH = courant_factor / (mur * vacuum_impedance);
    const double CE = vacuum_impedance * courant_factor / epr;

    const double SH = (sigmam * getDelta() / 2.0) * CH;
    const double AHh = (1.0 - SH) / (1.0 + SH);
    const double AHe = 1.0 / (1.0 + SH);

    const double SE = (sigma * getDelta() / 2.0) * CE;
    const double AEh = 1.0 / (1.0 + SE);
    const double AEe = (1.0 - SE) / (1.0 + SE);

    for (int ix = 0; ix < NX; ix++) {
      for (int iy = 0; iy < NY; iy++) {
        const int idx = index(ix, iy);

        chxh[idx] = AHh;
        chxe[idx] = AHe * CH;

        chyh[idx] = AHh;
        chye[idx] = AHe * CH;

        cezh[idx] = AEh * CE;
        ceze[idx] = AEe;
      }
    }
  }

  // NOTE: approximate; may require more work to get this accurate
  double energyE() const {
    double sum = 0.0;
    for (int iy = 0; iy < NY; iy++) {
      for (int ix = 0; ix < NX; ix++) {
        const double Ezi = Ez[index(ix, iy)];
        sum += Ezi * Ezi;
      }
    }
    const double delta = getDelta();
    return relativePermittivity * vacuum_permittivity * sum * delta * delta / 2.0;
  }
/*
  // NOTE: approximate; requires more thinking to get this accurate
  double energyB() const {
    double sum = 0.0;
    for (int iy = 0; iy < NY - 1; iy++) {
      for (int ix = 0; ix < NX - 1; ix++) {
        const int idx =  index(ix, iy);
        const double Hxi = Hx[idx];
        const double Hyi = Hy[idx];
        sum += Hxi * Hxi + Hyi * Hyi;
      }
    }
    const double delta = getDelta();
    return sum * delta * delta / (2.0 * relativePermeability * vacuum_permeability);
  }
*/
  
  void update() {
    updateHxHy();
    updateEz();
    if (periodicAlongX) makeEzPeriodicX();
    if (periodicAlongY) makeEzPeriodicY();
    applySource();
    updateCounter++;
  }

  void setPeriodicX(bool onoff) {
    periodicAlongX = onoff;
    if (!periodicAlongX) {
      zeroBoundaryEzX();
    } 
  }

  void setPeriodicY(bool onoff) {
    periodicAlongY = onoff;
    if (!periodicAlongY) {
      zeroBoundaryEzY();
    } 
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

    const double xgmin = getXmin();
    const double ygmin = getYmin();

    for (int i = 0; i < w; i++) {
      const double xi = xmin + i * xupp;
      const double xhati = (xi - xgmin) / delta;
      for (int j = 0; j < h; j++) {
        const double yj = ymax - j * yupp;
        const double yhatj = (yj - ygmin) / delta;
        const double Ezij = interpolateEz(xhati, yhatj);
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

  // uniform medium (set properties)
  double relativePermittivity;
  double relativePermeability;
  double electricConductivity;
  double magneticConductivity;

  // update coefficient arrays
  double chxh[NX * NY];
  double chxe[NX * NY];

  double chyh[NX * NY];
  double chye[NX * NY];

  double ceze[NX * NY];
  double cezh[NX * NY];

  int updateCounter;

  bool periodicAlongX;
  bool periodicAlongY;

  fdtdSource source;

  int index(int ix, int iy) const {
    return NX * iy + ix;
  }

  int integerx(double x) const {
    return (int) std::round((x - getXmin()) / getDelta());
  }

  int integery(double y) const {
    return (int) std::round((y - getYmin()) / getDelta());
  }

  double interpolateEz(double xhat, 
                       double yhat) const
  {
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

  // Hx usage size if (NX, NY - 1)
  // Hy usage size is (NX - 1, NY)
  void makeEzPeriodicX() {
    const int ixmin = 0;
    for (int iy = 1; iy < NY - 1; iy++) {
      const int idx = index(ixmin, iy);
      const double dxhy = Hy[idx] - Hy[index(NX - 2, iy)];
      const double dyhx = Hx[idx] - Hx[index(ixmin, iy - 1)];
      Ez[idx] = ceze[idx] * Ez[idx] + cezh[idx] * (dxhy - dyhx);
    }

    const int ixmax = NX - 1;
    for (int iy = 1; iy < NY - 1; iy++) {
      const int idx = index(ixmax, iy);
      const double dxhy = Hy[index(0, iy)] - Hy[index(ixmax - 1, iy)];
      const double dyhx = Hx[idx] - Hx[index(ixmax, iy - 1)];
      Ez[idx] = ceze[idx] * Ez[idx] + cezh[idx] * (dxhy - dyhx);
    }
  }

  void zeroBoundaryEzX() {
    for (int iy = 0; iy < NY; iy++) {
      Ez[index(0, iy)] = 0.0;
      Ez[index(NX - 1, iy)] = 0.0;
    }
  }

  void makeEzPeriodicY() {
    const int iymin = 0;
    for (int ix = 1; ix < NX - 1; ix++) {
      const int idx = index(ix, iymin);
      const double dxhy = Hy[idx] - Hy[index(ix - 1, iymin)];
      const double dyhx = Hx[idx] - Hx[index(ix, NY - 2)];
      Ez[idx] = ceze[idx] * Ez[idx] + cezh[idx] * (dxhy - dyhx);
    }

    const int iymax = NY - 1;
    for (int ix = 1; ix < NX - 1; ix++) {
      const int idx = index(ix, iymax);
      const double dxhy = Hy[idx] - Hy[index(ix - 1, iymax)];
      const double dyhx = Hx[index(ix, 0)] - Hx[index(ix, iymax - 1)];
      Ez[idx] = ceze[idx] * Ez[idx] + cezh[idx] * (dxhy - dyhx);
    }
  }

  void zeroBoundaryEzY() {
    for (int ix = 0; ix < NX; ix++) {
      Ez[index(ix, 0)] = 0.0;
      Ez[index(ix, NY - 1)] = 0.0;
    }
  }

  void applySource() {
    if (source.type == fdtdSourceType::NoSource)
      return;

    const int ix = integerx(source.x);
    if (ix < 0 || ix >= NX)
      return;

    const int iy = integery(source.y);
    if (iy < 0 || iy >= NY)
      return;

    switch (source.type)
    {
    case fdtdSourceType::Monochromatic:
      monochromaticSource(ix, iy);
      break;

    case fdtdSourceType::RickerPulse:
      // FIXME: implement this source
      break;

    default:
      break;
    }
  }

  void monochromaticSource(int ix, int iy) {
    const double lambda = getDelta() * source.ppw;
    const double omega = (2.0 * M_PI) * vacuum_velocity / lambda;
    const double time = updateCounter * getTimestep();
    Ez[index(ix, iy)] = source.amp * std::sin(omega * time);
  }

};

}
