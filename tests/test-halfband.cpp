#include <iostream>
#include <cmath>
#include "../halfband.hpp"

int main(int argc, const char** argv) {

  HalfbandFilter<5> hbf5;
  hbf5.init();

  HalfbandFilter<7> hbf7;
  hbf7.init();

  HalfbandFilter<10> hbf10;
  hbf10.init();

  std::cout << hbf5.dc() << std::endl;
  std::cout << hbf7.dc() << std::endl;
  std::cout << hbf10.dc() << std::endl;

  // TODO: run the filter on a unit signal and check that the result is also unit; including edges..

}
