#pragma once

namespace TMz {

template <int NX, int NY>
struct fdtdAbsorbingBoundary
{
  double ezLeft[6 * NY];
  double ezRight[6 * NY];
  double ezTop[6 * NX];
  double ezBottom[6 * NX];

  double coef0, coef1, coef2;
  int bskip;

  int indexLR(int m, int q, int n) const {
    return n * 6 + q * 3 + m;
  }

  double EzLeft(int m, int q, int n) const {
    return ezLeft[indexLR(m, q, n)];
  }

  double EzRight(int m, int q, int n) const {
    return ezRight[indexLR(m, q, n)];
  }

  int indexTB(int n, int q, int m) const {
    return m * 6 + q * 3 + n;
  }

  double EzTop(int n, int q, int m) const {
    return ezTop[indexTB(n, q, m)];
  }

  double EzBottom(int n, int q, int m) const {
    return ezBottom[indexTB(n, q, m)];
  }

  int index(int ix, int iy) const {
    return NX * iy + ix;
  }

  void zeroX() {
    std::memset(ezLeft, 0 , sizeof(double) * 6 * NY);
    std::memset(ezRight, 0 , sizeof(double) * 6 * NY);
  }

  void zeroY() {
    std::memset(ezTop, 0 , sizeof(double) * 6 * NX);
    std::memset(ezBottom, 0 , sizeof(double) * 6 * NX);
  }

  void zero() {
    zeroX();
    zeroY();
  }

  void cornerExclude() { bskip = 1; }
  void cornerInclude() { bskip = 0; }

  void initialize(double cezh0, 
                  double chye0) 
  {
    const double temp1 = std::sqrt(cezh0 * chye0);
    const double temp2 = 1.0 / temp1 + 2.0 + temp1;
    coef0 = -(1.0 / temp1 - 2.0 + temp1) / temp2;
    coef1 = -2.0 * (temp1 - 1.0 / temp1) / temp2;
    coef2 = 4.0 * (temp1 + 1.0 / temp1) / temp2;
    cornerExclude(); // NOTE: if this is 0; problems appear ** in combination ** with periodic boundaries
    zero();
  }

  void applyLeft(double* Ez,
                 bool apply = true,
                 bool record = true)
  {
    for (int iy = bskip; iy < NY - bskip; iy++) {
      if (apply)
        Ez[index(0, iy)] = coef0 * (Ez[index(2, iy)] + EzLeft(0, 1, iy)) + 
                           coef1 * (EzLeft(0, 0, iy) + EzLeft(2, 0, iy) - Ez[index(1, iy)] - EzLeft(1, 1, iy)) +
                           coef2 * EzLeft(1, 0, iy) - EzLeft(2, 1, iy);
      if (record)
        for (int w = 0; w < 3; w++) {
          ezLeft[indexLR(w, 1, iy)] = EzLeft(w, 0, iy);
          ezLeft[indexLR(w, 0, iy)] = Ez[index(w, iy)];
        }
    }
  }

  void applyRight(double* Ez,
                  bool apply = true,
                  bool record = true)
  {
    for (int iy = bskip; iy < NY - bskip; iy++) {
      if (apply)
        Ez[index(NX - 1, iy)] = coef0 * (Ez[index(NX - 3, iy)] + EzRight(0, 1, iy)) +
                                coef1 * (EzRight(0, 0, iy) + EzRight(2, 0, iy) - Ez[index(NX - 2, iy)] - EzRight(1, 1, iy)) +
                                coef2 * EzRight(1, 0, iy) - EzRight(2, 1, iy);
      if (record)
        for (int w = 0; w < 3; w++) {
          ezRight[indexLR(w, 1, iy)] = EzRight(w, 0, iy);
          ezRight[indexLR(w, 0, iy)] = Ez[index(NX - 1 - w, iy)];
        }
    }
  }

  void applyTop(double* Ez,
                bool apply = true,
                bool record = true)
  {
    for (int ix = bskip; ix < NX - bskip; ix++) {
      if (apply)
        Ez[index(ix, NY - 1)] = coef0 * (Ez[index(ix, NY - 3)] + EzTop(0, 1, ix)) + 
                                coef1 * (EzTop(0, 0, ix) + EzTop(2, 0, ix) - Ez[index(ix, NY - 2)] - EzTop(1, 1, ix)) + 
                                coef2 * EzTop(1, 0, ix) - EzTop(2, 1, ix);
      if (record)
        for (int w = 0; w < 3; w++) {
          ezTop[indexTB(w, 1, ix)] = EzTop(w, 0, ix);
          ezTop[indexTB(w, 0, ix)] = Ez[index(ix, NY - 1 - w)];
        }
    }
  }

