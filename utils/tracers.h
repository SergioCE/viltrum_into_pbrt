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
#include "../utils/tracers.h"
#include "../utils/parsePbrt.h"



SpectrumVilt F(pbrt::Camera camera, pbrt::Sampler &sampler, pbrt::Point2i photoSize, pbrt::ScratchBuffer &scratchBuffer, pbrt::RayIntegrator* integrator);

class renderPbrt {                   //Wrapper para la función sphere
    public:
    SpectrumVilt operator()(const std::array<double,N>& x) const {
        pbrt::ViltrumSamplerPbrt samplerViltrum(x, samplerP, spp_);
        pbrt::Sampler sampler = samplerViltrum.Clone(alloc);

        return F(camera_, sampler, photoSize, scratchBuffer, integrator_);
    };
   /* renderPbrt(pbrt::IndependentSampler &sampler, pbrt::Camera camera, 
        pbrt::Primitive shapes, pbrt::Point2i photoSize, pbrt::ScratchBuffer &scratchBuffer, 
        pbrt::Integrator integrator) : integrator(integrator) scratchBuffer(scratchBuffer), shapes_(shapes), photoSize(photoSize), camera_(camera) , samplerP(sampler){}
*/
    renderPbrt(pbrt::RayIntegrator* integrator, pbrt::Camera camera, pbrt::Sampler sampler, int spp, pbrt::Point2i photoSize, pbrt::ScratchBuffer &scratchBuffer
            ): spp_(spp), integrator_(integrator), camera_(camera), samplerP(sampler), scratchBuffer(scratchBuffer), photoSize(photoSize){
                
    }

    private:
    int spp_;
    pbrt::Allocator alloc;
    pbrt::RayIntegrator* integrator_;
    pbrt::ScratchBuffer &scratchBuffer;
    pbrt::Point2i photoSize;
    pbrt::Camera camera_;
    pbrt::Sampler &samplerP;
};


SpectrumVilt F(pbrt::Camera camera, pbrt::Sampler &sampler, pbrt::Point2i photoSize, pbrt::ScratchBuffer &scratchBuffer, pbrt::RayIntegrator* integrator){
    
    pbrt::SampledWavelengths lambda = camera.GetFilm().SampleWavelengths(0.5);
    pbrt::Filter filter = camera.GetFilm().GetFilter();                                 //Mejor get2D
    Point2f imgSample = sampler.Get2D();
    pbrt::CameraSample cameraSample = GetCameraSample(sampler, pbrt::Point2i(photoSize[0]*imgSample[0],photoSize[1]*imgSample[1]), filter);       //Nota: Ver cómo la cámara genera el rayo

    // Generate camera ray for current sample
    pstd::optional<pbrt::CameraRayDifferential> cr = camera.GenerateRayDifferential(cameraSample, lambda);

    Float rayDiffScale =
            std::max<Float>(.125f, 1 / std::sqrt((Float)sampler.SamplesPerPixel()));
    
    cr->ray.ScaleDifferentials(rayDiffScale);
    
    pbrt::SampledSpectrum L(0.);
    pbrt::VisibleSurface visibleSurface;

    bool initializeVisibleSurface = camera.GetFilm().UsesVisibleSurface();
    L = cr->weight * integrator->Li(cr->ray, lambda, sampler, scratchBuffer,initializeVisibleSurface ? &visibleSurface : nullptr);                                //Necesito un integrador para usar su LI, qué hago?

    pbrt::RGB color = camera.GetFilm().GetPixelSensor()->ToSensorRGB(L, lambda);     
    
    //cout<<L<<endl;
    //pbrt::RGB color = L.ToRGB(lambda, *pbrt::RGBColorSpace::sRGB);
    //cout<<color[0]<<","<< color[1]<<"."<< color[2]<<endl;
    return SpectrumVilt(color[0] * cameraSample.filterWeight, color[1] * cameraSample.filterWeight, color[2] * cameraSample.filterWeight);
    //RGB ToRGB(const SampledWavelengths &lambda, const RGBColorSpace &cs) const;
}