//
//  shapes.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Shape;
class Sphere;
class Cube;

#pragma once

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"

class Shape {
protected:
    Vector3 center;
    
public:
    Material material;
    
    Vector3 getCenter();
    virtual bool intersects(Vector3) = 0;
    virtual Vector3 getNormal(Vector3) = 0;
    virtual Vector3 getSurface(Vector3) = 0;
};


class Sphere : public Shape {
private:
    float radius;
    
public:
    Sphere(Vector3, float, Material);
    bool intersects(Vector3);
    Vector3 getNormal(Vector3);
    Vector3 getSurface(Vector3);
};


class Cube : public Shape {
private:
    float radius;
    
public:
    Cube(Vector3, float, Material);
    bool intersects(Vector3);
    Vector3 getNormal(Vector3);
    Vector3 getSurface(Vector3);
};
