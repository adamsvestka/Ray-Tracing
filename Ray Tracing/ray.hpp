//
//  ray.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Timer;
struct RayInput;
struct RayIntersection;

#pragma once

#include <vector>
#include <valarray>

#include "settings.hpp"

#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

using namespace std;

struct Timer {
    static const short c = 4;
    static const array<string, c> names;
    static const array<Color, c> colors;
    
    chrono::steady_clock::time_point start;
    short last;
    array<float, c> times;
    
    Timer();
    
    void operator()();
    void operator+=(const Timer);
};

struct RayInput {
    bool render;
    short bounce_count;
    
    bool lighting, diffuse, reflections, transmission;
    vector<bool> shadows;
};

struct RayIntersection {
    bool hit;
    Vector3 position;
    float distance;
    const Object *object;
    float id;
    
    Vector3 normal;
    float kr;
    vector<bool> shadows;
    valarray<Color> diffuse, specular;
    Color light, ambient, texture, reflection, transmission;
    
    Color shaded();
    
    Timer timer;
};

RayIntersection castRay(Vector3, Vector3, const vector<Object *> &, const vector<Light *> &, RayInput mask);
