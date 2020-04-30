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

#include "settings.hpp"

#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

using namespace std;

struct Input {
    float step_size;
    short bounce_count;
    
    bool calculate_lighting;
    
    Input(float);
};

struct Intersection {
    bool hit;
    Vector3 position;
    float distance;
    Shape *object;
    
    Vector3 normal;
    float light;
    bool shadow;
    Color ambient, diffuse, specular, reflection, transmission;
    
    Color shaded();
};

Intersection castRay(Vector3, Vector3, vector<Shape *>, vector<Light>, Input mask);
