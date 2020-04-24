//
//  light_sources.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "light_sources.hpp"

Light::Light(Vector3 position, Color color, int strength) {
    this->position = position;
    this->color = color;
    intensity = strength;
}

Vector3 Light::getVector(Vector3 point) {
    return position - point;
}

int Light::getIntensity() { return intensity; };
