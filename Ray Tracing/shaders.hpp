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

struct Material {
    function<Color(float, float)> texture;
    float n, Ks, ior;
    bool transparent;
};


float smoothstep(float);
Color colorRamp(float, Color, Color);


class Checkerboard {
private:
    int scaleX, scaleY;
    
public:
    Checkerboard(int, int);
    
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
