
#include "../utils/tracers.h"
#include "../utils/parsePbrt.h"
#include <viltrum/viltrum.h>
#include "viltrum/utils/cimg-wrapper.h"
#include "viltrum/quadrature/monte-carlo.h"
#include "viltrum/quadrature/dyadic-nets.h"
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
    
    // Create media first (so have them for the camera...)
    std::map<std::string, Medium> media = scenePbrt.CreateMedia();

    NamedTextures textures = scenePbrt.CreateTextures();

    // Lights
    std::map<int, pstd::vector<Light> *> shapeIndexToAreaLights;
    std::vector<Light> lights =
        scenePbrt.CreateLights(textures, &shapeIndexToAreaLights);

    
    std::map<std::string, pbrt::Material> namedMaterials;
    std::vector<pbrt::Material> materials;
    scenePbrt.CreateMaterials(textures, &namedMaterials, &materials);


    Primitive accel = scenePbrt.CreateAggregate(textures, shapeIndexToAreaLights, media,
                                                  namedMaterials, materials);

    Camera camera = scenePbrt.GetCamera();
    Film film = camera.GetFilm();
    Sampler sampler = scenePbrt.GetSampler();

    std::unique_ptr<Integrator> integratorP(
        scenePbrt.CreateIntegrator(camera, sampler, accel, lights));
// LO MIO

    pbrt::Point2i resolution = camera.GetFilm().FullResolution();
    int spp = sampler.SamplesPerPixel();

    viltrum::CImgWrapper<double> image(resolution.x,resolution.y);


    pbrt::ScratchBuffer scratchBuffer;

    

    // Helpful warnings
    bool haveScatteringMedia = false;
    for (const auto &sh : scenePbrt.shapes)
        if (!sh.insideMedium.empty() || !sh.outsideMedium.empty())
            haveScatteringMedia = true;
    for (const auto &sh : scenePbrt.animatedShapes)
        if (!sh.insideMedium.empty() || !sh.outsideMedium.empty())
            haveScatteringMedia = true;

    if (haveScatteringMedia && scenePbrt.integrator.name != "volpath" &&
        scenePbrt.integrator.name != "simplevolpath" &&
        scenePbrt.integrator.name != "bdpt" && scenePbrt.integrator.name != "mlt")
        Warning("Scene has scattering media but \"%s\" integrator doesn't support "
                "volume scattering. Consider using \"volpath\", \"simplevolpath\", "
                "\"bdpt\", or \"mlt\".",
                scenePbrt.integrator.name);

    bool haveLights = !lights.empty();
    for (const auto &m : media)
        haveLights |= m.second.IsEmissive();

    if (!haveLights && scenePbrt.integrator.name != "ambientocclusion" &&
        scenePbrt.integrator.name != "aov")
        Warning("No light sources defined in scene; rendering a black image.");

    if (film.Is<GBufferFilm>() && !(scenePbrt.integrator.name == "path" ||
                                    scenePbrt.integrator.name == "volpath"))
        Warning("GBufferFilm is not supported by the \"%s\" integrator. The channels "
                "other than R, G, B will be zero.",
                scenePbrt.integrator.name);

    bool haveSubsurface = false;
    for (pbrt::Material mtl : materials)
        haveSubsurface |= mtl && mtl.HasSubsurfaceScattering();
    for (const auto &namedMtl : namedMaterials)
        haveSubsurface |= namedMtl.second && namedMtl.second.HasSubsurfaceScattering();

    if (haveSubsurface && scenePbrt.integrator.name != "volpath")
        Warning("Some objects in the scene have subsurface scattering, which is "
                "not supported by the %s integrator. Use the \"volpath\" integrator "
                "to render them correctly.",
                scenePbrt.integrator.name);

    LOG_VERBOSE("Memory used after scene creation: %d", GetCurrentRSS());



    if(!checkIntegrator(integratorP.get())){
        return 0;
    }
    else{
        string sum = "";
        pbrt::RayIntegrator* rayInt = dynamic_cast<pbrt::RayIntegrator*>(integratorP.get());

        auto integrator_bins = viltrum::integrator_bins_stepper(viltrum::stepper_bins_per_bin(viltrum::stepper_monte_carlo_uniform()),spp);
        
        auto range = viltrum::range_all<200>(0.0,1.0);
        
        vector<array<float,2>> dims;
        
        dims.push_back({4,5});
        dims.push_back({10,11});


        /*
        dims.push_back({8,9});            //PATH   Cornell box
        //dims.push_back({9,10});            //PATH   ESCENA NEGRA          DIMS SIN GETSAMPLER
        
        dims.push_back({14,15});          //VOLPATH
        */

        //auto integrator_bins = viltrum::integrator_bins_stepper(viltrum::stepper_bins_per_bin(viltrum::stepper_monte_carlo_dyadic_uniform(dims,spp)),spp);
        //sum += "D";
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

       string name = camera.GetFilm().GetFilename();
       int x = sizeof(name);
        for(int i = 0; i < x; i++) {
            if(name[i] == '.') {
                name = name.substr(0, i);
                break;
            }
        }


        std::string filename = name + sum + "MC" + to_string(spp) + ".hdr";
        //filename<<"image3.hdr";
        image.save(filename);
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
