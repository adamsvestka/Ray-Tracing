//
//  shaders.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 5/4/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>

#include <png.h>
#include <jpeglib.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define cimg_use_png
#define cimg_use_jpeg
#include <CImg.h>
#pragma clang diagnostic pop

#include "data_types.hpp"

using namespace std;
using namespace cimg_library;

// MARK: - Material
struct Material {
    function<Color(float, float)> texture;
    float n, Ks, ior;
    bool transparent;
};


// MARK: - Functions
float smoothstep(float);
Color colorRamp(float, Color, Color);


// MARK: - Textures
class Image {
private:
    CImg<unsigned char> image;
    
public:
    Image(string);
    
    Color operator()(float, float) const;
};

class Checkerboard {
private:
    int scale;
    
public:
    Checkerboard(int);
    
    float operator()(float, float) const;
};


class Brick {
private:
    int scale;
    float ratio, mortar;
    
public:
    Brick(int, float, float);
    
    float operator()(float, float) const;
};


class Noise {
private:
    vector<vector<pair<float, float>>> points;
    int scale;
    
public:
    Noise(int, int);
    
    float lerp(float, float, float) const;
    float dotGradient(int, int, float, float) const;
    
    float operator()(float, float) const;
};