  void applyBottom(double* Ez,
                   bool apply = true,
                   bool record = true)
  {
    for (int ix = bskip; ix < NX - bskip; ix++) {
      if (apply)
        Ez[index(ix, 0)] = coef0 * (Ez[index(ix, 2)] + EzBottom(0, 1, ix)) + 
                           coef1 * (EzBottom(0, 0, ix) + EzBottom(2, 0, ix) - Ez[index(ix, 1)] - EzBottom(1, 1, ix)) + 
                           coef2 * EzBottom(1, 0, ix) - EzBottom(2, 1, ix);
      if (record)
        for (int w = 0; w < 3; w++) {
          ezBottom[indexTB(w, 1, ix)] = EzBottom(w, 0, ix);
          ezBottom[indexTB(w, 0, ix)] = Ez[index(ix, w)];
        }
    }
  }

};

template <int NX, int NY>
class fdtdSolver
{
public:
  void initialize(double xmin, 
                  double ymin, 
                  double delta)
  {
    hbf.init();

    for (int ix = 0; ix < NX; ix++) {
      xgrid[ix] = xmin + ix * delta;
    }

    for (int iy = 0; iy < NY; iy++) {
      ygrid[iy] = ymin + iy * delta;
    }

    setPeriodicX();
    setPeriodicY();

    setUniformMedium(1.0, 1.0, 0.0, 0.0);
    abc.initialize(cezh[0], chye[0]);

    reset();

    source.initDefault();
  }

