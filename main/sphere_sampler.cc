#include "iostream"
#include "../functions/functions.h"
#include <viltrum/viltrum.h>
#include "viltrum/utils/cimg-wrapper.h"
#include "viltrum/quadrature/monte-carlo.h"

using namespace std;

int main(){
    int w = 500;
    int h = 500;
    viltrum::CImgWrapper<double> image(w,h);
    

    random_device rd;
    default_random_engine eng(rd());
    uniform_real_distribution<double> distr(0,1);

    //SphereViltrum<3> esfera(eng,distr);

    auto integrator_bins = viltrum::integrator_bins_monte_carlo_uniform(100000000); //Probar con más samples
    auto range = viltrum::range_all<6>(-1.0,1.0);

    integrator_bins.integrate(image,image.resolution(),SphereViltrum<6>(eng,distr), range);
    
    std::stringstream filename;
	filename<<"Sphere.hdr";
	image.save(filename.str());

    image.print();
    return 0;
}
