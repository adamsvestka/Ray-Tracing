//
//  shapes.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "shapes.hpp"

Vector3 Shape::getCenter() const { return center; };
Matrix3x3 Shape::getRotation() const { return rotation; }
Matrix3x3 Shape::getInverseRotation() const { return Irotation; }
Vector3 Shape::toObjectSpace(Vector3 point) const { return Irotation * (point - center); };
Vector3 Shape::toWorldSpace(Vector3 _point) const { return rotation * _point + center; };


// MARK: - Sphere
/// @param position Vector3{x, y, z}
/// @param radius float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Sphere::Sphere(Vector3 position, float radius, Vector3 angles, Material material) : radius(radius) {
    this->center = position;
    this->material = material;
    
    rotation = rot_mat(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
    if (this->material.transparent) this->material.Ks = 1;
}

bool Sphere::intersects(Vector3 _point, Vector3 _direction) const {
    return abs(_point.length()) < radius;
}

Vector3 Sphere::getNormal(Vector3 _point, Vector3 _direction) const {
    return rotation * _point.normal();
}

Vector3 Sphere::getSurface(Vector3 _point, Vector3 _direction) const {
    return toWorldSpace(_point.normal() * radius);
}

Color Sphere::getTexture(Vector3 _point) const {
    float u = asin(min(max(_point.z / radius, -1.f), 1.f)) / (2 * M_PI) + 0.25;
    float v = atan2(min(max(_point.x / radius, -1.f), 1.f), min(max(_point.y / radius, -1.f), 1.f)) / (2 * M_PI) + 0.5;
    
    return material.texture(min(max(u, 0.f), nextafter(1.f, 0.f)), min(max(v, 0.f), nextafter(1.f, 0.f)));
}


// MARK: - Cube
/// @param position Vector3{x, y, z}
/// @param size_x float
/// @param size_y float
/// @param size_z float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cuboid::Cuboid(Vector3 position, float size_x, float size_y, float size_z, Vector3 angles, Material material) : size_x(size_x), size_y(size_y), size_z(size_z) {
    this->center = position;
    this->material = material;
    
    rotation = rot_mat(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
    if (this->material.transparent) this->material.Ks = 1;
}

bool Cuboid::intersects(Vector3 _point, Vector3 _direction) const {
    return abs(_point.x) < size_x && abs(_point.y) < size_y && abs(_point.z) < size_z;
}

Vector3 Cuboid::getNormal(Vector3 _point, Vector3 _direction) const {
    if (abs(_point.x) > abs(_point.y) && abs(_point.x) > abs(_point.z)) return rotation * Vector3{_point.x > 0 ? 1.f : -1.f, 0, 0};
    else if (abs(_point.y) > abs(_point.z)) return rotation * Vector3{0, _point.y > 0 ? 1.f : -1.f, 0};
    else return rotation * Vector3{0, 0, _point.z > 0 ? 1.f : -1.f};
}

Vector3 Cuboid::getSurface(Vector3 _point, Vector3 _direction) const {
    float x = (size_x - abs(_point.x)) / abs(_direction.x);
    float y = (size_y - abs(_point.y)) / abs(_direction.y);
    float z = (size_z - abs(_point.z)) / abs(_direction.z);
    
    if (x < y && x < z) {
        _point.y -= _direction.y * x;
        _point.z -= _direction.z * x;
        _point.x = _point.x > 0 ? size_x : -size_x;
    } else if (y < z) {
        _point.x -= _direction.x * y;
        _point.z -= _direction.z * y;
        _point.y = _point.y > 0 ? size_y : -size_y;
    } else {
        _point.x -= _direction.x * z;
        _point.y -= _direction.y * z;
        _point.z = _point.z > 0 ? size_z : -size_z;
    }
    
    return toWorldSpace(_point);
}

