
#include "../utils/tracers.h"
#include "../utils/parsePbrt.h"
#include <viltrumDyadic/viltrum.h>
#include "viltrumDyadic/utils/cimg-wrapper.h"
#include "viltrumDyadic/quadrature/monte-carlo.h"
#include "viltrumDyadic/quadrature/dyadic-nets.h"
#include <pbrt/shapes.h>
#include <pbrt/materials.h>
#include <cmath>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <time.h>
#include <chrono>

bool checkIntegrator(pbrt::Integrator* integrator);


int main(int argc, char *argv[]){

    if(argc < 3){
        std::cout<<"Please select an integration option"<<std::endl;
        return 0;
    }

    int option = atoi(argv[1]);
    
    std::vector<std::string> filenames = getScene(argc-1,&argv[1]);
    
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
    cout<<resolution[0]<<","<<resolution[1]<<endl;
    int spp = sampler.SamplesPerPixel();

    viltrum::CImgWrapper<float> image(resolution.x,resolution.y);


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

    unsigned t0, t1;
 
    t0=clock();
    auto start = std::chrono::high_resolution_clock::now();
    



    if(!checkIntegrator(integratorP.get())){
        return 0;
    }
    else{
        string sum = "";
        pbrt::RayIntegrator* rayInt = dynamic_cast<pbrt::RayIntegrator*>(integratorP.get());

        auto range = viltrum::range_all<4>(0.0f,1.0f);
        //int option = 0;

        if(option == 0){
            auto integrator_bins = viltrum::integrator_bins_stepper(viltrum::stepper_bins_per_bin(viltrum::stepper_monte_carlo_uniform()),spp);
            sum += "MC";
            cout<<sum<<endl;
            integrator_bins.integrate(image,image.resolution(),renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer, true), range);
            std::cout<<"a"<<std::endl;
        }
        else if(option == 1){
            //Dimensions integrated with dyadic nets
            vector<array<float,2>> dims;
            dims.push_back({2,3});      //Primary space   
            dims.push_back({4,5});      //First intersection
            string dyadic_nets_folder = "../external/viltrumDyadic/utils/Blue-Nets/";
            auto integrator_bins = viltrum::integrator_bins_stepper(viltrum::stepper_bins_per_bin(viltrum::stepper_monte_carlo_dyadic_uniform(dims,spp,dyadic_nets_folder)),spp);
            sum += "DMC";
            cout<<sum<<endl;
            integrator_bins.integrate(image,image.resolution(),renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer, true), range);

        }
        else{
            bool _2dOnly = true;     //Only get samples when pbrt ask for 2 samples (dimensions 2,3 and 4,5)
            int repeatedDim = 2;    //Dims 2,3 will be just like 4,5
            int dim = 4;
            int bins = resolution.x*resolution.y;
            unsigned long spp_cv = std::max(1UL,(unsigned long)(spp*(1.0/16.0)));
            auto integrator_bins = integrator_optimized_perpixel_adaptive_stratified_control_variates(
                viltrum::nested(viltrum::simpson,viltrum::trapezoidal), // nested rule, order 2 polynomials
                viltrum::error_single_dimension_size(1.e-5), // error heuristic
                spp_cv*bins/(2*std::pow(3, dim-1)), // number of adaptive iterations calculated from the spps
                std::max(1UL,spp-spp_cv) // number of spps for the residual
            );
            sum += "CV4D";
            cout<<sum<<endl;
            integrator_bins.integrate(image,image.resolution(),renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer, _2dOnly, repeatedDim), range);

        }
        //
        
        
        
        
        //dims.push_back({10,11});

        //dims.push_back({4,5});
        //dims.push_back({6,7});

        /*
        dims.push_back({8,9});            //PATH   Cornell box
        //dims.push_back({9,10});            //PATH   ESCENA NEGRA          DIMS SIN GETSAMPLER
        
        dims.push_back({14,15});          //VOLPATH
        */
        

        
        
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

        t1 = clock();
        auto end = std::chrono::high_resolution_clock::now();
 
        auto int_s = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        double time = (double(t1-t0)/CLOCKS_PER_SEC);
        cout << "Execution Time: " << time << endl;
        cout << "Execution Time 2: " << int_s.count() << endl;


        std::string filename = name + sum + to_string(spp) + ".hdr";
        //filename<<"image3.hdr";
        cout<<"Generated image "<<filename<<endl;
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
