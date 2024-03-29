
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

    std::cout << "Flags:\n\t-f: pbrt scene filepath\n\t-i: Adaptive iterations" << std::endl;

    int scene;
    int spp;
    int option;
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
                std::cout << "Flag: " << flag << ", (samples) value: " << value << std::endl;
                spp = atoi(argv[i + 1]);
            }
            else if (argv[i][1] == 'o'){
                std::string value = argv[i + 1];
                std::cout << "Flag: " << flag << ", (option) value: " << value << std::endl;
                option = atoi(argv[i + 1]);
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

    
    std::vector<std::string> filenames = getScene(1,&argv[scene]);
    std::cout<<"Parsed"<<std::endl;
    
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
            //Monte Carlo samples are readen from the .pbrt scene file
            auto integrator_bins = viltrum::integrator_bins_stepper(viltrum::stepper_bins_per_bin(viltrum::stepper_monte_carlo_uniform()),spp);
            sum += "MC";
            cout<<sum<<endl;
            integrator_bins.integrate(image,image.resolution(),renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer, true), range);
            
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
