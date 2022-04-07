#pragma once

#include "viltrum_into_pbrt.h"
#include <pbrt/cameras.h>
#include <pbrt/util/pstd.h>
#include <pbrt/shapes.h>
#include <pbrt/cpu/primitive.h>
#include <pbrt/util/stats.h>
#include <pbrt/util/sampling.h>



template<std::size_t N>
class renderPbrt {                   //Wrapper para la función sphere
    public:
    Spectrum operator()(const std::array<double,N>& x) const {
        ViltrumSamplerPbrt<N> sampler(x, samplerP);

        return F(camera_, shapes_, sampler, photoSize);
    }
    renderPbrt(pbrt::IndependentSampler &sampler, pbrt::Camera camera, 
        pbrt::Primitive shapes, pbrt::Point2i photoSize) : shapes_(shapes), photoSize(photoSize), camera_(camera) , samplerP(sampler){}

    private:
    pbrt::Point2i photoSize;
    pbrt::Camera camera_;
    pbrt::Primitive shapes_;
    pbrt::IndependentSampler &samplerP;
};

template<std::size_t N>
Spectrum F(pbrt::Camera camera, pbrt::Primitive shapes, ViltrumSamplerPbrt<N> &sampler, pbrt::Point2i photoSize){
    pbrt::SampledWavelengths lambda = camera.GetFilm().SampleWavelengths(0.5);
    pbrt::Filter filter = camera.GetFilm().GetFilter();
    pbrt::CameraSample cameraSample = GetCameraSample(sampler, pbrt::Point2i(photoSize[0]*sampler.Get1D(),photoSize[1]*sampler.Get1D()), filter);       //Nota: Ver cómo la cámara genera el rayo

    // Generate camera ray for current sample
    pstd::optional<pbrt::CameraRay> cr = camera.GenerateRay(cameraSample, lambda);

    pbrt::Ray ray = cr->ray;
    //cout<<ray.ToString()<<endl;
    pstd::optional<pbrt::ShapeIntersection> intersect = shapes.Intersect(ray,100000000000000);
    if(!intersect) return Spectrum(1,1,1);
    else{
        pbrt::Vector3f direction = SampleCosineHemisphere(sampler.Get2D());
        pbrt::Ray ray = intersect->intr.SpawnRayTo(pbrt::Point3f(direction));
        pstd::optional<pbrt::ShapeIntersection> intersect2 = shapes.Intersect(ray,100000000000);
        if(!intersect2) return Spectrum(1,0,0);//return intersect->intr.material.color_;
        else return Spectrum(0,0,0);
    }

}