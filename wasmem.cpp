#include <emscripten.h>
#include <cstring>
#include <cmath>

#include "fdtd-tmz.hpp"

const int NX = 200;
const int NY = 120;

static TMz::fdtdSolver<NX, NY> sim;

uint32_t rgba_value(int r, int g, int b, int a) {
  return (a << 24) | (b << 16) | (g << 8) | r;
}

uint32_t rgb_value(int r, int g, int b) {
  return rgba_value(r, g, b, 255);
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
void initSolver(double xmin, 
                double deltax,
                double ymin, 
                double deltay)
{
  sim.initialize(xmin, 
                 deltax, 
                 ymin, 
                 deltay);
}

EMSCRIPTEN_KEEPALIVE
int getNX(void) {
  return sim.getNX();
}

EMSCRIPTEN_KEEPALIVE
int getNY(void) {
  return sim.getNY();
}

EMSCRIPTEN_KEEPALIVE
double getEta0(void) {
  return TMz::vacuum_impedance;
}

EMSCRIPTEN_KEEPALIVE
double getVel0(void) {
  return TMz::vacuum_velocity;
}

EMSCRIPTEN_KEEPALIVE
uint32_t* initDataBuffer(int offset, 
                         int w, 
                         int h)
{
  uint32_t* data = reinterpret_cast<uint32_t*>(offset);
  std::memset(data, 0, sizeof(uint32_t) * w * h);
  return &data[0];
}

EMSCRIPTEN_KEEPALIVE
void renderDataBuffer(int offset, 
                      int gvalue, 
                      int w, 
                      int h)
{
  uint32_t* data = reinterpret_cast<uint32_t*>(offset);
  for (int i = 0; i < w; i++) {
    for (int j = 0; j < h; j++) {
      data[i + j * w] = rgb_value(i % 255, gvalue % 255, j % 255);
    }
  }
}

} // close extern "C"
