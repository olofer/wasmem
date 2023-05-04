#include <emscripten.h>
#include <cstring>
#include <cmath>

#include "halfband.hpp"
#include "rgb-utils.hpp"
#include "fdtd-constants.hpp"
#include "fdtd-source.hpp"
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
void setPeriodicX(void) {
  sim.setPeriodicX();
}

EMSCRIPTEN_KEEPALIVE
bool getPeriodicX(void) {
  return sim.isPeriodicX();
}

EMSCRIPTEN_KEEPALIVE
void setPeriodicY(void) {
  sim.setPeriodicY();
}

EMSCRIPTEN_KEEPALIVE
bool getPeriodicY(void) {
  return sim.isPeriodicY();
}

EMSCRIPTEN_KEEPALIVE
void setAbsorbingX(void) {
  sim.setAbsorbingX();
}

EMSCRIPTEN_KEEPALIVE
bool getAbsorbingX(void) {
  return sim.isAbsorbingX();
}

EMSCRIPTEN_KEEPALIVE
void setAbsorbingY(void) {
  sim.setAbsorbingY();
}

EMSCRIPTEN_KEEPALIVE
bool getAbsorbingY(void) {
  return sim.isAbsorbingY();
}

EMSCRIPTEN_KEEPALIVE
void setPECX(void) {
  sim.setPECX();
}

EMSCRIPTEN_KEEPALIVE
void setPECY(void) {
  sim.setPECY();
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
void dropGaussian(double x, 
                  double y) 
{
  sim.superimposeGaussian(x, y, 10.0, 10.0);
}

EMSCRIPTEN_KEEPALIVE
void applyHalfbandFilter(void) {
  sim.halfbandFilterXY();
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
  sim.sourceType(fdtdSourceType::NoSource);
}

EMSCRIPTEN_KEEPALIVE
void sourceMono(void) {
  sim.sourceType(fdtdSourceType::Monochromatic);
}

EMSCRIPTEN_KEEPALIVE
void sourceRicker(void) {
  sim.sourceType(fdtdSourceType::RickerPulse);
}

EMSCRIPTEN_KEEPALIVE
void sourceSquare(void) {
  sim.sourceType(fdtdSourceType::SquareWave);
}

EMSCRIPTEN_KEEPALIVE
void sourceSaw(void) {
  sim.sourceType(fdtdSourceType::Sawtooth);
}

EMSCRIPTEN_KEEPALIVE
void sourceAdditive(bool a) {
  sim.sourceAdditive(a);
}

EMSCRIPTEN_KEEPALIVE
bool isSourceAdditive(void) {
  return sim.sourceAdditive();
}

EMSCRIPTEN_KEEPALIVE
double fieldEnergyE(void) {
  return sim.energyE();
}

EMSCRIPTEN_KEEPALIVE
double fieldEnergyB(void) {
  return sim.energyB();
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
  return vacuum_impedance;
}

EMSCRIPTEN_KEEPALIVE
double getVel0(void) {
  return vacuum_velocity;
}

EMSCRIPTEN_KEEPALIVE
double getCourant(void) {
  return courant_factor;
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
                                 int h,
                                 bool viridis)
{
  uint32_t* data = reinterpret_cast<uint32_t*>(offset);
  sim.rasterizeTestPattern(data, w, h, viridis);
}

EMSCRIPTEN_KEEPALIVE
void renderDataBufferEz(int offset, 
                        int w, 
                        int h,
                        bool viridis,
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
                  viridis,
                  cmin, 
                  cmax, 
                  sim.getXmin(), 
                  sim.getXmax() - 1.0e-8 * getDelta(),
                  sim.getYmin(),
                  sim.getYmax() - 1.0e-8 * getDelta());
}

} // close extern "C"
