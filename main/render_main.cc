
#include "../utils/tracers.h"
#include "../utils/tracers_parallel.h"
#include "../utils/parsePbrt.h"
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

    unsigned t0, t1;
 
    t0=clock();
    auto start = std::chrono::high_resolution_clock::now();
    



    if(!checkIntegrator(integratorP.get())){
        return 0;
    }
    else{
        string sum = "";
        pbrt::RayIntegrator* rayInt = dynamic_cast<pbrt::RayIntegrator*>(integratorP.get());

        auto range = viltrum::range_all<4>(0.0,1.0);
        //int option = 0;

        int bins = resolution.x*resolution.y;

        if(option == 0){
            //auto integrator_bins = viltrum::integrator_bins_stepper(viltrum::stepper_bins_per_bin(viltrum::stepper_monte_carlo_uniform()),spp);
            sum += "MC";
            cout<<sum<<endl;
            std::vector<std::vector<SpectrumVilt>> sol(resolution.x,std::vector<SpectrumVilt>(resolution.y,SpectrumVilt(0.0f)));
            viltrum::LoggerProgress logger("Monte-Carlo parallel");
            
            unsigned int numThreads = std::thread::hardware_concurrency();
            std::vector<pbrt::ScratchBuffer> s_buffers;
            if (numThreads == 0) {
                std::cout << "Unable to determine the number of threads supported by the hardware." << std::endl;
            } else {
                std::cout << "Number of concurrent threads supported by the hardware: " << numThreads << std::endl;
            }
            for(int i=0;i<numThreads; i++){
                s_buffers.emplace_back();
            }   

            integrate(viltrum::integrator_per_bin_parallel(viltrum::monte_carlo(spp)),sol,renderPbrt_parallel(rayInt, camera, sampler, spp, resolution, s_buffers, true),viltrum::range_primary<4>(),logger);
            std::cout<<"finished"<<std::endl;

            viltrum::LoggerProgress logger2("Monte-Carlo");
            integrate(viltrum::integrator_per_bin(viltrum::monte_carlo(spp)),sol,renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer, true),viltrum::range_primary<4>(),logger2);
            std::cout<<"finished"<<std::endl;
        }

       //string name = camera.GetFilm().GetFilename();
       //int x = sizeof(name);
       // for(int i = 0; i < x; i++) {
       //     if(name[i] == '.') {
       //         name = name.substr(0, i);
       //         break;
       //     }
       // }

       // t1 = clock();
       // auto end = std::chrono::high_resolution_clock::now();
 
       // auto int_s = std::chrono::duration_cast<std::chrono::seconds>(end - start);
       // double time = (double(t1-t0)/CLOCKS_PER_SEC);
       // cout << "Execution Time: " << time << endl;
       // cout << "Execution Time 2: " << int_s.count() << endl;


       // std::string filename = name + sum + to_string(spp) + ".hdr";
       // //filename<<"image3.hdr";
       // cout<<"Generated image "<<filename<<endl;
       // image.save(filename);
       // cout<<"\nDoing"<<endl;
       // image.print();
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
