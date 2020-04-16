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
Sphere::Sphere(Vector3 position, float radius) {
    this->center = position;
    this->radius = radius;
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
Cube::Cube(Vector3 position, float radius) {
    this->center = position;
    this->radius = radius;
}

bool Cube::intersects(Vector3 point) {
    return abs(center.x - point.x) < radius && abs(center.y - point.y) < radius && abs(center.z - point.z) < radius;
}

Vector3 Cube::getNormal(Vector3 point) {
    Vector3 d = center - point;
    return Vector3{(float)(abs(d.x) < radius ? 0 : (d.x > 0 ? -1 : 1)), (float)(abs(d.y) < radius ? 0 : (d.y > 0 ? -1 : 1)), (float)(abs(d.z) < radius ? 0 : (d.z > 0 ? -1 : 1))}.normal();
}

Vector3 Cube::getSurface(Vector3 point) {
    Vector3 d = center - point;
    if (abs(d.x + radius) < STEP_SIZE) point.x = -radius;
    else if (abs(d.x - radius) < STEP_SIZE) point.x = radius;
    if (abs(d.y + radius) < STEP_SIZE) point.y = -radius;
    else if (abs(d.y - radius) < STEP_SIZE) point.y = radius;
    if (abs(d.z + radius) < STEP_SIZE) point.z = -radius;
    else if (abs(d.z - radius) < STEP_SIZE) point.z = radius;
    return point;
}
