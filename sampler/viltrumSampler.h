#pragma once

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

    virtual Sampler* GetSampler(){}

    virtual Float Get1D() {}

    virtual Float Get1DSp(){}
    
    virtual Point2f Get2D() {}
    
    virtual Point2f GetPixel2D() {}

    virtual Sampler Clone(Allocator alloc){}

    virtual std::string ToString() const {}

    virtual ~ViltrumSamplerPbrt_father(){}

    

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

    Sampler* GetSampler() override{ return NULL;}
    
    Point2f Get2D() override{ 
      //float x = *(*it); ++(*it);
      //float y = *(*it); ++(*it);
      return {Get1D(), Get1D()}; 
    }
    
    Point2f GetPixel2D() override{ return {Get1D(), Get1D()};  }

    std::string ToString() const override{
      return "ViltrumSamplerPbrt";
    }


    ~ViltrumSamplerPbrt_template() override{
      delete(it);
    }

  private:
    int spp_;
    Iterator* it;
};


class ViltrumSamplerPbrt {                      //NOTA: Fijarse en este
  public:
    // IndependentSampler Public Methods
    ViltrumSamplerPbrt(ViltrumSamplerPbrt_father* sampler_, int cvDims_, const std::vector<std::tuple<int,int>>& chosen_dims_)
    : chosen_dims(chosen_dims_), totalPairs(chosen_dims_.size()), pair_idx(0), curr_pair(0), sampler(sampler_){
      
      for(int i=0; i<cvDims_; i++){
          float x = sampler->Get1D();
        cv_samples.push_back(x);
      }
    }

    static constexpr const char *Name() { return "ViltrumSamplerPbrt"; }


    static ViltrumSamplerPbrt *Create(const ParameterDictionary &parameters,
                                 Point2i fullResolution, const FileLoc *loc,
                                 Allocator alloc){
        return nullptr;
    }

    int SamplesPerPixel() const { return sampler->SamplesPerPixel(); } //Ojo, returning 0

    void StartPixelSample(Point2i p, int sampleIndex, int dim) { return sampler->StartPixelSample(p, sampleIndex, dim);}

    Float Get1D() { 
      return sampler->Get1D();
    }

    Sampler* GetSampler(){ return NULL;}

    Float Get1DSp(){ return Get1D();}
    
    Point2f Get2D() { 
      return {Get1D(), Get1D()};
    }
    
    Point2f GetPixel2D() { 
      //std::cout<<"\t\tCurr pair: "<<curr_pair<<std::endl;
      if(pair_idx < totalPairs){
        if(curr_pair == std::get<0>(chosen_dims[pair_idx])){
          
          pair_idx++;
          float x1 = cv_samples[2*std::get<1>(chosen_dims[pair_idx-1])];
          float x2 = cv_samples[2*std::get<1>(chosen_dims[pair_idx-1]) + 1];
          curr_pair++;
          //std::cout<<"\t -> "<< x1<<" - "<< x2<<std::endl;
          return {x1, x2};
        }
      }
      curr_pair++;
      return Get2D();
    }

    Sampler Clone(Allocator alloc){ return alloc.new_object<ViltrumSamplerPbrt>(*this);}

    std::string ToString() const {return sampler->ToString();}

    ~ViltrumSamplerPbrt(){
      delete(sampler);
      cv_samples.clear();
    }
  
  private: 
    ViltrumSamplerPbrt_father placeholder;
    ViltrumSamplerPbrt_father* sampler;
    std::vector<float> cv_samples;
    std::vector<std::tuple<int,int>> chosen_dims;
    int curr_pair;
    int pair_idx;
    int totalPairs;
};
