#pragma once

#include "viltrum_into_pbrt.h"
#include <pbrt/cameras.h>
#include <pbrt/util/pstd.h>
//#include <pbrt/shapes.h>
#include <pbrt/cpu/primitive.h>
#include <pbrt/util/stats.h>


template<std::size_t N>
Spectrum F(pbrt::Camera camera, ViltrumSamplerPbrt<N> &sampler){
    pbrt::SampledWavelengths lambda = camera.GetFilm().SampleWavelengths(0.5);
    pbrt::Filter filter = camera.GetFilm().GetFilter();
    pbrt::CameraSample cameraSample = GetCameraSample(sampler, pbrt::Point2i(1,1), filter);       //Nota: Ver cómo la cámara genera el rayo

    // Generate camera ray for current sample
    pstd::optional<pbrt::CameraRayDifferential> cameraRay =
        camera.GenerateRayDifferential(cameraSample, lambda);


    //pstd::optional<pbrt::ShapeIntersection> si = shape.Intersect(cameraRay->ray);
    return Spectrum(0,0,0);
}