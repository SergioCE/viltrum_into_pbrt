#include "iostream"
#include "../utils/spheres.h"
#include <viltrum/viltrum.h>
#include "viltrum/utils/cimg-wrapper.h"
#include "viltrum/quadrature/monte-carlo.h"

using namespace std;

int main(){
    int w = 500;
    int h = 500;
    viltrum::CImgWrapper<double> image(w,h);
    

    pbrt::IndependentSampler sampler(1024);

    

    auto integrator_bins = viltrum::integrator_bins_monte_carlo_uniform(100000000); //Probar con más samples
    auto range = viltrum::range_all<3>(-1.0,1.0);

    integrator_bins.integrate(image,image.resolution(),SphereViltrumPbrt<3>(sampler), range);
    
    std::stringstream filename;
	filename<<"Sphere.hdr";
	image.save(filename.str());

    image.print();
    return 0;
}
