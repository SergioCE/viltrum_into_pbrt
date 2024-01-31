#include "iostream"
#include "../functions/functions.h"
#include <viltrumDyadic/viltrum.h>
#include "viltrumDyadic/utils/cimg-wrapper.h"
#include "viltrumDyadic/quadrature/dyadic-nets.h"

int main(){
    int w = 100;
    int h = 100;

    int spp = 4;
    vector<array<float,2>> dims;
    dims.push_back({1,2});
    viltrum::CImgWrapper<float> image(w,h);

    auto integrator_bins = viltrum::integrator_bins_stepper(viltrum::stepper_bins_per_bin(viltrum::stepper_monte_carlo_dyadic_uniform(dims,spp)),spp);
    auto range = viltrum::range_all<3>(-1.0,1.0);

    integrator_bins.integrate(image,image.resolution(),viltrum::function_wrapper(sphere), range);
    
    std::stringstream filename;
	filename<<"Sphere.hdr";
	image.save(filename.str());

    image.print();
    return 0;
}