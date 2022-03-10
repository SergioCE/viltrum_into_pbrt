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
Spectrum sphereSamplerPbrt(ViltrumSamplerPbrt<N> &sampler){
    if(pow(sampler.Get1D(),2) + pow(sampler.Get1D(),2) + pow(sampler.Get1D(),2) < 1) {
        //std::cout<<sampler.Get1D()<<std::endl;
        return Spectrum(sampler.Get1D(),sampler.Get1D(),sampler.Get1D());
    }
    else return Spectrum(0,0,0); 
}


template<std::size_t N>
class SphereViltrumPbrt {                   //Wrapper para la función sphere
    public:
    Spectrum operator()(const std::array<double,N>& x) const {
        ViltrumSamplerPbrt<N> sampler(x, samplerP);

        return sphereSamplerPbrt(sampler);
    }
    SphereViltrumPbrt(pbrt::IndependentSampler &sampler) : samplerP(sampler){}

    private:
    pbrt::IndependentSampler &samplerP;
};
