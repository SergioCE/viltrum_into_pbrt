#include <math.h>
#include <iostream>



float f(float x, float a, float b, float c, float k){
    return (a*pow(x,3)/3 + b*pow(x,2)/2 + c*x - k) / k;
}


float f_inv(float x, float a, float b, float c, float k){ 
    float term_one = -pow(sqrt(pow((-648*pow(a,2)*k*x - 648*pow(a,2)*k - 324*a*b*c + 54*pow(b,3)),2) + 4*pow((36*a*c - 9*pow(b,2)),3)) - 648*pow(a,2)*k*x - 648*pow(a,2)*k - 324*a*b*c + 54*pow(b,3),1.0/3.0);
    float term_two = 6*pow(2,1.0/3.0)*a;
    float term_three = (36*a*c - 9*pow(b,2)) / (3*pow(2,2.0/3.0)* a * pow(sqrt(pow(-648*pow(a,2)*k*x - 648*pow(a,2)*k - 324*a*b*c + 54*pow(b,3),2) + 4*pow(36*a*c - 9*pow(b,2),3))  
                                                        -648*pow(a,2)*k*x - 648*pow(a,2)*k - 324*a*b*c + 54*pow(b,3),1.0/3.0)) - b/(2*a);

    return term_one / term_two + term_three;
}


float f(float x, float a, float b, float c){
    return a*pow(x,3)/3 + b*pow(x,2)/2 + c*x;
}


float f_inv(float x, float a, float b, float c){ 
    float term_one = -pow(sqrt(pow((-648*pow(a,2)*x - 324*a*b*c + 54*pow(b,3)),2) + 4*pow((36*a*c - 9*pow(b,2)),3)) - 648*pow(a,2)*x - 324*a*b*c + 54*pow(b,3),1.0/3.0);
    float term_two = 6*pow(2,1.0/3.0)*a;
    float term_three = (36*a*c - 9*pow(b,2)) / (3*pow(2,2.0/3.0)* a * pow(sqrt(pow(-648*pow(a,2)*x - 324*a*b*c + 54*pow(b,3),2) + 4*pow(36*a*c - 9*pow(b,2),3))  
                                                        -648*pow(a,2)*x - 324*a*b*c + 54*pow(b,3),1.0/3.0)) - b/(2*a);

    return term_one / term_two + term_three;
}


int main(int argc, char *argv[]){

    float x = 0.5;
    float a = 20;
    float b = 3.3;
    float c = 0.576;
    float k = 0.87;

    std::cout<<f_inv(x,a,b,c,k)<<std::endl;
    std::cout<<f(f_inv(x,a,b,c,k),a,b,c,k)<<std::endl;

    std::cout<<f_inv(x,a,b,c)<<std::endl;
    std::cout<<f(f_inv(x,a,b,c),a,b,c)<<std::endl;
    return 0;
}