//
//  light_sources.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Light;

#pragma once

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"

class Light {
private:
    Vector3 position;
    int intensity;
    
public:
    Light(Vector3 position, int strength);
    
    Vector3 getVector(Ray ray);
    int getIntensity();
};
