#pragma once


const int N=30;

class ViltrumSamplerPbrt {                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSamplerPbrt(const std::array<double,N>& x, pbrt::Sampler sampler, int spp) : spp_(spp), sampler(sampler), i(0), v(x){}

    static constexpr const char *Name() { return "ViltrumSamplerPbrt"; }


    static ViltrumSamplerPbrt *Create(const ParameterDictionary &parameters,
                                 Point2i fullResolution, const FileLoc *loc,
                                 Allocator alloc){
        return nullptr;
    }

    int SamplesPerPixel() const { return spp_; } //Ojo, returning 0

    void StartPixelSample(Point2i p, int sampleIndex, int dim) {
    }

    Float Get1D() { 
        if(i < v.size()) return v[i++]; 
        else return sampler.Get1D();
    }
    
    Point2f Get2D() { return {Get1D(), Get1D()}; }
    
    Point2f GetPixel2D() { return Get2D(); }

    Sampler Clone(Allocator alloc){
      return alloc.new_object<ViltrumSamplerPbrt>(*this);
    }
    std::string ToString() const {
      return "ViltrumSamplerPbrt";
    }

  private:
    int spp_;
    pbrt::Sampler sampler;                    //Sampler por referencia (todo aleatorios) o por copia (mismos siempre)
    std::size_t i;
    std::array<double,N> v;
};