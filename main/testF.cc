
#include "../utils/tracers.h"
#include "../utils/parsePbrt.h"
#include <viltrum/viltrum.h>
#include "viltrum/utils/cimg-wrapper.h"
#include "viltrum/quadrature/monte-carlo.h"
#include <pbrt/shapes.h>
#include <pbrt/materials.h>

int main(int argc, char *argv[]){

    std::vector<std::string> filenames = getScene(argc,argv);
    
    BasicScene scenePbrt;
    BasicSceneBuilder builder(&scenePbrt);
    ParseFiles(&builder, filenames);
    
    int w = 400;
    int h = 400;
    int spp = 100;

    viltrum::CImgWrapper<double> image(w,h);

    Primitive shapes = getPrimitives(scenePbrt);

    pbrt::ScratchBuffer scratchBuffer(65536);

    pbrt::IndependentSampler sampler(1024);


    NamedTextures textures = scenePbrt.CreateTextures();
    std::map<int, pstd::vector<Light> *> shapeIndexToAreaLights;
    std::vector<Light> lights =
        scenePbrt.CreateLights(textures, &shapeIndexToAreaLights);
    std::map<std::string, pbrt::Material> namedMaterials;
    std::vector<pbrt::Material> materials;
    scenePbrt.CreateMaterials(textures, &namedMaterials, &materials);
    
    std::unique_ptr<pbrt::Integrator> integratorP(
        scenePbrt.CreateIntegrator(scenePbrt.GetCamera(), scenePbrt.GetSampler(), shapes, lights));
    
    pbrt::RayIntegrator* rayInt = dynamic_cast<pbrt::RayIntegrator*>(integratorP.get());

    auto integrator_bins = viltrum::integrator_bins_monte_carlo_uniform(w*h*spp); //Probar con más samples
    auto range = viltrum::range_all<30>(0.0,1.0);
    

    //cout<<"a"<<endl;
    integrator_bins.integrate(image,image.resolution(),renderPbrt(rayInt, scenePbrt.GetCamera(), scenePbrt.GetSampler(), spp, Point2i(w,h), scratchBuffer), range);
    std::stringstream filename;
	filename<<"image.hdr";
	image.save(filename.str());
    cout<<"\nDoing"<<endl;
    image.print();
    return 0;
}


