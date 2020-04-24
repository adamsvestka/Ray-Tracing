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

struct Intersection {
    bool hit;
    Vector3 position;
    float distance;
    Shape *object;

    Color color;
    Vector3 normal;
    float light;
    bool shadow, diffuse, specular, reflection, refraction;

    Color shaded;
};

Intersection castRay(Vector3, Vector3, vector<Shape *>, vector<Light>);
