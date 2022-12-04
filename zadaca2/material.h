#pragma once
#include "geometry.h"

struct Material
{  
    Vec3f diffuse_color;
    
    float diffuse_coef = 1;
    float specular_coef = 0;
    float phong_exp = 1;
	
	float specular_exponent;
	float opacity;
    
    Material(const Vec3f &color, float op) : diffuse_color(color), specular_exponent(specular_coef), opacity(op){}
    Material() : diffuse_color(Vec3f(0, 0, 0)), specular_exponent(specular_coef), opacity(1.f) {}
};


