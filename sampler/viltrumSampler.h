#pragma once

#include <iostream>
#include <array>
#include <random>

using namespace std;


template<std::size_t N>
class ViltrumSampler {                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSampler(const std::array<double,N>& x, default_random_engine eng, 
        uniform_real_distribution<double> distr) : eng(eng), distr(distr), i(0), v(x){}

    double Get1D() { 
        if(i < v.size()) return v[i++]; 
        else return distr(eng);
    }
    
    //Point2f Get2D() { return {Get1D(), Get1D()}; }
    
    //Point2f GetPixel2D() { return Get2D(); }

  private:
  default_random_engine eng;
    uniform_real_distribution<double> distr;
    std::size_t i;
    std::array<double,N> v;
};