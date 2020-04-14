//
//  ray.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "ray.hpp"

Ray::Ray(Vector3 position, Vector3 direction) {
    origin = this->position = position;
    this->direction = direction;
    object = NULL;
    distance = 0;
}

Vector3 Ray::getIntersectionPosition() { return position; }
Shape *Ray::getIntersectingObject() { return object; }

float Ray::traceObject(Shape *objects[], int count) {
    for (int i = 0; i < RENDER_DISTANCE / STEP_SIZE; i++) {
        this->operator++();
        for (int i = 0; i < count; i++) {
            if (objects[i]->intersects(position)) {
                object = objects[i];
                position = object->getSurface(position);
                distance = (position - origin).length();
                return distance;
            }
        }
    }
    
    return -1;
}

float Ray::traceLight(Light light, Shape *objects[], int count) {
    Vector3 surface_normal = getIntersectingObject()->getNormal(getIntersectionPosition());
    Vector3 vector_to_light = light.getVector(*this);

//        origin = position;
    direction = Vector3{0, 0, 0} - vector_to_light.normal();
//        distance = 0;
    
    count--;
    Shape *objects2[count];
    for (int i = 0, j = 0; i < count+1; i++) if (objects[i] != object) objects2[j++] = objects[i];
    for (int i = 0; i < RENDER_DISTANCE / STEP_SIZE; i++) {
        this->operator++();
        for (int j = 0; j < count; j++) if (objects2[j]->intersects(position)) return -1;
    }
    
    float cosine_term = surface_normal * vector_to_light.normal();
    if (cosine_term < 0) cosine_term = 0;

    return M_1_PI * cosine_term * light.getIntensity() / pow(vector_to_light.length(), 2);
}

void Ray::operator++() {
    position = position + direction * STEP_SIZE;
}
