#include "../utils/tracers.h"
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

    std::cout << "Flags:\n\t-f: pbrt scene filepath\n\t-i: Adaptive iterations" << std::endl;

    int scene;
    int iter;
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
            else if (argv[i][1] == 'i') {
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (iterations) value: " << value << std::endl;
                iter = atoi(argv[i + 1]);
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
    viltrum::LoggerProgress logger("Adaptive quadrature rules parallel");
    std::cout<<"Adaptive quadrature rules parallel: "<<iter<<" iterations."<<std::endl;
    string int_tech = "Adaptive";
    integrate(viltrum::integrator_adaptive_iterations_parallel(viltrum::nested(viltrum::simpson,viltrum::trapezoidal),iter),sol,
        renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, true, 2),viltrum::range_primary<4>(),logger);


    string name = get_image_name(pbrt);
    std::string filename = name + int_tech + to_string(iter) + ".hdr";
    
    save_image_hdr(filename, sol);

    return 0;
    
}




