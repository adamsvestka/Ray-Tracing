//
//  shapes.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Shape;
class Sphere;

#pragma once

#include "data_types.hpp"

class Shape {
protected:
    Vector3 center;
    
public:
    Vector3 getCenter();
    virtual bool intersects(Vector3) = 0;
    virtual Vector3 getNormal(Vector3) = 0;
    virtual Vector3 getSurface(Vector3) = 0;
};

class Sphere : public Shape {
private:
    float radius;
    
public:
    Sphere(Vector3, float);
    bool intersects(Vector3);
    Vector3 getNormal(Vector3);
    Vector3 getSurface(Vector3);
};
