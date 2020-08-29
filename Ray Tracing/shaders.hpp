//
//  shaders.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 5/4/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Material;
class Image;
class Checkerboard;
class Brick;
class Noise;

#pragma once

#include <iostream>
#include <array>
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

typedef function<Color(float, float)> Shader;

// MARK: - Material
struct Material {
    Shader texture;
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
    explicit Image(string);
    
    Color operator()(float, float) const;
};


class Checkerboard {
private:
    int scale;
    Color primary, secondary;
    
public:
    Checkerboard(int, Color, Color);
    
    Color operator()(float, float) const;
};


class Brick {
private:
    vector<vector<Color>> colors;
    int scale;
    float ratio, mortar;
    Color primary, secondary, tertiary;
    
public:
    Brick(int, float, float, Color, Color, Color, int);
    
    Color operator()(float, float) const;
};


class Noise {
private:
    vector<vector<pair<float, float>>> points;
    int scale;
    Color primary;
    
public:
    Noise(int, int, Color);
    
    float lerp(float, float, float) const;
    float dotGradient(int, int, float, float) const;
    
    Color operator()(float, float) const;
};
