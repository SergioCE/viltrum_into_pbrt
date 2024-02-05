#pragma once

#include <viltrum/viltrum.h>
#include <viltrum/utils/cimg-wrapper.h>
#include <pbrt/shapes.h>
#include <pbrt/materials.h>
#include <sstream>
#include <CImg.h>

void save_image_hdr(string filename, std::vector<std::vector<SpectrumVilt>> sol){
    
    int resolution_x = sol.size();
    int resolution_y = sol[0].size();
    
    cimg_library::CImg<float> image(resolution_x, resolution_y, 1, 3);

    for(int i=0; i<resolution_x; i++){
        for(int j=0; j<resolution_y; j++){
            for(int k=0; k<3; k++){
                image(i,j,0,k) = sol[i][j][k];
            }
        }
    }

    cout<<"Generated image "<<filename<<endl;
    image.save(filename.c_str());
    cout<<"\nDoing"<<endl;
    image.print();
}