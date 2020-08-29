//
//  shapes.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Hit;
class Shape;
class Sphere;
class Cuboid;
class Plane;
class Triangle;
class Object;

#pragma once

#include <array>
#include <functional>

#include "settings.hpp"

#include "data_types.hpp"
#include "shaders.hpp"
#include "ray.hpp"

struct Hit {
    float distance;
    function<Vector3()> getNormal;
    function<Color()> getTexture;
};


class Shape {
protected:
    Vector3 center;
    Matrix3x3 rotation, Irotation;
    
public:
    Material material;
    
    Shape(Vector3, Vector3, Material);
    /***/ Vector3 getCenter() const;
    /***/ Matrix3x3 getRotation() const;
    /***/ Matrix3x3 getInverseRotation() const;
    /***/ Vector3 toObjectSpace(Vector3 point) const;
    /***/ Vector3 toWorldSpace(Vector3 _point) const;
    /***/ virtual Hit intersect(Vector3 origin, Vector3 direction) const = 0;
};


class Sphere : public Shape {
private:
    float radius;
    float radius2;
    
public:
    Sphere(Vector3, float, Vector3, Material);
    Vector3 getNormal(Vector3) const;
    Color getTexture(Vector3) const;
    Hit intersect(Vector3, Vector3) const;
};


class Cuboid : public Shape {
private:
    Vector3 size;
    Vector3 vmin, vmax;
    
public:
    Cuboid(Vector3, float, Vector3, Material);
    Cuboid(Vector3, float, float, float, Vector3, Material);
    Cuboid(Vector3, Vector3, Vector3, Material);
    Vector3 getNormal(Vector3) const;
    Color getTexture(Vector3) const;
    Hit intersect(Vector3, Vector3) const;
};


class Plane : public Shape {
private:
    float size_x, size_y;
    
public:
    Plane(Vector3, float, float, Vector3, Material);
    Vector3 getNormal(Vector3) const;
    Color getTexture(Vector3) const;
    Hit intersect(Vector3, Vector3) const;
};


class Triangle {
private:
    Vector3 v0, v0v1, v0v2, normal;
    float height;
    
public:
    Triangle(array<Vector3, 3>);
    Vector3 getNormal() const;
    Color getTexture(const Material &, Vector3) const;
    float intersect(Vector3, Vector3) const;
};


class Object : public Shape {
private:
    vector<Triangle> triangles;
    Cuboid bounds;
    
public:
    Object(vector<array<Vector3, 3>>, Vector3, float, Vector3, Material);
    Hit intersect(Vector3, Vector3) const;
};
