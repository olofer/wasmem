#include <cmath>
#include "../halfband.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

int main(int argc, 
         const char** argv)
{

  const int ndigits = 16;

  HalfbandFilter<5> hbf05;
  hbf05.init();

  HalfbandFilter<7> hbf07;
  hbf07.init();

  HalfbandFilter<10> hbf10;
  hbf10.init();

  if (argc != 2) {
    std::cout << "need one argument" << std::endl;
    return 1;
  }

  if (std::string(argv[1]) == "smoke")
  {
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
  }
  else if (std::string(argv[1]) == "impulse")
  {
    int K = (hbf10.taps() - 1) / 2;
    std::cout << std::setprecision(ndigits);
    std::cout << hbf10.coefficient(-K);
    for (int k = -K + 1; k <= K; k++) {
      std::cout << ", " << hbf10.coefficient(k); 
    }
    std::cout << std::endl;
  }
  else if (std::string(argv[1]) == "test")
  {
    const int L = 365;
    std::vector<double> x(L, 1.0);
    std::vector<double> y(L, 0.0);

    hbf10.applyZero(y.data(), 1, x.data(), 1, x.size());

    std::cout << std::setprecision(ndigits);
    for (size_t i = 0; i < x.size(); i++) {
      std::cout << x[i] << ", " << y[i] << std::endl;
    }
  }
  else 
  {
    std::cout << "did not recognize: \"" << argv[1] << "\"" << std::endl;
    return 1;
  }

  return 0;
}
