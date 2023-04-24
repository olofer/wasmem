#include <emscripten.h>
#include <cstring>
#include <cmath>

#include "rgb-utils.hpp"
#include "fdtd-tmz.hpp"

const int NX = 200;
const int NY = 120;

static TMz::fdtdSolver<NX, NY> sim;

extern "C" {

EMSCRIPTEN_KEEPALIVE
void* simulatorAddress(void) {
  return reinterpret_cast<void*>(&sim);
}

EMSCRIPTEN_KEEPALIVE
int simulatorBytesize(void) {
  return sizeof(sim);
}

EMSCRIPTEN_KEEPALIVE
void initSolver(double xmin, 
                double ymin, 
                double delta)
{
  sim.initialize(xmin, 
                 ymin, 
                 delta);
}

EMSCRIPTEN_KEEPALIVE
void takeOneTimestep(void) {
  sim.update();
}

EMSCRIPTEN_KEEPALIVE
double getDelta(void) {
  return sim.getDelta();
}

EMSCRIPTEN_KEEPALIVE
double getTimestep(void) {
  return sim.getTimestep();
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
double getCourant(void) {
  return TMz::courant_factor;
}

EMSCRIPTEN_KEEPALIVE
double minimumEz(void) {
  return sim.minimumEz();
}

EMSCRIPTEN_KEEPALIVE
double maximumEz(void) {
  return sim.maximumEz();
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

EMSCRIPTEN_KEEPALIVE
void renderDataBufferEz(int offset, 
                        int w, 
                        int h)
{
  uint32_t* data = reinterpret_cast<uint32_t*>(offset);
  sim.rasterizeEz(data, 
                  w, 
                  h, 
                  -1.0, 
                  1.0, 
                  sim.getXmin(), 
                  sim.getXmax(),
                  sim.getYmin(),
                  sim.getYmax());
}

} // close extern "C"
