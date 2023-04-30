#pragma once

// (2K + 1)-tap halfband FIR filter
template <int K>
class HalfbandFilter {
public:
  void init() {
    setHead(0.0);
    setTail(0.0);
    for (int n = -K; n <= K; n++) {
      const double npi = static_cast<double>(n) * M_PI;
      b[K + n] = (n != 0 ? std::sin(npi / 2.0) / npi : 1.0 / 2.0);
      const double wn = 0.54 + 0.46 * std::cos(npi / K);
      b[K + n] *= wn;
    }
  }

  void setHead(double v) {
    for (int i = 0; i < K; i++)
      head[i] = v;
  }

  void setTail(double v) {
    for (int i = 0; i < K; i++)
      tail[i] = v;
  }

  void applyHold(double* y,
                 int stridey,
                 const double* x,
                 int stridex, 
                 int L)
  {
    setHead(x[0]);
    setTail(x[L - 1]);
    apply(y, stridey, x, stridex, L);
  }

  void apply(double* y,
             int stridey,
             const double* x,
             int stridex, 
             int L)
  {
    for (int i = 0; i < K; i++) {
      double s = 0.0;
      for (int n = -K; n < -i; n++) s += b[n + K] * head[n + K + i];
      for (int n = -i; n <= K; n++) s += b[n + K] * x[(n + i) * stridex];
      y[i * stridey] = s;
    }
    for (int i = K; i < L - K; i++) {
      double s = 0.0;
      for (int n = -K; n <= K; n++) {
        s += b[n + K] * x[(n + i) * stridex];
      }
      y[i * stridey] = s;
    }
    for (int i = L - K; i < L; i++) {
      double s = 0.0;
      for (int n = -K; n <= 0; n++) s += b[n + K] * x[(n + i) * stridex];
      // TODO: apply tail
      // ...
      y[i * stridey] = s;
    }
  }

  void applyPeriodic(double* y,
                     int stridey,
                     const double* x, 
                     int stridex, 
                     int L)
  {
    for (int i = 0; i < K; i++) {
      head[K - i - 1] = x[(L - i - 1) * stridex];
      tail[i] = x[i * stridex];
    }
    apply(y, stridey, x, stridex, L);
  }

private:
  double b[2 * K + 1];
  double head[K];
  double tail[K];
};
