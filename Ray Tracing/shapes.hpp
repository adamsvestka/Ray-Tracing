//
//  shapes.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct ObjectHit;
struct ObjectInfo;

class Object;
class Sphere;
class Cuboid;
class Plane;
class Triangle;
class Mesh;

#pragma once

#include <array>
#include <functional>

#include "settings.hpp"

#include "data_types.hpp"
#include "shaders.hpp"
#include "ray.hpp"

struct ObjectInfo {
    int vertices = 0;
    int faces = 0;
    int objects = 0;
    
    void operator+=(const ObjectInfo &);
};

struct ObjectHit {
    float distance;
    function<Vector3()> getNormal;
    function<Color()> getTexture;
};


class Object {
protected:
    Vector3 center;
    Matrix3x3 rotation, Irotation;
    
public:
    Material material;
    
    Object(Vector3, Vector3, Material);
    /***/ Vector3 getCenter() const;
    /***/ Matrix3x3 getRotation() const;
    /***/ Matrix3x3 getInverseRotation() const;
    /***/ Vector3 toObjectSpace(Vector3 point) const;
    /***/ Vector3 toWorldSpace(Vector3 _point) const;
    /***/ virtual ObjectHit intersect(Vector3 origin, Vector3 direction) const = 0;
    /***/ virtual ObjectInfo getInfo() const = 0;
};


class Sphere : public Object {
private:
    float radius;
    float radius2;
    
public:
    Sphere(Vector3, float, Vector3, Material);
    Vector3 getNormal(Vector3) const;
    Color getTexture(Vector3) const;
    ObjectHit intersect(Vector3, Vector3) const;
    ObjectInfo getInfo() const;
};


class Cuboid : public Object {
private:
    Vector3 size;
    Vector3 vmin, vmax;
    
public:
    Cuboid(Vector3, float, Vector3, Material);
    Cuboid(Vector3, float, float, float, Vector3, Material);
    Cuboid(Vector3, Vector3, Vector3, Material);
    Vector3 getNormal(Vector3) const;
    Color getTexture(Vector3) const;
    ObjectHit intersect(Vector3, Vector3) const;
    ObjectInfo getInfo() const;
};


class Plane : public Object {
private:
    float size_x, size_y;
    
public:
    Plane(Vector3, float, float, Vector3, Material);
    Vector3 getNormal(Vector3) const;
    Color getTexture(Vector3) const;
    ObjectHit intersect(Vector3, Vector3) const;
    ObjectInfo getInfo() const;
};


class Triangle {
private:
    bool vn, tc;
    Vector3 v0, v0v1, v0v2;
    array<VectorUV, 3> textures;
    array<Vector3, 3> normals;
    Material &material;
    
public:
    explicit Triangle(array<Vector3, 3>, array<VectorUV, 3>, array<Vector3, 3>, Material &);
    Vector3 getNormal(VectorUV) const;
    Color getTexture(VectorUV) const;
    ObjectHit intersect(Vector3, Vector3) const;
    ObjectInfo getInfo() const;
};


class Mesh : public Object {
private:
    vector<Triangle> triangles;
    Cuboid bounds;
    
public:
    Mesh(vector<array<Vector3, 3>>, vector<array<VectorUV, 3>>, vector<array<Vector3, 3>>, Vector3, float, Vector3, Material);
    ObjectHit intersect(Vector3, Vector3) const;
    ObjectInfo getInfo() const;
};
