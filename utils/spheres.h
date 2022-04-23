#pragma once


#include "viltrum_into_pbrt.h"



SpectrumVilt sphere(double x, double y, double z){
    if(x*x + y*y + z*z < 1) return SpectrumVilt(1,1,1);
    else return SpectrumVilt(0,0,0); 
}

template<std::size_t N>
SpectrumVilt sphereSamplerPbrt(ViltrumSamplerPbrt<N> &sampler){
    if(pow(sampler.Get1D(),2) + pow(sampler.Get1D(),2) + pow(sampler.Get1D(),2) < 1) {
        //std::cout<<sampler.Get1D()<<std::endl;
        return SpectrumVilt(sampler.Get1D(),sampler.Get1D(),sampler.Get1D());
    }
    else return SpectrumVilt(0,0,0); 
}


template<std::size_t N>
class SphereViltrumPbrt {                   //Wrapper para la función sphere
    public:
    SpectrumVilt operator()(const std::array<double,N>& x) const {
        ViltrumSamplerPbrt<N> sampler(x, samplerP);

        return sphereSamplerPbrt(sampler);
    }
    SphereViltrumPbrt(pbrt::IndependentSampler &sampler) : samplerP(sampler){}

    private:
    pbrt::IndependentSampler &samplerP;
};
