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

#include "data_types.hpp"

using namespace std;

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
    Buffer image;
    
public:
    explicit Image(Buffer &);
    
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
    Buffer colors;
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
