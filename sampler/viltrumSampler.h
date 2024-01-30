#pragma once
const int N=4;

class ViltrumSamplerPbrt {                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSamplerPbrt(const std::array<double,N>& x, pbrt::Sampler sampler, int spp, bool _2DOnly_ = false, int repeatedDim_ = -1) : _2DOnly(_2DOnly_), repeatedDim(repeatedDim_), spp_(spp), sampler(sampler), j(0), i(0), v(x){}

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
      //std::cout<<i<<std::endl;
      if(_2DOnly){
        return sampler.Get1D();
      }
      else{
        if(i < v.size()){
          return static_cast<float>(v[i++]);
        }  
        else {
          i++;
          return sampler.Get1D();
        }
      }
    }

    Sampler* GetSampler(){
      return &sampler;
    }

    Float Get1DSp(){
      return sampler.Get1D();
    }
    
    Point2f Get2D() { 
      if(_2DOnly){
        if(i+1 < v.size()){
            if(j == repeatedDim){
              j=j+2;
              return {static_cast<float>(v[i]),static_cast<float>(v[i+1])};
            }
            else{
              j=j+2;
              return {static_cast<float>(v[i++]),static_cast<float>(v[i++])};
            }
        }  
        else {
          return {sampler.Get1D(),sampler.Get1D()};
        }
      }
      else{
        return {Get1D(), Get1D()}; 
      }
    }
    
    Point2f GetPixel2D() { return {Get1D(), Get1D()};  }

    Sampler Clone(Allocator alloc){
      return alloc.new_object<ViltrumSamplerPbrt>(*this);
    }
    std::string ToString() const {
      return "ViltrumSamplerPbrt";
    }

  private:
    bool _2DOnly;
    int repeatedDim;
    int spp_;
    pbrt::Sampler sampler;                    //Sampler por referencia (todo aleatorios) o por copia (mismos siempre)
    std::size_t i;
    std::size_t j;
    std::array<double,N> v;
};