#pragma once

#include "../utils/viltrum_into_pbrt.h"
#include <pbrt/src/pbrt/samplers.h>

using namespace std;



template<std::size_t N>
class ViltrumSamplerPbrt {                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSamplerPbrt(const std::array<double,N>& x, pbrt::IndependentSampler &sampler) : sampler(sampler), i(0), v(x){}

    pbrt::Float Get1D() { 
        if(i < v.size()) return v[i++]; 
        else return sampler.Get1D();
    }
    
    pbrt::Point2f Get2D() { return {Get1D(), Get1D()}; }
    
    pbrt::Point2f GetPixel2D() { return Get2D(); }

  private:
    pbrt::IndependentSampler &sampler;                    //Sampler por referencia (todo aleatorios) o por copia (mismos siempre)
    std::size_t i;
    std::array<double,N> v;
};