Color Cuboid::getTexture(Vector3 _point) const {
    float u, v;
    
    if (abs(_point.x) > abs(_point.y) && abs(_point.x) > abs(_point.z)) {
        v = _point.y / (2 * size_y) + 0.5;
        u = _point.z / (2 * size_z) + 0.5;
    } else if (abs(_point.y) > abs(_point.z)) {
        v = _point.x / (2 * size_x) + 0.5;
        u = _point.z / (2 * size_z) + 0.5;
    } else {
        u = _point.x / (2 * size_x) + 0.5;
        v = _point.y / (2 * size_y) + 0.5;
    }
    
    return material.texture(min(max(u, 0.f), nextafter(1.f, 0.f)), min(max(v, 0.f), nextafter(1.f, 0.f)));
}


// MARK: - Cylinder
/// @param position Vector3{x, y, z}
/// @param radius float
/// @param height float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Cylinder::Cylinder(Vector3 position, float radius, float height, Vector3 angles, Material material) : radius(radius), height(height) {
    this->center = position;
    this->material = material;
    
    rotation = rot_mat(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
    if (this->material.transparent) this->material.Ks = 1;
}

bool Cylinder::intersects(Vector3 _point, Vector3 _direction) const {
    return sqrt(pow(_point.x, 2) + pow(_point.y, 2)) < radius && abs(_point.z) < height;
}

Vector3 Cylinder::getNormal(Vector3 _point, Vector3 _direction) const {
    if (abs(_point.z) / height > sqrt(pow(_point.x, 2) + pow(_point.y, 2)) / radius) return rotation * Vector3{0, 0, _point.z > 0 ? 1.f : -1.f};
    else return rotation * Vector3{_point.x, _point.z, 0}.normal();
}

Vector3 Cylinder::getSurface(Vector3 _point, Vector3 _direction) const {
    if (abs(_point.z) / height > sqrt(pow(_point.x, 2) + pow(_point.y, 2)) / radius) _point.z = _point.z > 0 ? height : -height;
    else {
        auto temp = Vector3{_point.x, _point.y}.normal() * radius;
        _point = {temp.x, temp.y, _point.z};
    }
    return toWorldSpace(_point);
}

Color Cylinder::getTexture(Vector3 _point) const {
    float u = _point.z / (2 * height) + 0.5;
    float v = atan2(min(max(_point.x / radius, -1.f), 1.f), min(max(_point.y / radius, -1.f), 1.f)) / (2 * M_PI) + 0.5;
    
    return material.texture(min(max(u, 0.f), nextafter(1.f, 0.f)), min(max(v, 0.f), nextafter(1.f, 0.f)));
}


// MARK: - Plane
/// @param position Vector3{x, y, z}
/// @param size_x float
/// @param size_y float
/// @param angles Vector3{x, y, z}
/// @param material Material{texture, n, Ks, ior, transparent}
Plane::Plane(Vector3 position, float size_x, float size_y, Vector3 angles, Material material) : size_x(size_x), size_y(size_y) {
    this->center = position;
    this->material = material;
    
    rotation = rot_mat(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
    if (this->material.transparent) this->material.Ks = 1;
}

bool Plane::intersects(Vector3 _point, Vector3 _direction) const {
    float dx = _direction.x / _direction.z * _point.z;
    float dy = _direction.y / _direction.z * _point.z;
    
    if (_point.z > 0) return abs(_point.x + dx) < size_x && abs(_point.y + dy) < size_y && _point.z < -_direction.z;
    else return abs(_point.x - dx) < size_x && abs(_point.y - dy) < size_y && _point.z > -_direction.z;
}

Vector3 Plane::getNormal(Vector3 _point, Vector3 _direction) const {
    return rotation * Vector3{0, 0, _direction.z < 0 ? 1.f : -1.f};
}

Vector3 Plane::getSurface(Vector3 _point, Vector3 _direction) const {
    _point.x -= _direction.x / _direction.z * _point.z;
    _point.y -= _direction.y / _direction.z * _point.z;
    _point.z = 0;
    
    return toWorldSpace(_point);
}

Color Plane::getTexture(Vector3 _point) const {
    float u = _point.x / (2 * size_x) + 0.5;
    float v = _point.y / (2 * size_y) + 0.5;
    
    return material.texture(min(max(u, 0.f), nextafter(1.f, 0.f)), min(max(v, 0.f), nextafter(1.f, 0.f)));
}
