//
//  shapes.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "shapes.hpp"

Vector3 Shape::getCenter() { return center; };


// MARK: - Sphere
Sphere::Sphere(Vector3 position, float radius, Material material) {
    this->center = position;
    this->radius = radius;
    this->material = material;
}

bool Sphere::intersects(Vector3 point) {
    return abs((point - center).length()) < radius;
}

Vector3 Sphere::getNormal(Vector3 point) {
    return (point - center).normal();
}

Vector3 Sphere::getSurface(Vector3 point) {
    return center + getNormal(point) * radius;
}


// MARK: - Cube
Cube::Cube(Vector3 position, float radius, Material material) {
    this->center = position;
    this->radius = radius;
    this->material = material;
}

bool Cube::intersects(Vector3 point) {
    return abs(point.x - center.x) < radius && abs(point.y - center.y) < radius && abs(point.z - center.z) < radius;
}

Vector3 Cube::getNormal(Vector3 point) {
    Vector3 d = point - center;
    if (abs(d.x) > abs(d.y) && abs(d.x) > abs(d.z)) return {d.x < 0 ? 1.f : -1.f, 0, 0};
    else if (abs(d.y) > abs(d.z)) return {0, d.y > 0 ? 1.f : -1.f, 0};
    else return {0, 0, d.z > 0 ? 1.f : -1.f};
}

Vector3 Cube::getSurface(Vector3 point) {
    Vector3 d = point - center;
    if (d.x - settings.step_size < -radius) point.x = center.x - radius;
    else if (d.x + settings.step_size > radius) point.x = center.x + radius;
    else if (d.y - settings.step_size < -radius) point.y = center.y - radius;
    else if (d.y + settings.step_size > radius) point.y = center.y + radius;
    else if (d.z - settings.step_size < -radius) point.z = center.z - radius;
    else if (d.z + settings.step_size > radius) point.z = center.z + radius;
    return point;
}
