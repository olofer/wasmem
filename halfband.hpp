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
    dc_normalize();
  }

  void setHead(double v) {
    for (int i = 0; i < K; i++)
      head[i] = v;
  }

  void setTail(double v) {
    for (int i = 0; i < K; i++)
      tail[i] = v;
  }

  void apply(double* y,
             int stridey,
             const double* x,
             int stridex, 
             int L) const
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
      for (int n = -K; n < L - i; n++) s += b[n + K] * x[(n + i) * stridex];
      for (int n = L - i; n <= K; n++)  s += b[n + K] * tail[n + i - L];
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

  void applyZero(double* y,
                 int stridey,
                 const double* x,
                 int stridex, 
                 int L)
  {
    setHead(0.0);
    setTail(0.0);
    apply(y, stridey, x, stridex, L);
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

  double dc() const {
    double s = 0.0;
    for (int i = 0; i < 2 * K + 1; i++)
      s += b[i];
    return s;
  }

  void dc_normalize() {
    const double DC = dc();
    for (int i = 0; i < 2 * K + 1; i++)
      b[i] /= DC;
  }

  double coefficient(int k) const {
    if (k < -K || k > K) return 0.0;
    return b[k + K];
  }

  int taps() const {
    return 2 * K + 1;
  }

private:
  double b[2 * K + 1];
  double head[K];
  double tail[K];
};
