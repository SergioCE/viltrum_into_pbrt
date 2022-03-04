#include "iostream"
#include "../functions/functions.h"
#include <viltrum/viltrum.h>
#include "viltrum/utils/cimg-wrapper.h"
#include "viltrum/quadrature/monte-carlo.h"

int main(){
    int w = 500;
    int h = 500;

    viltrum::CImgWrapper<float> image(w,h);

    auto integrator_bins = viltrum::integrator_bins_monte_carlo_uniform(1024);
    auto range = viltrum::range_all<3>(-1.0,1.0);

    integrator_bins.integrate(image,image.resolution(),viltrum::function_wrapper(sphere), range);
    
    std::stringstream filename;
	filename<<"Sphere.hdr";
	image.save(filename.str());

    image.print();
    return 0;
}