//
//  light_sources.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "light_sources.hpp"

Light::Light(Vector3 position, int strength) {
    this->position = position;
    intensity = strength;
}

Vector3 Light::getVector(Ray ray) {
    return position - ray.getIntersectionPosition();
}

int Light::getIntensity() { return intensity; };
