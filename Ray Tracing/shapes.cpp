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


// MARK: - Sphere
Sphere::Sphere(Vector3 position, float radius, Vector3 angles, Material material) {
    this->center = position;
    this->radius = radius;
    this->material = material;
    
    rotation = rot_mat(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
}

bool Sphere::intersects(Vector3 d) const {
    return abs(d.length()) < radius;
}

Vector3 Sphere::getNormal(Vector3 point) const {
    return (point - center).normal();
}

Vector3 Sphere::getSurface(Vector3 point) const {
    return center + getNormal(point) * radius;
}

Color Sphere::getTexture(Vector3 point) const {
    float u = asin(min(max(point.z / radius, -1.f), 1.f)) / (2 * M_PI) + 0.25;
    float v = atan2(min(max(point.x / radius, -1.f), 1.f), min(max(point.y / radius, -1.f), 1.f)) / (2 * M_PI) + 0.5;
    
    return material.texture(min(max(u, 0.f), nextafter(1.f, 0.f)), min(max(v, 0.f), nextafter(1.f, 0.f)));
}


// MARK: - Cube
Cube::Cube(Vector3 position, float radius, Vector3 angles, Material material) {
    this->center = position;
    this->radius = radius;
    this->material = material;
    
    rotation = rot_mat(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    Irotation = rotation.inverse();
}

bool Cube::intersects(Vector3 d) const {
    return abs(d.x) < radius && abs(d.y) < radius && abs(d.z) < radius;
}

Vector3 Cube::getNormal(Vector3 point) const {
    Vector3 d = rotation * (point - center);
    
    if (abs(d.x) > abs(d.y) && abs(d.x) > abs(d.z)) return Irotation * Vector3{d.x > 0 ? 1.f : -1.f, 0, 0};
    else if (abs(d.y) > abs(d.z)) return Irotation * Vector3{0, d.y > 0 ? 1.f : -1.f, 0};
    else return Irotation * Vector3{0, 0, d.z > 0 ? 1.f : -1.f};
}

Vector3 Cube::getSurface(Vector3 point) const {
    Vector3 d = rotation * (point - center);
    
    if (d.x - settings.quick_step_size < -radius) d.x = -radius;
    else if (d.x + settings.quick_step_size > radius) d.x = radius;
    else if (d.y - settings.quick_step_size < -radius) d.y = -radius;
    else if (d.y + settings.quick_step_size > radius) d.y = radius;
    else if (d.z - settings.quick_step_size < -radius) d.z = -radius;
    else if (d.z + settings.quick_step_size > radius) d.z = radius;
    return Irotation * d + center;
}

Color Cube::getTexture(Vector3 point) const {
    float u = point.x / (2 * radius) + 0.5;
    float v = point.y / (2 * radius) + 0.5;
    
    return material.texture(min(max(u, 0.f), nextafter(1.f, 0.f)), min(max(v, 0.f), nextafter(1.f, 0.f)));
}
