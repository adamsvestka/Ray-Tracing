//
//  shapes.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "shapes.hpp"

inline float guard(float f) { return clamp(f, 0.f, nextafter(1.f, 0.f)); }

Shape::Shape(Vector3 position, Vector3 angles, Material material) : center(position), material(material) {
    rotation = Matrix3x3::RotationMatrix(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
    if (this->material.transparent) this->material.Ks = 1;
}
Vector3 Shape::getCenter() const { return center; };
Matrix3x3 Shape::getRotation() const { return rotation; }
Matrix3x3 Shape::getInverseRotation() const { return Irotation; }
Vector3 Shape::toObjectSpace(Vector3 point) const { return Irotation * (point - center); };
Vector3 Shape::toWorldSpace(Vector3 _point) const { return rotation * _point + center; };


// MARK: - Sphere
/// @param position Vector3{x, y, z}
/// @param diameter float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Sphere::Sphere(Vector3 position, float diameter, Vector3 angles, Material material) : Shape(position, angles, material) {
    this->radius = diameter / 2;
    this->radius2 = pow(radius, 2);
}

float Sphere::intersect(Vector3 origin, Vector3 direction) const {
    float t0, t1;
    
    Vector3 path = center - origin;
    float tca = path * direction;
    if (tca < 0) return -1;
    
    float d2 = path * path - tca * tca;
    if (d2 > radius2) return -1;
    
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;
    if (t0 > t1) swap(t0, t1);
    
    if (t0 < 0) {
        if (t1 < 0) return -1;
        return t1;
    }
    
    return t0;
}

Vector3 Sphere::getNormal(Vector3 point, Vector3 direction) const {
    return rotation * toObjectSpace(point).normal();
}

Color Sphere::getTexture(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    float u = asin(clamp(_point.z / radius, -1.f, 1.f)) / (2 * M_PI) + 0.25;
    float v = atan2(clamp(_point.x / radius, -1.f, 1.f), clamp(_point.y / radius, -1.f, 1.f)) / (2 * M_PI) + 0.5;
    
    return material.texture(guard(u), guard(v));
}


// MARK: - Cuboid
/// @param position Vector3{x, y, z}
/// @param size float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 position, float size, Vector3 angles, Material material) : Cuboid(position, size, size, size, angles, material) {};

/// @param position Vector3{x, y, z}
/// @param size_x float
/// @param size_y float
/// @param size_z float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 position, float size_x, float size_y, float size_z, Vector3 angles, Material material) : Shape(position, angles, material) {
    this->size = Vector3{size_x, size_y, size_z};
    this->vmin = position - size / 2;
    this->vmax = position + size / 2;
}

/// @param corner_min Vector3{x, y, z}
/// @param corner_max Vector3{x, y, z}
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 corner_min, Vector3 corner_max, Vector3 angles, Material material) : Shape((corner_min + corner_max) / 2, angles, material) {
    this->size = corner_max - corner_min;
    this->vmin = corner_min;
    this->vmax = corner_max;
}

float Cuboid::intersect(Vector3 origin, Vector3 direction) const {
    Vector3 _origin = toObjectSpace(origin) + center;
    Vector3 _direction = Irotation * direction;
    
    Vector3 tmin = (vmin - _origin) / _direction;
    Vector3 tmax = (vmax - _origin) / _direction;
    
    if (tmin.x > tmax.x) swap(tmin.x, tmax.x);
    if (tmin.y > tmax.y) swap(tmin.y, tmax.y);
    if (tmin.z > tmax.z) swap(tmin.z, tmax.z);
    
    if (tmin.x > tmax.y || tmin.y > tmax.x) return -1;
    if (tmin.y > tmin.x) tmin.x = tmin.y;
    if (tmax.y < tmax.x) tmax.x = tmax.y;
    
    if (tmin.x > tmax.z || tmin.z > tmax.x) return -1;
    if (tmin.z > tmin.x) tmin.x = tmin.z;
    if (tmax.z < tmax.x) tmax.x = tmax.z;
    
    return tmin.x;
}

Vector3 Cuboid::getNormal(Vector3 point, Vector3 direction) const {
    const Vector3 _point = toObjectSpace(point);
    
    if (abs(_point.x) > abs(_point.y) && abs(_point.x) > abs(_point.z)) return rotation * Vector3{_point.x > 0 ? 1.f : -1.f, 0, 0};
    else if (abs(_point.y) > abs(_point.z)) return rotation * Vector3{0, _point.y > 0 ? 1.f : -1.f, 0};
    else return rotation * Vector3{0, 0, _point.z > 0 ? 1.f : -1.f};
}

Color Cuboid::getTexture(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    float u, v;
    if (abs(_point.x) > abs(_point.y) && abs(_point.x) > abs(_point.z)) {
        v = _point.y / size.y + 0.5;
        u = _point.z / size.z + 0.5;
    } else if (abs(_point.y) > abs(_point.z)) {
        v = 0.5 - _point.x / size.x;
        u = _point.z / size.z + 0.5;
    } else {
        u = _point.x / size.x + 0.5;
        v = _point.y / size.y + 0.5;
    }
    
    return material.texture(guard(u), guard(v));
}


// MARK: - Plane
/// @param position Vector3{x, y, z}
/// @param size_x float
/// @param size_y float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Plane::Plane(Vector3 position, float size_x, float size_y, Vector3 angles, Material material) : Shape(position, angles, material) {
    this->size_x = size_x;
    this->size_y = size_y;
}

float Plane::intersect(Vector3 origin, Vector3 direction) const {
    Vector3 normal = getNormal(Vector3::Zero, direction);
    float denom = normal * direction;
    
    if (denom < 0) {
        Vector3 path = center - origin;
        float t = path * normal / denom;
        
        Vector3 _point = toObjectSpace(origin + direction * t);
        if (abs(_point.x) > size_x / 2 || abs(_point.y) > size_y / 2) return -1;
        return t;
    }
    
    return -1;
}

Vector3 Plane::getNormal(Vector3 point, Vector3 direction) const {
    return rotation * Vector3{0, 0, (Irotation * direction).z < 0 ? 1.f : -1.f};
}

Color Plane::getTexture(Vector3 point) const {
    const Vector3 _point = toObjectSpace(point);
    
    float u = _point.x / size_x + 0.5;
    float v = _point.y / size_y + 0.5;
    
    return material.texture(guard(u), guard(v));
}
