#pragma once

#include <Eigen/Dense>

#ifndef Spectrum
using Spectrum = Eigen::Array3f;
#endif

Spectrum sphere(float x, float y, float z){
    if(x*x + y*y + z*z < 1) return Spectrum(1,1,1);
    else return Spectrum(0,0,0); 
}