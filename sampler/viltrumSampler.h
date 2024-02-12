#pragma once
const int N=4;

class ViltrumSamplerPbrt_father {                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSamplerPbrt_father() {}

    static constexpr const char *Name() { return "ViltrumSamplerPbrt"; }


    static ViltrumSamplerPbrt *Create(const ParameterDictionary &parameters,
                                 Point2i fullResolution, const FileLoc *loc,
                                 Allocator alloc){
        return nullptr;
    }

    virtual int SamplesPerPixel() const {}

    void StartPixelSample(Point2i p, int sampleIndex, int dim) {}

    virtual Float Get1D() {}

    virtual Float Get1DSp(){}
    
    virtual Point2f Get2D() {}
    
    virtual Point2f GetPixel2D() {}

    virtual Sampler Clone(Allocator alloc){}

    virtual std::string ToString() const {}

};

template<typename Iterator>
class ViltrumSamplerPbrt_template : public ViltrumSamplerPbrt_father{                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSamplerPbrt_template(const auto& seq_, int spp) : spp_(spp){
      it = new Iterator(seq_.begin());
    }

    int SamplesPerPixel() const override { return spp_; }

    Float Get1D() override{ 
      float x = *(*it); ++(*it);
      return x;
    }

    Float Get1DSp() override{
      return Get1D();
    }
    
    Point2f Get2D() override{ 
      return {Get1D(), Get1D()}; 
    }
    
    Point2f GetPixel2D() override{ return {Get1D(), Get1D()};  }

    std::string ToString() const override{
      return "ViltrumSamplerPbrt";
    }

  private:
    int spp_;
    Iterator* it;
};


class ViltrumSamplerPbrt {                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSamplerPbrt(ViltrumSamplerPbrt_father* sampler_, pbrt::Sampler samplerPbrt_): sampler(sampler_), samplerPbrt(samplerPbrt_){}

    static constexpr const char *Name() { return "ViltrumSamplerPbrt"; }


    static ViltrumSamplerPbrt *Create(const ParameterDictionary &parameters,
                                 Point2i fullResolution, const FileLoc *loc,
                                 Allocator alloc){
        return nullptr;
    }

    int SamplesPerPixel() const { return sampler->SamplesPerPixel(); } //Ojo, returning 0

    void StartPixelSample(Point2i p, int sampleIndex, int dim) { return sampler->StartPixelSample(p, sampleIndex, dim);}

    Float Get1D() { return sampler->Get1D();}

    Sampler* GetSampler(){ return &samplerPbrt;}

    Float Get1DSp(){ return sampler->Get1DSp();}
    
    Point2f Get2D() { return sampler->Get2D();}
    
    Point2f GetPixel2D() { return sampler->GetPixel2D();}

    Sampler Clone(Allocator alloc){ return alloc.new_object<ViltrumSamplerPbrt>(*this);}

    std::string ToString() const { return sampler->ToString();}
  
  private: 
    ViltrumSamplerPbrt_father placeholder;
    ViltrumSamplerPbrt_father* sampler;
    pbrt::Sampler samplerPbrt;

};
