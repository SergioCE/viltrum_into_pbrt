#pragma once

#include <Eigen/Dense>
#include "../sampler/viltrumSampler.h"
#include <math.h>
#include <random>

using namespace std;

#ifndef Spectrum
using Spectrum = Eigen::Array3f;
#endif

Spectrum sphere(double x, double y, double z){
    if(x*x + y*y + z*z < 1) return Spectrum(1,1,1);
    else return Spectrum(0,0,0); 
}

template<std::size_t N>
Spectrum sphereSampler(ViltrumSampler<N> sampler){
    if(pow(sampler.Get1D(),2) + pow(sampler.Get1D(),2) + pow(sampler.Get1D(),2) < 1) return Spectrum(sampler.Get1D(),sampler.Get1D(),sampler.Get1D());
    else return Spectrum(0,0,0); 
}


template<std::size_t N>
class SphereViltrum {
    public:
    Spectrum operator()(const std::array<double,N>& x) const {
        ViltrumSampler<N> sampler(x, eng, distr);

        return sphereSampler(sampler);
    }
    SphereViltrum(std::default_random_engine eng, 
        uniform_real_distribution<double> distr) : eng(eng), distr(distr){}

    private:
    default_random_engine eng;
    uniform_real_distribution<double> distr;
};
