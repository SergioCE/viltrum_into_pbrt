
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
#include <CImg.h>

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


    pbrt::Point2i resolution = camera.GetFilm().FullResolution();
    cout<<resolution[0]<<","<<resolution[1]<<endl;
    int spp = sampler.SamplesPerPixel();


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
        //Here starts viltrum integration

        //For the name of the image
        string sum = "";
        pbrt::RayIntegrator* rayInt = dynamic_cast<pbrt::RayIntegrator*>(integratorP.get());

        auto range = viltrum::range_all<4>(0.0,1.0);

        int bins = resolution.x*resolution.y;

        //Solution
        std::vector<std::vector<SpectrumVilt>> sol(resolution.x,std::vector<SpectrumVilt>(resolution.y,SpectrumVilt(0.0f)));

        //Scratch buffers for parallel algorithms
        unsigned int numThreads = std::thread::hardware_concurrency();
        std::vector<pbrt::ScratchBuffer> s_buffers;
        if (numThreads == 0) {
            std::cout << "Unable to determine the number of threads supported by the hardware." << std::endl;
        } else {
            std::cout << "Number of concurrent threads supported by the hardware: " << numThreads << std::endl;
        }
        for(int i=0;i<numThreads; i++){
            s_buffers.emplace_back(pbrt::ScratchBuffer(32));
        }   

        if(option == 0){
            //Monte Carlo samples are readen from the .pbrt scene file
            
            sum += "MC";
            cout<<sum<<endl;
            
            viltrum::LoggerProgress logger("Monte-Carlo parallel");

            integrate(viltrum::integrator_per_bin_parallel(viltrum::monte_carlo(spp)),sol,renderPbrt_parallel(rayInt, camera, sampler, spp, resolution, s_buffers, true),viltrum::range_primary<4>(),logger);
            std::cout<<"finished"<<std::endl;

            //Not parallel 
            //viltrum::LoggerProgress logger2("Monte-Carlo");
            //integrate(viltrum::integrator_per_bin(viltrum::monte_carlo(spp)),sol,renderPbrt(rayInt, camera, sampler, spp, resolution, scratchBuffer, true),viltrum::range_primary<4>(),logger2);
            //std::cout<<"finished"<<std::endl;
        }
        else if (option==1){
            //With these parameters we use the newton cotes algorithm in camera space dimensions and direct light dimensions
            bool _2dOnly = true;     //Only get samples when pbrt ask for 2 samples (dimensions 2,3 and 4,5)
            int repeatedDim = 2;    //Dims 2,3 will be just like 4,5
            int dim = 4;
            int bins = resolution.x*resolution.y;

            sum += "NC";
            cout<<sum<<endl;

            viltrum::LoggerProgress logger("Parallel trapezoids");

            integrate(viltrum::integrator_newton_cotes_parallel(viltrum::steps<16*2>(viltrum::trapezoidal)),sol,renderPbrt_parallel(rayInt, camera, sampler, spp, resolution, s_buffers, _2dOnly, repeatedDim),viltrum::range_primary<4>(),logger);
        }
        else if (option==2){
            //With these parameters we use the newton cotes algorithm in camera space dimensions and direct light dimensions
            bool _2dOnly = true;     //Only get samples when pbrt ask for 2 samples (dimensions 2,3 and 4,5)
            int repeatedDim = 2;    //Dims 2,3 will be just like 4,5
            int dim = 4;
            int bins = resolution.x*resolution.y;

            sum += "Adaptive";
            cout<<sum<<endl;

            viltrum::LoggerProgress logger("Parallel adaptive");

            integrate(viltrum::integrator_adaptive_iterations_parallel(viltrum::nested(viltrum::simpson,viltrum::trapezoidal),200000),sol,renderPbrt_parallel(rayInt, camera, sampler, spp, resolution, s_buffers, _2dOnly, repeatedDim),viltrum::range_primary<4>(),logger);
        }


       string name = camera.GetFilm().GetFilename();
       int x = sizeof(name);
        for(int i = 0; i < x; i++) {
            if(name[i] == '.') {
                name = name.substr(0, i);
                break;
            }
        }

        cimg_library::CImg<float> image(resolution.x, resolution.y, 1, 3);

        for(int i=0; i<resolution.x; i++){
            for(int j=0; j<resolution.y; j++){
                for(int k=0; k<3; k++){
                    image(i,j,0,k) = sol[i][j][k];
                }
            }
        }

        std::string filename = name + sum + to_string(spp) + ".hdr";
        //filename<<"image3.hdr";
        cout<<"Generated image "<<filename<<endl;
        image.save(filename.c_str());
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
