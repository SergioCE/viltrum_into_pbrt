#pragma once
#include "tracers.h"
#include "parsePbrt.h"
#include <pbrt/shapes.h>
#include <pbrt/materials.h>
#include <thread>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <time.h>


//Class to prepare a PBRT render

bool checkIntegrator(pbrt::Integrator* integrator);

class PreparePBRT{

    public:
        std::unique_ptr<Integrator> integratorP;
        //pbrt::RayIntegrator* integrator;
        pbrt::Camera camera;
        pbrt::Sampler sampler;
        int spp;
        pbrt::Point2i resolution;
        pbrt::ScratchBuffer scratchBuffer;
        std::vector<pbrt::ScratchBuffer> s_buffers;
        BasicScene* scenePbrt;
        BasicSceneBuilder* builder;

        PreparePBRT(int argc, char *argv[], bool parallel){
            std::vector<std::string> filenames = getScene(argc,argv);
    
            scenePbrt = new BasicScene();
            builder = new BasicSceneBuilder(scenePbrt);
            //BasicSceneBuilder builder(&scenePbrt);
            ParseFiles(builder, filenames);
            
            // Create media first (so have them for the camera...)
            std::map<std::string, Medium> media = scenePbrt->CreateMedia();

            NamedTextures textures = scenePbrt->CreateTextures();

            // Lights
            std::map<int, pstd::vector<Light> *> shapeIndexToAreaLights;
            std::vector<Light> lights =
                scenePbrt->CreateLights(textures, &shapeIndexToAreaLights);

            
            std::map<std::string, pbrt::Material> namedMaterials;
            std::vector<pbrt::Material> materials;
            scenePbrt->CreateMaterials(textures, &namedMaterials, &materials);


            Primitive accel = scenePbrt->CreateAggregate(textures, shapeIndexToAreaLights, media,
                                                        namedMaterials, materials);

            camera = scenePbrt->GetCamera();
            Film film = camera.GetFilm();
            sampler = scenePbrt->GetSampler();

            integratorP =std::unique_ptr<Integrator>(
                scenePbrt->CreateIntegrator(camera, sampler, accel, lights));


            resolution = camera.GetFilm().FullResolution();
            spp = sampler.SamplesPerPixel();

            // Helpful warnings
            bool haveScatteringMedia = false;
            for (const auto &sh : scenePbrt->shapes)
                if (!sh.insideMedium.empty() || !sh.outsideMedium.empty())
                    haveScatteringMedia = true;
            for (const auto &sh : scenePbrt->animatedShapes)
                if (!sh.insideMedium.empty() || !sh.outsideMedium.empty())
                    haveScatteringMedia = true;

            if (haveScatteringMedia && scenePbrt->integrator.name != "volpath" &&
                scenePbrt->integrator.name != "simplevolpath" &&
                scenePbrt->integrator.name != "bdpt" && scenePbrt->integrator.name != "mlt")
                Warning("Scene has scattering media but \"%s\" integrator doesn't support "
                        "volume scattering. Consider using \"volpath\", \"simplevolpath\", "
                        "\"bdpt\", or \"mlt\".",
                        scenePbrt->integrator.name);

            bool haveLights = !lights.empty();
            for (const auto &m : media)
                haveLights |= m.second.IsEmissive();

            if (!haveLights && scenePbrt->integrator.name != "ambientocclusion" &&
                scenePbrt->integrator.name != "aov")
                Warning("No light sources defined in scene; rendering a black image.");

            if (film.Is<GBufferFilm>() && !(scenePbrt->integrator.name == "path" ||
                                            scenePbrt->integrator.name == "volpath"))
                Warning("GBufferFilm is not supported by the \"%s\" integrator. The channels "
                        "other than R, G, B will be zero.",
                        scenePbrt->integrator.name);

            bool haveSubsurface = false;
            for (pbrt::Material mtl : materials)
                haveSubsurface |= mtl && mtl.HasSubsurfaceScattering();
            for (const auto &namedMtl : namedMaterials)
                haveSubsurface |= namedMtl.second && namedMtl.second.HasSubsurfaceScattering();

            if (haveSubsurface && scenePbrt->integrator.name != "volpath")
                Warning("Some objects in the scene have subsurface scattering, which is "
                        "not supported by the %s integrator. Use the \"volpath\" integrator "
                        "to render them correctly.",
                        scenePbrt->integrator.name);

            LOG_VERBOSE("Memory used after scene creation: %d", GetCurrentRSS());


            unsigned int numThreads = std::thread::hardware_concurrency();
            if(parallel){
                if (numThreads == 0) {
                    std::cout << "Unable to determine the number of threads supported by the hardware." << std::endl;
                } else {
                    std::cout << "Number of concurrent threads supported by the hardware: " << numThreads << std::endl;
                }
                for(int i=0;i<numThreads; i++){
                    s_buffers.emplace_back(pbrt::ScratchBuffer());
                }  
            }
            
        }


        std::vector<pbrt::ScratchBuffer>& get_scratch_buffers_parallel(){
            return s_buffers;
        }

        pbrt::ScratchBuffer& get_scratch_buffer(){
            return scratchBuffer;
        }

        ~PreparePBRT() { delete scenePbrt; delete builder; }

};

string get_image_name(PreparePBRT& pbrt){
    string name = pbrt.camera.GetFilm().GetFilename();
    int x = sizeof(name);
    for(int i = 0; i < x; i++) {
        if(name[i] == '.') {
            name = name.substr(0, i);
            break;
        }
    }
    return name;
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