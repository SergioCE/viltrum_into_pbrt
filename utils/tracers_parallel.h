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

SpectrumVilt F_parall(const pbrt::Camera& camera, pbrt::ViltrumSamplerPbrt &samplerViltrum, const pbrt::Point2i& photoSize, pbrt::ScratchBuffer &scratchBuffer, pbrt::RayIntegrator* integrator);

class renderPbrt_parallel {                   //Wrapper para la función sphere
    public:
    SpectrumVilt operator()(const auto& seq) const {
        ViltrumSamplerPbrt_father* samplerViltrum_ = new ViltrumSamplerPbrt_template<decltype(seq.begin())>(seq, spp_);
        pbrt::ViltrumSamplerPbrt samplerViltrum(samplerViltrum_, samplerP);
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
        return F_parall(camera_, samplerViltrum, photoSize, s_buffers[id], integrator_);
    };
    
    renderPbrt_parallel(pbrt::RayIntegrator* integrator, pbrt::Camera camera, pbrt::Sampler sampler, int spp, pbrt::Point2i photoSize
            ,std::vector<pbrt::ScratchBuffer>& s_buffers_): s_buffers(s_buffers_), spp_(spp), integrator_(integrator), camera_(camera), samplerP(sampler), photoSize(photoSize){
        
    }

    private:
    int spp_;
    pbrt::Allocator alloc;
    pbrt::RayIntegrator* integrator_;
    pbrt::ScratchBuffer *scratchBuffer;
    pbrt::Point2i photoSize;
    pbrt::Camera camera_;
    pbrt::Sampler &samplerP;

    std::vector<pbrt::ScratchBuffer>& s_buffers;
    unsigned int numThreads = std::thread::hardware_concurrency(); 
};


SpectrumVilt F_parall(const pbrt::Camera& camera, pbrt::ViltrumSamplerPbrt &samplerViltrum, const pbrt::Point2i& photoSize, pbrt::ScratchBuffer &scratchBuffer, pbrt::RayIntegrator* integrator){
    pbrt::Allocator alloc;
    pbrt::Sampler sampler = samplerViltrum.Clone(alloc);

    pbrt::SampledWavelengths lambda = camera.GetFilm().SampleWavelengths(0.5); //samplerViltrum.Get1DSp());
    pbrt::Filter filter = camera.GetFilm().GetFilter();                                 //Mejor get2D
    //cout<<"PIXELES 2 SAMPLES"<<endl;
    Point2f imgSample = sampler.Get2D();
    pbrt::CameraSample cameraSample = GetCameraSample(*samplerViltrum.GetSampler(), pbrt::Point2i(photoSize[0]*imgSample[0],photoSize[1]*imgSample[1]), filter);       //Nota: Ver cómo la cámara genera el rayo
    //pbrt::CameraSample cameraSample = GetCameraSample(sampler, pbrt::Point2i(photoSize[0]*imgSample[0],photoSize[1]*imgSample[1]), filter);       //Nota: Ver cómo la cámara genera el rayo
    // Generate camera ray for current sample
    pstd::optional<pbrt::CameraRayDifferential> cr = camera.GenerateRayDifferential(cameraSample, lambda);
    
    SampledSpectrum L(0.);
    VisibleSurface visibleSurface;
    if(cr){

        //TODO: It's here!
        DCHECK_GT(Length(cameraRay->ray.d), .999f);
        DCHECK_LT(Length(cameraRay->ray.d), 1.001f);
        Float rayDiffScale =
                std::max<Float>(.125f, 1 / std::sqrt((Float)sampler.SamplesPerPixel()));
        if (!Options->disablePixelJitter)
            cr->ray.ScaleDifferentials(rayDiffScale);
        
        bool initializeVisibleSurface = camera.GetFilm().UsesVisibleSurface();

        L = cr->weight * integrator->Li(cr->ray, lambda, sampler, scratchBuffer,initializeVisibleSurface ? &visibleSurface : nullptr);                                //Necesito un integrador para usar su LI, qué hago?

        if (L.HasNaNs()) {
            L = SampledSpectrum(0.f);
        } else if (IsInf(L.y(lambda))) {
            L = SampledSpectrum(0.f);
        }
    }

    RGB rgb = camera.GetFilm().GetPixelSensor()->ToSensorRGB(L, lambda);
    Float maxComponentValue = Infinity;
        // Optionally clamp sensor RGB value
        Float m = std::max({rgb.r, rgb.g, rgb.b});
        if (m > maxComponentValue)
            rgb *= maxComponentValue / m;

        DCHECK(InsideExclusive(pFilm, pixelBounds));
        // Update RGB fields in Pixel structure.

        // Spectral processing starts here.
        // Optionally clamp spectral value. (TODO: for spectral should we
        // just clamp channels individually?)
        Float lm = L.MaxComponentValue();
        if (lm > maxComponentValue)
            L *= maxComponentValue / lm;

        // The CIE_Y_integral factor effectively cancels out the effect of
        // the conversion of light sources to use photometric units for
        // specification.  We then do *not* divide by the PDF in |lambda|
        // but take advantage of the fact that we know that it is uniform
        // in SampleWavelengths(), the fact that the buckets all have the
        // same extend, and can then just average radiance in buckets
        // below.
        L *= cameraSample.filterWeight * CIE_Y_integral;
    //cout<<L<<endl;
    //pbrt::RGB color = L.ToRGB(lambda, *pbrt::RGBColorSpace::sRGB);
    //cout<<color[0]<<","<< color[1]<<"."<< color[2]<<endl;
    return SpectrumVilt(rgb[0] * cameraSample.filterWeight, rgb[1] * cameraSample.filterWeight, rgb[2] * cameraSample.filterWeight);
    //RGB ToRGB(const SampledWavelengths &lambda, const RGBColorSpace &cs) const;
}