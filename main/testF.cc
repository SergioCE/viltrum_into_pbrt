
#include "../utils/tracers.h"
#include "../utils/parsePbrt.h"
#include <viltrum/viltrum.h>
#include "viltrum/utils/cimg-wrapper.h"
#include "viltrum/quadrature/monte-carlo.h"
#include <pbrt/shapes.h>
#include <pbrt/materials.h>

int main(int argc, char *argv[]){

    std::vector<std::string> filenames = getScene(argc,argv);
    
    BasicScene scenePbrt;
    BasicSceneBuilder builder(&scenePbrt);
    ParseFiles(&builder, filenames);
    
    int w = 400;
    int h = 400;
    viltrum::CImgWrapper<double> image(w,h);

    Primitive shapes = getPrimitives(scenePbrt);



    pbrt::IndependentSampler sampler(1024);


    auto integrator_bins = viltrum::integrator_bins_monte_carlo_uniform(w*h*100); //Probar con más samples
    auto range = viltrum::range_all<10>(0.0,1.0);
    integrator_bins.integrate(image,image.resolution(),renderPbrt<10>(sampler, scenePbrt.GetCamera(), shapes, Point2i(w,h)), range);
    std::stringstream filename;
	filename<<"image.hdr";
	image.save(filename.str());
    cout<<"\nDoing"<<endl;
    image.print();
    return 0;
}