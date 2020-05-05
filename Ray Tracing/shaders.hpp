//
//  shaders.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 5/4/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <functional>

#include "data_types.hpp"

using namespace std;

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
