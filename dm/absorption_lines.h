/**
 *
 *
 *  Generate a fake absorption spectrum for the
 *  Dark Matter lab
 *
 */

#ifndef ABSORPTION_LINES
#define ABSORPTION_LINES

#include <cmath>
#include <ctime>
#include <iostream>
#include <random>

#if WIN32 // Fucking windows
#include <qwt_plot_curve.h>
#include <qwt_point_data.h>
#include <qwt_series_data.h>
#else

#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_point_data.h>
#include <qwt/qwt_series_data.h>

#endif


class absorption_lines : public QwtSyntheticPointData {

public:

  bool noise_ = false;

  // true if there is `signal` data
  bool has_signal_ = false;

  // Scale: (5, 30, 250)
  float scale_ = 1.0f;
  float x_shift_ = 0.0f;

  float _height = 1.0f;

  const static int signal_peaks = 4;

#if 0
  struct signal_peaks{

    float sigma_ = 1.0f;
    float mu_

  } peaks[signal_peaks];
#endif

  // Central peak at 0, Second peak on the right, Peak on the left, Large-scale peak

  float sigma_[signal_peaks] = {1.0f, 0.5f, 0.6f, 0.8f};
  float separations_[signal_peaks] = {0.0f, 6.1f, -12.5f, 200.0f};
  float signal_scale_[signal_peaks] = {1.5f, 1.5f, 1.5f, 1.5f};

  // Internal data

  int random_seed_ = 4910;
  std::uniform_real_distribution<> rand_;

  int noise_samples_ = 2048;
  float *random_noise_;

  absorption_lines(size_t points_per_interval = 2000)
      : QwtSyntheticPointData(points_per_interval),
        rand_(-0.5f, 0.5f){

    random_noise_ = new float[noise_samples_];
    generate_noise();
  }

  ~absorption_lines(){
    delete[] random_noise_;
  }

  /**
   * Generate a batch of random noise
   */
  void generate_noise(){
    // Initialize Random Number Generator
#if 1
    std::random_device rd;
    std::mt19937 generator(rd());
#else
    std::mt19937 generator(random_seed_);
    ++random_seed_;
#endif

    for(int i = 0; i < noise_samples_; ++i){
      random_noise_[i] = (float) rand_(generator);
    }

  }

  double gauss(double x, float sigma, float mu) const{
    using namespace std;
    float sigma_sq = 2 * sigma * sigma;
    float x_m_mu = float(x) - mu;
    return exp(-x_m_mu * x_m_mu / sigma_sq) / (sigma * M_PI);
  }

  void set_x_shift(float shift){
    x_shift_ = shift;
    generate_noise();
  }

  void set_scale(float scale){

    scale_ = scale;
    generate_noise();
  }

  double y(double x) const{

    float y = _height; // We want absorption

    if(has_signal_){
      // Add x-shift for measured signal
      x += x_shift_;

      /**
       * The signal peaks
       */

      for(int i = 0; i < signal_peaks; ++i){
        y -= signal_scale_[i] * gauss(x, sigma_[i], separations_[i]);
      }
    }

    // Add some noise if it's enabled
    if(noise_){

      // We add 1 here to make sure we get a positive number
      int sample = int(noise_samples_ * (fabs(x + scale_) / (2 * scale_)));

      // then we roll it over
      sample %= noise_samples_;

      y += 0.1 * random_noise_[sample];
    }
    return y;
  }
};

#endif // ABSORPTION_LINES
