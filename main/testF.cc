
#include "../utils/tracers.h"
#include "../utils/parsePbrt.h"
#include <viltrum/viltrum.h>
#include "viltrum/utils/cimg-wrapper.h"
#include "viltrum/quadrature/monte-carlo.h"
#include <pbrt/shapes.h>
#include <pbrt/materials.h>
#include <cmath>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

bool checkIntegrator(pbrt::Integrator* integrator);


int main(int argc, char *argv[]){
    
    std::vector<std::string> filenames = getScene(argc,argv);
    
    BasicScene scenePbrt;
    BasicSceneBuilder builder(&scenePbrt);
    ParseFiles(&builder, filenames);
    
    pbrt::Camera camera = scenePbrt.GetCamera();
    pbrt::Sampler sampler = scenePbrt.GetSampler();

    pbrt::Point2i resolution = camera.GetFilm().FullResolution();
    int spp = sampler.SamplesPerPixel();

    viltrum::CImgWrapper<double> image(resolution.x,resolution.y);

    Primitive shapes = getPrimitives(scenePbrt);

    pbrt::ScratchBuffer scratchBuffer(65536);


    NamedTextures textures = scenePbrt.CreateTextures();
    std::map<int, pstd::vector<Light> *> shapeIndexToAreaLights;
    std::vector<Light> lights =
        scenePbrt.CreateLights(textures, &shapeIndexToAreaLights);
    std::map<std::string, pbrt::Material> namedMaterials;
    std::vector<pbrt::Material> materials;
    scenePbrt.CreateMaterials(textures, &namedMaterials, &materials);
    
    std::unique_ptr<pbrt::Integrator> integratorP(
        scenePbrt.CreateIntegrator(camera, sampler, shapes, lights));
    
    if(!checkIntegrator(integratorP.get())){
        return 0;
    }
    else{
        pbrt::RayIntegrator* rayInt = dynamic_cast<pbrt::RayIntegrator*>(integratorP.get());

        //auto integrator_bins = viltrum::integrator_bins_monte_carlo_uniform(resolution.x*resolution.y*spp); //Probar con más samples          MC
        auto integrator_bins = viltrum::integrator_bins_per_bin(viltrum::integrator_monte_carlo_uniform(spp)); //Modificar integrator_monte_carlo_uniform
        auto range = viltrum::range_all<3>(0.0,1.0);
        integrator_bins.integrate(image,image.resolution(),renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer), range);




        /*int w = resolution.x;
        int h = resolution.y;
        float cv_rate = 0.5;
        unsigned long max_spp = 128;
        unsigned long spp_pixel = 4;
        float error_rate = 1.e-5f;
        std::size_t seed = std::random_device()();
        int slice_at_column = -1;
        unsigned long spp_cv =spp-1;

        integrator_optimized_adaptive_stratified_control_variates(viltrum::trapezoidal,viltrum::trapezoidal,viltrum::error_single_dimension_size(error_rate), (spp_cv*w*h)/(3*3*2), spp - spp_cv, seed).integrate(image,image.resolution(),renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer), range);
        */

        std::stringstream filename;
        filename<<"image3.hdr";
        image.save(filename.str());
        cout<<"\nDoing"<<endl;
        image.print();
        return 0;
    }
    
}



bool checkIntegrator(pbrt::Integrator* integrator){
    string str = integrator->ToString();

    string name = "";
    int i=0;
    for (auto x : str) 
    {
        if (x == ' ')
        {
            i++;
            if(i==2) break;
            name = "";
        }
        else {
            name = name + x;
        }
    }

    string fail = "Integrator type " + name + " not permited, use only rayIntegrators:\n\tRandomWalkIntegrator\n\tSimplePathIntegrator\n\tPathIntegrator\n\tSimpleVolPathIntegrator\n\tVolPathIntegrator\n\tAOIntegrator\n\tBDPTIntegrator";

    int comp = name.compare("RandomWalkIntegrator") + name.compare("SimplePathIntegrator") + name.compare("PathIntegrator") + name.compare("SimpleVolPathIntegrator")
         + name.compare("VolPathIntegrator") + name.compare("AOIntegrator") + name.compare("BDPTIntegrator");

    if(comp > 0) return true;
    else{
        cout<<fail<<endl;
        return false;
    }

}
