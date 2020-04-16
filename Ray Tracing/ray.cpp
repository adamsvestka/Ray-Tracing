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

float Ray::traceObject(std::vector<Shape*> objects) {
    int repetitions = RENDER_DISTANCE / STEP_SIZE;
    Vector3 increment = direction * STEP_SIZE;
    
    for (int i = 0; i < repetitions; i++) {
        position = position + increment;
        for (std::vector<Shape*>::iterator it = objects.begin(); it != objects.end(); ++it) {
            if ((*it)->intersects(position)) {
                object = *it;
                position = (*it)->getSurface(position);
                distance = (position - origin).length();
                return distance;
            }
        }
    }
    
    return -1;
}

float Ray::traceLight(std::vector<Light> lights, std::vector<Shape*> objects) {
    Vector3 surface_normal = getIntersectingObject()->getNormal(getIntersectionPosition());
    float intensity = 0;
    
    for (std::vector<Light>::iterator light = lights.begin(); light != lights.end(); ++light) {
        Vector3 vector_to_light = light->getVector(*this);

    //        origin = position;
        direction = vector_to_light.normal();
    //        distance = 0;
        
        for (std::vector<Shape*>::iterator object = objects.begin(); object != objects.end(); ++object) if (*object == this->object) objects.erase(object--);
        int repetitions = vector_to_light.length() / STEP_SIZE;
        Vector3 increment = direction * STEP_SIZE;
        
        for (int i = 0; i < repetitions; ++i) {
            position = position + increment;
            for (std::vector<Shape*>::iterator object = objects.begin(); object != objects.end(); ++object) if ((*object)->intersects(position)) return -1;
        }
        
        float cosine_term = surface_normal * vector_to_light.normal();
        if (cosine_term < 0) cosine_term = 0;
        
        intensity = fmax(intensity, M_1_PI * cosine_term * light->getIntensity() / pow(vector_to_light.length(), 2));
    }
    
    return intensity;
}
