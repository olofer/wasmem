#include <iostream>
#include <cmath>
#include "../halfband.hpp"
#include <vector>

// g++ test-halfband.cpp
// ./a.out && echo OK!

int main(int argc, const char** argv) {

  HalfbandFilter<5> hbf05;
  hbf05.init();

  HalfbandFilter<7> hbf07;
  hbf07.init();

  HalfbandFilter<10> hbf10;
  hbf10.init();

  const int L = 101;

  std::vector<double> x(L, 1.0); // unit signal test

  std::vector<double> y05(x.size(), 0.0);
  std::vector<double> y07(x.size(), 0.0);
  std::vector<double> y10(x.size(), 0.0);

  // First test is that the results should be all-1's
  const double atol = 1.0e-14;

  hbf05.applyHold(y05.data(), 1, x.data(), 1, x.size());
  hbf07.applyHold(y07.data(), 1, x.data(), 1, x.size());
  hbf10.applyHold(y10.data(), 1, x.data(), 1, x.size());

  for (size_t i = 0; i < x.size(); i++) {
    if (std::fabs(y05[i] - x[i]) > atol) return 1;
    if (std::fabs(y07[i] - x[i]) > atol) return 1;
    if (std::fabs(y10[i] - x[i]) > atol) return 1;
  }

  // Second smoke test also full of 1's

  hbf05.applyPeriodic(y05.data(), 1, x.data(), 1, x.size());
  hbf07.applyPeriodic(y07.data(), 1, x.data(), 1, x.size());
  hbf10.applyPeriodic(y10.data(), 1, x.data(), 1, x.size());

  for (size_t i = 0; i < x.size(); i++) {
    if (std::fabs(y05[i] - x[i]) > atol) return 1;
    if (std::fabs(y07[i] - x[i]) > atol) return 1;
    if (std::fabs(y10[i] - x[i]) > atol) return 1;
  }

  // TODO: some way of setting an impulse response
  // TODO: next test should be more complete: generate random signal and write to stdout; then postprocess in Octave

  //std::cout << "x, y05 ,y07, y10" << std::endl;
    //std::cout << x[i] << ", " << y05[i] << ", " << y07[i] << ", " << y10[i]  << std::endl;

  return 0;
}
