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

#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

#define RENDER_DISTANCE 20
#define STEP_SIZE 0.02

class Ray {
private:
    Vector3 origin, direction, position;
    Shape *object;
    float distance;
    
public:
    Ray(Vector3, Vector3);
    
    Vector3 getIntersectionPosition();
    Shape *getIntersectingObject();
    
    float traceObject(std::vector<Shape*>);
    float traceLight(std::vector<Light>, std::vector<Shape*>);
};
