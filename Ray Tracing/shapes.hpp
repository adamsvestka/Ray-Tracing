//
//  shapes.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Shape;
class Sphere;
class Cuboid;

#pragma once

#include "settings.hpp"

#include "data_types.hpp"
#include "shaders.hpp"
#include "ray.hpp"

class Shape {
protected:
    Vector3 center;
    Matrix3x3 rotation, Irotation;
    
public:
    Material material;
    
    /***/ Vector3 getCenter() const;
    /***/ Matrix3x3 getRotation() const;
    /***/ Matrix3x3 getInverseRotation() const;
    /***/ Vector3 toObjectSpace(Vector3 point) const;
    /***/ Vector3 toWorldSpace(Vector3 _point) const;
    /***/ virtual bool intersects(Vector3 _point, Vector3 _direction) const = 0;
    /***/ virtual Vector3 getNormal(Vector3 _point, Vector3 _direction) const = 0;
    /***/ virtual Vector3 getSurface(Vector3 _point, Vector3 _direction) const = 0;
    /***/ virtual Color getTexture(Vector3 _point) const = 0;
};


class Sphere : public Shape {
private:
    float radius;
    
public:
    Sphere(Vector3, float, Vector3, Material);
    virtual bool intersects(Vector3, Vector3) const;
    virtual Vector3 getNormal(Vector3, Vector3) const;
    virtual Vector3 getSurface(Vector3, Vector3) const;
    virtual Color getTexture(Vector3) const;
};


class Cuboid : public Shape {
private:
    float size_x, size_y, size_z;
    
public:
    Cuboid(Vector3, float, float, float, Vector3, Material);
    virtual bool intersects(Vector3, Vector3) const;
    virtual Vector3 getNormal(Vector3, Vector3) const;
    virtual Vector3 getSurface(Vector3, Vector3) const;
    virtual Color getTexture(Vector3) const;
};


class Cylinder : public Shape {
private:
    float radius, height;
    
public:
    Cylinder(Vector3, float, float, Vector3, Material);
    virtual bool intersects(Vector3, Vector3) const;
    virtual Vector3 getNormal(Vector3, Vector3) const;
    virtual Vector3 getSurface(Vector3, Vector3) const;
    virtual Color getTexture(Vector3) const;
};


class Plane : public Shape {
private:
    float size_x, size_y;
    
public:
    Plane(Vector3, float, float, Vector3, Material);
    virtual bool intersects(Vector3, Vector3) const;
    virtual Vector3 getNormal(Vector3, Vector3) const;
    virtual Vector3 getSurface(Vector3, Vector3) const;
    virtual Color getTexture(Vector3) const;
};
