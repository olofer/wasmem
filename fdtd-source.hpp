#pragma once

enum fdtdSourceType {
  NoSource,
  Monochromatic,
  RickerPulse,
  SquareWave,
  Sawtooth
};

struct fdtdSource
{
  fdtdSourceType type;
  bool additive;

  double ppw;
  double x;
  double y;
  double amp;
  double delayMultiplier;
  double theta;
  double radiansPerTimestep;

  void initDefault() {
    type = fdtdSourceType::Monochromatic;
    additive = false;
    x = 0.05;
    y = 0.05;
    amp = 1.0;
    delayMultiplier = 2.0;
    resetTheta();
    setPPW(30.0);
  }

  double sinusoidal(double q) const {
    return amp * std::sin(2.0 * M_PI * courant_factor * q / ppw);
  }

  double sinusoidal() const {
    return amp * std::sin(theta);
  }

  double ricker(int q) const {
    const int qd = static_cast<int>(delayMultiplier * ppw / courant_factor);
    const int qeff = q % (2 * qd);
    const double eta = M_PI * courant_factor * (qeff - qd) / ppw;
    return amp * std::exp(-1.0 * eta * eta) * (1.0 - 2.0 * eta * eta);
  }

  double sawtooth(int q) const {
    const int Q = static_cast<int>(this->ppw / courant_factor);
    return (-1 + 2.0 * static_cast<double>(q % Q) / Q) * (this->amp);
  }

  double soft_sawtooth() const {
    // (sin(x) - 1/2 sin(2 x) + 1/3 sin(3 x) - 1/4 sin(4 x) + 1/5 sin(5 x)) * (2 / pi)
    const double multiplier = (2.0 / M_PI) * amp;
    double s = std::sin(theta);
    s -= std::sin(2.0 * theta) / 2.0;
    s += std::sin(3.0 * theta) / 3.0;
    s -= std::sin(4.0 * theta) / 4.0;
    s += std::sin(5.0 * theta) / 5.0;
    return s * multiplier;
  }

  double soft_squarewave() const {
    const double multiplier = (4.0 / M_PI) * amp;
    double s = std::sin(theta);
    s += std::sin(3.0 * theta) / 3.0;
    s += std::sin(5.0 * theta) / 5.0;
    s += std::sin(7.0 * theta) / 7.0;
    return s * multiplier;
  }

  void off() {
    type = fdtdSourceType::NoSource;
  }

  void setPPW(int ppw) {
    this->ppw = ppw;
    this->radiansPerTimestep = 2.0 * M_PI * courant_factor / this->ppw;
  }

  void updateTheta() {
    this->theta += this->radiansPerTimestep;
  }

  void resetTheta() {
    this->theta = 0.0;
  }

  // Compute conductivity (sigma) times space-step (delta) to achieve a skin-length of lhat space steps
  // for the present value of the source wavelength (expressed in PPW)
  double sigmaDelta(double lhat, 
                    double mur) const
  {
    const double recip = lhat * lhat * vacuum_permeability * mur * M_PI * vacuum_velocity / this->ppw;
    return 1.0 / recip;
  }

  double get(int counter) const {
    double Sxy = 0.0;

    switch (this->type)
    {
    case fdtdSourceType::Monochromatic:
      //Sxy = sinusoidal(static_cast<double>(counter));
      Sxy = sinusoidal();
      break;

    case fdtdSourceType::RickerPulse:
      Sxy = ricker(counter);
      break;

    case fdtdSourceType::SquareWave:
      //Sxy = (sinusoidal(static_cast<double>(counter)) < 0.0 ? -this->amp : this->amp);
      //Sxy = (sinusoidal() < 0.0 ? -this->amp : this->amp);
      Sxy = soft_squarewave();
      break;

    case fdtdSourceType::Sawtooth:
      //Sxy = sawtooth(counter);
      Sxy = soft_sawtooth();
      break;

    case fdtdSourceType::NoSource:
      break;
    }

    return Sxy;
  }

};
