#include "../utils/tracers_parallel.h"
#include "../utils/preparePBRT.h"
#include "../utils/save_image.h"
#include <viltrum/viltrum.h>
#include <viltrum/utils/cimg-wrapper.h>
#include <pbrt/shapes.h>
#include <pbrt/materials.h>
#include <cmath>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <time.h>
#include <chrono>


int main(int argc, char *argv[]){

    std::cout << "Flags:\n\t-f: pbrt scene filepath\n\t-s: Samples per pixel" << std::endl;

    int scene;
    int spp;
    std::string output;
    // Process the command line arguments
    for (int i = 1; i < argc; i += 2) {
        // Check if the current argument is a flag
        if (argv[i][0] == '-') {
            // Process the flag and its corresponding value
            std::string flag = argv[i];
            if (argv[i][1] == 'f') {
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (filename) value: " << value << std::endl;
                scene = i+1;
            }
            else if (argv[i][1] == 's') {
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (spp) value: " << value << std::endl;
                spp = atoi(argv[i + 1]);
            }
            else if (argv[i][1] == 'o') {
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (output) value: " << value << std::endl;
                output = argv[i + 1];
            }
            else {
                std::cerr << "Missing value for flag " << flag << std::endl;
                return 1;  // Return an error code
            }
        } else {
            // Error: Expected a flag but found an argument without a flag
            std::cerr << "Error: Expected a flag but found argument without a flag." << std::endl;
            return 1;  // Return an error code
        }
    }

    //True if the integration technique is parallel
    bool parallel = true;
    PreparePBRT pbrt(1,&argv[scene],parallel);
    pbrt::RayIntegrator* integrator = dynamic_cast<pbrt::RayIntegrator*>(pbrt.integratorP.get());

    //Solution image
    cout<<"\nImage size: "<<pbrt.resolution.x<<"x"<<pbrt.resolution.y<<std::endl;
    std::vector<std::vector<SpectrumVilt>> sol(pbrt.resolution.x,std::vector<SpectrumVilt>(pbrt.resolution.y,SpectrumVilt(0.0f)));

    //Integration technique
    viltrum::LoggerProgress logger("Adaptive Control Variates parallel");
    std::cout<<"Adaptive Control Variates parallel: "<<spp<<" samples per pixel."<<std::endl;
    string int_tech = "CV";

    const int dim = 4;
    unsigned long spp_cv = std::max(1UL,(unsigned long)(spp*(1.0/16.0)));
    int bins = pbrt.resolution.x*pbrt.resolution.y;
    unsigned long iteration = spp_cv*bins/(2*std::pow(3, dim-1));
    integrate(viltrum::integrator_fubini<4>(viltrum::integrator_adaptive_control_variates_parallel(viltrum::nested(viltrum::simpson,viltrum::trapezoidal),iteration,std::max(1UL,spp-spp_cv)),viltrum::monte_carlo(1)),
        sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers),viltrum::range_infinite(0.0,0.0,1.0,1.0),logger);


    string name = get_image_name(pbrt);
    std::string filename = output;
    
    save_image_hdr(filename, sol);

    return 0;
    
}




