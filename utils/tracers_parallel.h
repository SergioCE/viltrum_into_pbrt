#pragma once

#include "viltrum_into_pbrt.h"
#include <pbrt/cameras.h>
#include <pbrt/util/pstd.h>
#include <pbrt/shapes.h>
#include <pbrt/cpu/primitive.h>
#include <pbrt/util/stats.h>
#include <pbrt/base/sampler.h>
#include <pbrt/util/sampling.h>
#include <pbrt/cpu/integrators.h>
#include <pbrt/materials.h>
#include "../utils/parsePbrt.h"
#include <unordered_map>
#include <thread>
#include <mutex>


std::mutex mymutex;
int numIds = 0;
std::unordered_map<unsigned long int, int> threadIds;
bool all_inserted = false;

thread_local int myId = -1;

SpectrumVilt F_parall(const pbrt::Camera& camera, Sampler &sampler, Sampler &samplerPbrt, const pbrt::Point2i& photoSize, pbrt::ScratchBuffer &scratchBuffer, pbrt::RayIntegrator* integrator);

class renderPbrt_parallel {                   //Wrapper para la función sphere
    public:
    SpectrumVilt operator()(const auto& seq) const {
        
        ViltrumSamplerPbrt_father* samplerViltrum_ = new ViltrumSamplerPbrt_template<decltype(seq.begin())>(seq, spp_, &samplerP);
        unique_ptr<ViltrumSamplerPbrt> samplerV(new pbrt::ViltrumSamplerPbrt(samplerViltrum_, numDim, chosen_dims));
        
        Sampler sampler = samplerV.get();
        
        //ViltrumSamplerPbrt* a = new pbrt::ViltrumSamplerPbrt(samplerViltrum_);
        //Sampler sampler = a;
        //samplers.push_back(new pbrt::ViltrumSamplerPbrt(samplerViltrum_, samplerP));
        //pbrt::ViltrumSamplerPbrt samplerViltrum(samplerViltrum_, samplerP);
        
        int id;
        unsigned long int thisThreadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
        

        if(!all_inserted){
            {
                std::lock_guard<std::mutex> lock(mymutex);

                auto it = threadIds.find(thisThreadId);
                if(it == threadIds.end()){
                    myId=numIds;
                    threadIds.insert({thisThreadId,numIds});
                    //std::cout<<"Id "<<numIds<<"assigned to "<<thisThreadId<<std::endl;
                    //for (const auto& pair : threadIds) {
                    //    std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
                    //}
                    numIds++;
                    if(numIds == numThreads) all_inserted=true;
                }
            }
        }
        id=myId;
        //std::cout<<id<<std::endl;
        s_buffers[id].Reset();
        
        return F_parall(camera_, sampler, *samplerV->GetSampler(), photoSize, s_buffers[id], integrator_);
    };
    
    renderPbrt_parallel(pbrt::RayIntegrator* integrator, pbrt::Camera camera, pbrt::Sampler sampler, int spp, pbrt::Point2i photoSize
            ,std::vector<pbrt::ScratchBuffer>& s_buffers_, int numDim_ = 0, const std::vector<std::tuple<int,int>>& chosen_dims_ = {}): 
            numDim(numDim_), chosen_dims(chosen_dims_), s_buffers(s_buffers_), spp_(spp), integrator_(integrator), camera_(camera), samplerP(sampler), photoSize(photoSize){
        //alloc = new pbrt::Allocator();
        
    }

    private:
    int spp_;
    pbrt::RayIntegrator* integrator_;
    pbrt::ScratchBuffer *scratchBuffer;
    pbrt::Point2i photoSize;
    pbrt::Camera camera_;
    pbrt::Sampler &samplerP;

    std::vector<std::tuple<int,int>> chosen_dims;
    int numDim;

    std::vector<pbrt::ScratchBuffer>& s_buffers;
    unsigned int numThreads = std::thread::hardware_concurrency(); 
};


// Sampler Inline Functions
pbrt::CameraSample GetCameraSample_Viltrum(Sampler &sampler, Point2f pPixelfi,
                                                 Filter filter)
{
    pbrt::CameraSample cs;
    //Initialize _CameraSample_ member variables
    cs.pFilm = pPixelfi;
    cs.time = sampler.Get1D();
    cs.pLens = sampler.Get2D();

    return cs;
}



SpectrumVilt F_parall(const pbrt::Camera& camera, Sampler &sampler, Sampler &samplerPbrt, const pbrt::Point2i& photoSize, pbrt::ScratchBuffer &scratchBuffer, pbrt::RayIntegrator* integrator){

    // 0 and 1
    Point2f imgSample = sampler.Get2D();
    
    pbrt::SampledWavelengths lambda = camera.GetFilm().SampleWavelengths(sampler.Get1D()); //samplerViltrum.Get1DSp());
    pbrt::Filter filter = camera.GetFilm().GetFilter();                                 //Mejor get2D

    pbrt::CameraSample cameraSample = GetCameraSample_Viltrum(sampler, pbrt::Point2f(photoSize[0]*imgSample[0],photoSize[1]*imgSample[1]), filter);       //Nota: Ver cómo la cámara genera el rayo

    //std::cout<<"from 0 to 5"<<std::endl;
    pstd::optional<pbrt::CameraRayDifferential> cr = camera.GenerateRayDifferential(cameraSample, lambda);
    
    SampledSpectrum L(0.);
    VisibleSurface visibleSurface;
    if(cr){

        //TODO: It's here!
        DCHECK_GT(Length(cameraRay->ray.d), .999f);
        DCHECK_LT(Length(cameraRay->ray.d), 1.001f);
        Float rayDiffScale =
                std::max<Float>(.125f, 1 / std::sqrt((Float)sampler.SamplesPerPixel()));
        
        cr->ray.ScaleDifferentials(rayDiffScale);
        
        bool initializeVisibleSurface = camera.GetFilm().UsesVisibleSurface();

        L = cr->weight * integrator->Li(cr->ray, lambda, sampler, scratchBuffer,initializeVisibleSurface ? &visibleSurface : nullptr);                                //Necesito un integrador para usar su LI, qué hago?

        if (L.HasNaNs()) {
            std::cout<<"Invalid value"<<std::endl;
            L = SampledSpectrum(0.f);
        } else if (IsInf(L.y(lambda))) {
            std::cout<<"Invalid value"<<std::endl;
            L = SampledSpectrum(0.f);
        }
    }
    else{
        std::cout<<"No ray generated"<<std::endl;
    }


    RGB rgb = camera.GetFilm().GetPixelSensor()->ToSensorRGB(L, lambda);
    Float m = std::max({rgb.r, rgb.g, rgb.b});
    float maxComponentValue = 25;
    if (m > maxComponentValue)
        rgb *= maxComponentValue / m;
    
    //rgb = rgb*cameraSample.filterWeight;

    pbrt::SquareMatrix<3> outputRGBFromSensorRGB = pbrt::RGBColorSpace::sRGB->RGBFromXYZ * camera.GetFilm().GetPixelSensor()->XYZFromSensorRGB;
    rgb = outputRGBFromSensorRGB * rgb;
    cr.reset();
    return SpectrumVilt(rgb[0], rgb[1], rgb[2]);
    
}