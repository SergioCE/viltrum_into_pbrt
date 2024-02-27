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

using namespace viltrum;

int main(int argc, char *argv[]){

    std::cout << "Flags:\n\t-f: pbrt scene filepath\n\t-s: Samples per pixel" << std::endl;

    int scene;
    int spp;
    int mc_spp;
    int cv;
    int var_red;
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
            else if (argv[i][1] == 'm' && argv[i][2] == 'c') {
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (montecarlo) value: " << value << std::endl;
                mc_spp = atoi(argv[i + 1]);
            }
            else if (argv[i][1] == 'c' && argv[i][2] == 'v') {
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (control var) value: " << value << std::endl;
                cv = atoi(argv[i + 1]);
            }
            else if (argv[i][1] == 'v') {
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (technique) value: " << value << std::endl;
                var_red = atoi(argv[i + 1]);
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

    int numDim = 4;
    //Dimensions are chosen in pairs of two
    //If get<0> is 2 the third getPixel2D will be the control variate dims
    //If get<0>==2 && get<1>==1 the third getPixel2D will get the second pair of the control variate (Third and fourth dimensions)
    std::vector<std::tuple<int,int>> chosen_dims;
    chosen_dims.push_back(std::make_tuple(0,0));
    chosen_dims.push_back(std::make_tuple(1,1));
    chosen_dims.push_back(std::make_tuple(2,1));


    spp = spp / mc_spp;
    const int dim = 4;
    int bins = pbrt.resolution.x*pbrt.resolution.y;
    unsigned long iteration = std::max(1UL,(unsigned long)(spp*bins/(2*std::pow(3, dim-1)*cv)));
    unsigned long spp_residual = std::max(1UL,(unsigned long)(spp*mc_spp*(cv-1)/cv));
    if(var_red == 0){
        LoggerProgress logger("rr_uniform, alpha=optimized");
        std::cout<<"rr_uniform, alpha=optimized parallel: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_uniform_region(),cv_optimize_weight(),region_sampling_uniform(),spp_residual),
        sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.0f,1.0f),logger);
    }
    else if(var_red == 1){
        LoggerProgress logger("rr_uniform, alpha=0");
        std::cout<<"rr_uniform, alpha=0 parallel: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_uniform_region(),cv_fixed_weight(0),region_sampling_uniform(),spp_residual),
        sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.0f,1.0f),logger);
    }
    else if(var_red == 2){
        LoggerProgress logger("rr_uniform, alpha=1");
        std::cout<<"rr_uniform, alpha=opt parallel: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_uniform_region(),cv_fixed_weight(1),region_sampling_uniform(),spp_residual),
        sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.0f,1.0f),logger);
    }


    else if(var_red == 3){ 
        LoggerProgress logger("rr_integral - Optimized alpha");
        std::cout<<"rr_integral - Optimized alpha: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_integral_region(),cv_optimize_weight(),region_sampling_uniform(),spp_residual),
            sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.f,0.f,1.f,1.f),logger);
    }
    else if(var_red == 4){
        LoggerProgress logger("rr_integral - alpha=0");
        std::cout<<"rr_integral - alpha=0: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_integral_region(),cv_fixed_weight(0),region_sampling_uniform(),spp_residual),
            sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.f,0.f,1.f,1.f),logger);
    }
    else if(var_red == 5){
        LoggerProgress logger("rr_integral - alpha=1");
        std::cout<<"rr_integral - alpha=1: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_integral_region(),cv_fixed_weight(1),region_sampling_uniform(),spp_residual),
            sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.f,0.f,1.f,1.f),logger);
    }


    else if(var_red == 6){ 
        LoggerProgress logger("rr_error_region - Optimized alpha");
        std::cout<<"rr_error_region - Optimized alpha: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_error_region(),cv_optimize_weight(),region_sampling_uniform(),spp_residual),
            sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.f,0.f,1.f,1.f),logger);
    }
    else if(var_red == 7){
        LoggerProgress logger("rr_error_region - alpha=0");
        std::cout<<"rr_error_region - alpha=0: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_error_region(),cv_fixed_weight(0),region_sampling_uniform(),spp_residual),
            sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.f,0.f,1.f,1.f),logger);
    }
    else if(var_red == 8){
        LoggerProgress logger("rr_error_region - alpha=1");
        std::cout<<"rr_error_region - alpha=1: "<<spp*mc_spp<<" samples per pixel."<<std::endl;
        integrate(
            integrator_adaptive_fubini_variance_reduction_parallel<dim>(
                    nested(simpson,trapezoidal),error_heuristic_default(error_metric_absolute()),iteration,mc_spp,
                    rr_error_region(),cv_fixed_weight(1),region_sampling_uniform(),spp_residual),
            sol,renderPbrt_parallel(integrator, pbrt.camera, pbrt.sampler, pbrt.spp, pbrt.resolution, pbrt.s_buffers, numDim, chosen_dims),range_infinite(0.f,0.f,1.f,1.f),logger);
    }


    string name = get_image_name(pbrt);
    std::string filename = output;
    
    save_image_hdr(filename, sol);

    return 0;
    
}

