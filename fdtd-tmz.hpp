#pragma once

namespace TMz {

template <int NX, int NY>
class fdtdSolver
{
public:
  void initialize(double xmin, double deltax,
                  double ymin, double deltay)
  {
    for (int i = 0; i < NX; i++) {
      xgrid[i] = xmin + i * deltax;
    }

    for (int i = 0; i < NY; i++) {
      ygrid[i] = ymin + i * deltay;
    }

    std::memset(Hx, 0, NX * NY * sizeof(double));
    std::memset(Hy, 0, NX * NY * sizeof(double));
    std::memset(Ez, 0, NX * NY * sizeof(double));
  }

  int getNX() const { return NX; }
  int getNY() const { return NY; }

private:
  double xgrid[NX];
  double ygrid[NY];

  double Hx[NX * NY];
  double Hy[NX * NY];
  double Ez[NX * NY];
};

}
