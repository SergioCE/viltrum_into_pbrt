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

    if(argc < 3){
        std::cout<<"Please select an integration option"<<std::endl;
        return 0;
    }

    int option = atoi(argv[1]);
    
    bool parallel = true;
    PreparePBRT pbrt(1,&argv[2],parallel);
    pbrt::RayIntegrator* integrator = dynamic_cast<pbrt::RayIntegrator*>(pbrt.integratorP.get());

    int bins = pbrt.resolution.x*pbrt.resolution.y;

    //Solution
    std::vector<std::vector<SpectrumVilt>> sol(pbrt.resolution.x,std::vector<SpectrumVilt>(pbrt.resolution.y,SpectrumVilt(0.0f)));

    string int_tech = "";

    if(option == 0){
        //Monte Carlo samples are readen from the .pbrt scene file
        
        int_tech += "MC";
        cout<<int_tech<<endl;
        cout<<pbrt.spp<<std::endl;
        
        viltrum::LoggerProgress logger("Monte-Carlo parallel");

        integrate(viltrum::integrator_per_bin_parallel(viltrum::monte_carlo(pbrt.spp)),sol,
            renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, true),viltrum::range_primary<4>(),logger);
        std::cout<<"finished"<<std::endl;

        //Not parallel 
        //viltrum::LoggerProgress logger2("Monte-Carlo");
        //integrate(viltrum::integrator_per_bin(viltrum::monte_carlo(spp)),sol,renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer, true),viltrum::range_primary<4>(),logger2);
        //std::cout<<"finished"<<std::endl;
    }
    else if (option==1){
        int dim = 4;
        int bins = pbrt.resolution.x*pbrt.resolution.y;

        int_tech += "NC";
        cout<<int_tech<<endl;
        

        viltrum::LoggerProgress logger("Parallel trapezoids");

        integrate(viltrum::integrator_newton_cotes_parallel(viltrum::steps<16*2>(viltrum::trapezoidal)),sol,
        renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, true, 2),viltrum::range_primary<4>(),logger);
    }
    else if (option==2){
        //With these parameters we use the newton cotes algorithm in camera space dimensions and direct light dimensions
        int dim = 4;
        int bins = pbrt.resolution.x*pbrt.resolution.y;

        int_tech += "Adaptive";
        cout<<int_tech<<endl;

        viltrum::LoggerProgress logger("Parallel adaptive");

        integrate(viltrum::integrator_adaptive_iterations_parallel(viltrum::nested(viltrum::simpson,viltrum::trapezoidal),200000),sol,
        renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, true, 2),viltrum::range_primary<4>(),logger);
    }


    string name = get_image_name(pbrt);
    std::string filename = name + int_tech + to_string(pbrt.spp) + ".hdr";
    
    save_image_hdr(filename, sol);

    return 0;
    
}

