//
//  ray.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Ray;

#pragma once

#include <vector>
#include <valarray>

#include "settings.hpp"

#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

using namespace std;

struct Input {
    float step_size;
    short bounce_count;
    
    bool lighting, reflections, transmission;
    vector<bool> shadows;
};

struct Intersection {
    bool hit;
    Vector3 position;
    float distance;
    const Shape *object;
    
    Vector3 normal;
    float light, kr;
    vector<bool> shadows;
    valarray<Color> diffuse, specular;
    Color ambient, texture, reflection, transmission;
    
    Color shaded();
};

Intersection castRay(Vector3, Vector3, const vector<Shape *> &, const vector<Light *> &, Input mask);
