//
//  shapes.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "shapes.hpp"

Vector3 Shape::getCenter() { return center; };


Sphere::Sphere(Vector3 position, float radius) {
    this->center = position;
    this->radius = radius;
}

bool Sphere::intersects(Vector3 point) {
    return abs((center - point).length()) < radius;
}

Vector3 Sphere::getNormal(Vector3 point) {
    return (center - point).normal();
}

Vector3 Sphere::getSurface(Vector3 point) {
    return center - getNormal(point) * radius;
}