  void reset() {
    zeroField();
    abc.zero();
    resetUpdateCount();
    source.resetTheta();
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
  bool isAbsorbingX() const { return absorbingLeft && absorbingRight && !periodicAlongX; }
  bool isAbsorbingY() const { return absorbingTop && absorbingBottom && !periodicAlongY; }
  bool isMixedX() const { return (absorbingLeft ^ absorbingRight) && !periodicAlongX;  }
  bool isMixedY() const { return (absorbingTop ^ absorbingBottom) && !periodicAlongY;  }

  void zeroField() {
    std::memset(Hx, 0, NX * NY * sizeof(double));
    std::memset(Hy, 0, NX * NY * sizeof(double));
    std::memset(Ez, 0, NX * NY * sizeof(double));
  }

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

  void setDamping(double lhat) {
    const double sigma_delta = source.sigmaDelta(lhat, relativePermeability);
    setUniformMedium(relativePermeability,
                     relativePermittivity,
                     magneticConductivity,
                     sigma_delta / getDelta());
  }

  void setVacuum() {
    setUniformMedium(1.0, 1.0, 0.0, 0.0);
  }

  bool isVacuum() const {
    return relativePermeability == 1.0 && 
           relativePermittivity == 1.0 &&
           magneticConductivity == 0.0 &&
           electricConductivity == 0.0;
  }

  // add Gaussian centered at (xc, yc) onto the Ez field (units are points)
  void superimposeGaussian(double xc, 
                           double yc, 
                           double sigmax, 
                           double sigmay)
  {
    for (int ix = 1; ix < NX - 1; ix++) {
      for (int iy = 1; iy < NY - 1; iy++) {
        const int idx = index(ix, iy);
        const double xhat = ((double) ix - xc) / sigmax;
        const double yhat = ((double) iy - yc) / sigmay;
        Ez[idx] += std::exp(-0.5 * (xhat * xhat + yhat * yhat));
      }
    }
  }

  double energyE() const {
    double sum = 0.0;
    for (int i = 0; i < size(); i++) {
      const double Ezi = Ez[i];
      sum += Ezi * Ezi;
    }
    const double delta = getDelta();
    return relativePermittivity * vacuum_permittivity * (sum * delta * delta / 2.0);
  }

  // NOTE: not actually synchronized in time with the E-field energy calc above
  double energyB() const {
    double sum = 0.0;
    for (int i = 0; i < size(); i++) {
      const double Hxi = Hx[i];
      const double Hyi = Hy[i];
      sum += Hxi * Hxi + Hyi * Hyi;
    }
    const double delta = getDelta();
    return relativePermeability * vacuum_permeability * (sum * delta * delta / 2.0);
  }
  
  void update()  /* full single timestep state update */
  {
    updateHxHy();
    updateEz();

    if (periodicAlongX) {
      makeEzPeriodicX();
    } else {
      if (absorbingLeft) abc.applyLeft(Ez);
      if (absorbingRight) abc.applyRight(Ez);
    }

    if (periodicAlongY) {
      makeEzPeriodicY();
    } else {
      if (absorbingTop) abc.applyTop(Ez);
      if (absorbingBottom) abc.applyBottom(Ez);
    }

    applySource();

    source.updateTheta();
    updateCounter++;
  }

  void halfbandFilterXY() {
    halfbandFilterXY_(Ez);
    halfbandFilterXY_(Hx);
    halfbandFilterXY_(Hy);

    const int taper = 8;
    taperBorderX(taper);
    abc.zeroX();
    taperBorderY(taper);
    abc.zeroY();
  }

  void setPeriodicX() {
    absorbingLeft = false;
    absorbingRight = false;
    periodicAlongX = true;
    const int taper = 8;
    taperBorderX(taper);
  }

  void setPeriodicY() {
    absorbingTop = false;
    absorbingBottom = false;
    periodicAlongY = true;
    const int taper = 8;
    taperBorderY(taper);
  }

  void setAbsorbingX() {
    const int taper = 12;
    absorbingLeft = true;
    absorbingRight = true;
    periodicAlongX = false;
    abc.zeroX();
    taperBorderX(taper);
    if (isAbsorbingY()) {
      abc.zeroY();
      taperBorderY(taper);
    }
  }

  void setAbsorbingY() {
    const int taper = 12;
    absorbingTop = true;
    absorbingBottom = true;
    periodicAlongY = false;
    abc.zeroY();
    taperBorderY(taper);
    if (isAbsorbingX()) {
      abc.zeroX();
      taperBorderX(taper);
    }
  }

  void setPECX() {
    absorbingLeft = false;
    absorbingRight = false;
    periodicAlongX = false;
    zeroBoundaryEzX();
  }

  void setPECY() {
    absorbingTop = false;
    absorbingBottom = false;
    periodicAlongY = false;
    zeroBoundaryEzY();
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
                   bool viridis,
                   double ezmin,
                   double ezmax,
                   double xmin,
                   double xmax,
                   double ymin,
                   double ymax) const
  {
    uint32_t (*rgbfunc)(float) = (viridis ? rgb_d_viridis : rgb_d_jet);
    if (imgdata == nullptr) return;
    if (ezmin >= ezmax) return;

    const double delta = getDelta();
    const double crange = ezmax - ezmin;
    const float A = (float) (1.0 / crange);
    const float B = (float) (-1.0 * ezmin / crange);

    const double xupp = (xmax - xmin) / w; // x units per pixel
    const double yupp = (ymax - ymin) / h; // y units per pixel

    const double xgmin = getXmin();
    const double ygmin = getYmin();

    const double Y0 = (ymax - ygmin) / delta;
    const double Y1 = -1.0 * yupp / delta;

    for (int i = 0; i < w; i++) {
      const double xi = xmin + i * xupp;
      const double xhati = (xi - xgmin) / delta;
      for (int j = 0; j < h; j++) {
        //const double yj = ymax - j * yupp;
        //const double yhatj = (yj - ygmin) / delta;
        //const double Ezij = interpolate(Ez, xhati, yhatj);
        const double yhatj = Y0 + Y1 * j;
        const float Ezij = interpolate_float(Ez, (float) xhati, (float) yhatj);
        imgdata[i + j * w] = (*rgbfunc)(A * Ezij + B);
      }
    }
  }

  void rasterizeTestPattern(uint32_t* imgdata, 
                            int w, 
                            int h,
                            bool viridis) const 
  {
    uint32_t (*rgbfunc)(int) = (viridis ? rgb_i_viridis : rgb_i_jet);
    if (imgdata == nullptr) return;
    for (int i = 0; i < w; i++) {
      for (int j = 0; j < h; j++) {
        imgdata[i + j * w] = (*rgbfunc)((i + j + updateCounter) % 255);
      }
    }
  }

  void sourceMove(double dx, 
                  double dy)
  {
    source.x += dx;
    source.y += dy;
  }

  void sourcePlace(double x, 
                   double y)
  {
    source.x = x;
    source.y = y;
  }

  double sourceTune() const {
    return source.ppw;
  }

  void sourceTune(double dppw) {
    double new_ppw = source.ppw + dppw;
    if (new_ppw < 2.0) new_ppw = 2.0;
    source.setPPW(new_ppw);
  }

  void sourceType(fdtdSourceType s) {
    source.type = s;
  }

  double sourceAmplitude() const {
    return source.amp;
  }

  double sourceAmplitude(double a) {
    source.amp = a;
  }

  bool sourceAdditive() const {
    return source.additive;
  }

  void sourceAdditive(bool a) {
    source.additive = a;
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

  double Y[NX + NY]; // can be used for temporary filter results

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

  bool absorbingLeft;
  bool absorbingRight;
  bool absorbingTop;
  bool absorbingBottom;

  fdtdAbsorbingBoundary<NX, NY> abc;

  fdtdSource source;

  HalfbandFilter<5> hbf;

  int index(int ix, int iy) const {
    return NX * iy + ix;
  }

  int integerx(double x) const {
    return (int) std::round((x - getXmin()) / getDelta());
  }

  int integery(double y) const {
    return (int) std::round((y - getYmin()) / getDelta());
  }

  double interpolate(const double* f,
                     double xhat, 
                     double yhat) const
  {
    const int xi = (int) xhat;
    const int yi = (int) yhat;
    const double etax = xhat - xi;
    const double etay = yhat - yi;

    const double v00 = f[index(xi, yi)];
    const double v01 = f[index(xi, yi + 1)];
    const double v10 = f[index(xi + 1, yi)];
    const double v11 = f[index(xi + 1, yi + 1)];

    const double w00 = (1.0 - etax) * (1.0 - etay);
    const double w01 = (1.0 - etax) * etay;
    const double w10 = etax * (1.0 - etay);
    const double w11 = etax * etay;

    return w00 * v00 + w01 * v01 + w10 * v10 + w11 * v11;
  }

  float interpolate_float(const double* f,
                          float xhat, 
                          float yhat) const
  {
    const int xi = (int) xhat;
    const int yi = (int) yhat;
    const float etax = xhat - xi;
    const float etay = yhat - yi;

    const int idx = index(xi, yi);

    const float v00 = (float) f[idx];
    const float v01 = (float) f[idx + NX];
    const float v10 = (float) f[idx + 1];
    const float v11 = (float) f[idx + 1 + NX];

    const float w00 = (1.0 - etax) * (1.0 - etay);
    const float w01 = (1.0 - etax) * etay;
    const float w10 = etax * (1.0 - etay);
    const float w11 = etax * etay;

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

  void taperBorderX(int width) {
    for (int iy = 0; iy < NY; iy++) {
      for (int w = 0; w < width; w++) {
        const double sw = static_cast<double>(w) / width;
        const double swsq = sw * sw;
        Ez[index(w, iy)] *= swsq;
        Ez[index(NX - 1 - w, iy)] *= swsq;
        Hx[index(w, iy)] *= swsq;
        Hx[index(NX - 1 - w, iy)] *= swsq;
        Hy[index(w, iy)] *= swsq;
        Hy[index(NX - 1 - w, iy)] *= swsq;
      }
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

  void taperBorderY(int width) {
    for (int ix = 0; ix < NX; ix++) {
      for (int w = 0; w < width; w++) {
        const double sw = static_cast<double>(w) / width;
        const double swsq = sw * sw;
        Ez[index(ix, w)] *= swsq;
        Ez[index(ix, NY - 1 - w)] *= swsq;
        Hx[index(ix, w)] *= swsq;
        Hx[index(ix, NY - 1 - w)] *= swsq;
        Hy[index(ix, w)] *= swsq;
        Hy[index(ix, NY - 1 - w)] *= swsq;
      }
    }
  }

  void applySource() {
    const int ix = integerx(source.x);
    if (ix < 0 || ix >= NX)
      return;

    const int iy = integery(source.y);
    if (iy < 0 || iy >= NY)
      return;

    if (source.type == fdtdSourceType::NoSource)
      return;

    const double Sxy = source.get(updateCounter);

    if (source.additive) {
      Ez[index(ix, iy)] += Sxy;
    } else {
      Ez[index(ix, iy)] = Sxy;
    }
  }

  void halfbandFilterXY_(double* f) {
    // filter horizontally
    for (int iy = 0; iy < NY; iy++) {
      hbf.apply(Y, 1, &f[index(0, iy)], 1, NX);
      std::memcpy(&f[index(0, iy)], Y, sizeof(double) * NX);
    }
    // filter vertically
    for (int ix = 0; ix < NX; ix++) {
      hbf.apply(Y, 1, &f[index(ix, 0)], NX, NY);
      for (int iy = 0; iy < NY; iy++) {
        f[index(ix, iy)] = Y[iy];
      }
    }
  }

};

}
