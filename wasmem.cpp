#include <emscripten.h>
#include <cstring>
#include <cmath>

#include "rgb-utils.hpp"
#include "fdtd-tmz.hpp"

const int NX = 300;
const int NY = 175; // 300 / 175 = 1200 / 700 (same aspect ratio)

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
void resetSolver(void) {
  sim.reset();
}

EMSCRIPTEN_KEEPALIVE
void setPeriodicX(bool X) {
  sim.setPeriodicX(X);
}

EMSCRIPTEN_KEEPALIVE
bool getPeriodicX() {
  return sim.isPeriodicX();
}

EMSCRIPTEN_KEEPALIVE
void setPeriodicY(bool Y) {
  sim.setPeriodicY(Y);
}

EMSCRIPTEN_KEEPALIVE
bool getPeriodicY() {
  return sim.isPeriodicY();
}

EMSCRIPTEN_KEEPALIVE
void setVacuum(void) {
  sim.setVacuum();
}

EMSCRIPTEN_KEEPALIVE
bool isVacuum(void) {
  return sim.isVacuum();
}

EMSCRIPTEN_KEEPALIVE
void setDamping(double lhat) {
  sim.setDamping(lhat);
}

EMSCRIPTEN_KEEPALIVE
void sourceMove(double dx, 
                double dy)
{
  sim.sourceMove(dx, dy);
}

EMSCRIPTEN_KEEPALIVE
void sourcePlace(double x, 
                 double y)
{
  sim.sourcePlace(x, y);
}

EMSCRIPTEN_KEEPALIVE
void sourceTuneSet(double dppw) {
  sim.sourceTune(dppw);
}

EMSCRIPTEN_KEEPALIVE
double sourceTuneGet(void) {
  return sim.sourceTune();
}

EMSCRIPTEN_KEEPALIVE
void sourceNone(void) {
  sim.sourceType(TMz::fdtdSourceType::NoSource);
}

EMSCRIPTEN_KEEPALIVE
void sourceMono(void) {
  sim.sourceType(TMz::fdtdSourceType::Monochromatic);
}

EMSCRIPTEN_KEEPALIVE
void sourceRicker(void) {
  sim.sourceType(TMz::fdtdSourceType::RickerPulse);
}

EMSCRIPTEN_KEEPALIVE
void sourceSquare(void) {
  sim.sourceType(TMz::fdtdSourceType::SquareWave);
}

EMSCRIPTEN_KEEPALIVE
double fieldEnergyEz(void) {
  return sim.energyE();
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
void renderDataBufferTestPattern(int offset, 
                                 int w, 
                                 int h)
{
  uint32_t* data = reinterpret_cast<uint32_t*>(offset);
  sim.rasterizeTestPattern(data, w, h);
}

EMSCRIPTEN_KEEPALIVE
void renderDataBufferEz(int offset, 
                        int w, 
                        int h,
                        bool useSourceAmp,
                        double cmin,
                        double cmax)
{
  if (useSourceAmp || cmin >= cmax) {
    const double srcamp = std::fabs(sim.sourceAmplitude());
    cmin = -1.0 * srcamp;
    cmax = srcamp;
  }

  uint32_t* data = reinterpret_cast<uint32_t*>(offset);

  sim.rasterizeEz(data, 
                  w, 
                  h, 
                  cmin, 
                  cmax, 
                  sim.getXmin(), 
                  sim.getXmax() - 1.0e-8 * getDelta(),
                  sim.getYmin(),
                  sim.getYmax() - 1.0e-8 * getDelta());
}

} // close extern "C"
