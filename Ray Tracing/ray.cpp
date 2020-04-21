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
}

Vector3 Ray::getIntersectionPosition() { return position; }
Shape *Ray::getIntersectingObject() { return object; }

float Ray::traceObject(std::vector<Shape*> objects) {
    Vector3 increment = direction * settings.step_size;
    
    for (int i = 0; i < settings.repetitions; i++) {
        position = position + increment;
        for (std::vector<Shape*>::iterator it = objects.begin(); it != objects.end(); ++it) {
            if ((*it)->intersects(position)) {
                object = *it;
                position = (*it)->getSurface(position);
                return (position - origin).length();
            }
        }
    }
    
    return -1;
}

Color Ray::traceLight(std::vector<Light> lights, std::vector<Shape*> objects) {
    Vector3 surface_normal = getIntersectingObject()->getNormal(getIntersectionPosition());
    if (settings.render_special == RENDER_NORMALS) return surface_normal.toColor();
    
    Color diffuse, specular, reflection;
    diffuse = specular = reflection = Black;
    
    diffuse += settings.environment_color * settings.environment_intensity;
    
    for (std::vector<Light>::iterator light = lights.begin(); light != lights.end(); ++light) {
        Vector3 vector_to_light = light->getVector(*this);

        Ray ray(position, vector_to_light.normal());
        if (ray.traceObject(objects) > -1) {
            if (settings.render_special == RENDER_SHADOWS) diffuse = Red;
            break;
        }
        
        float cosine_term = vector_to_light.normal() * surface_normal;
        
        if (object->material.Ks < 1) diffuse += light->color * (float)(light->getIntensity() * fmax(cosine_term, 0.f) / (vector_to_light * vector_to_light * 4 * M_PI));
        if (object->material.Ks > 0) specular += light->color * (light->getIntensity() * pow(fmax(-(surface_normal * (cosine_term * 2) - vector_to_light.normal()) * direction, 0.f), object->material.n));
        if (object->material.reflection > 0) {
            Ray ray2(position, direction - surface_normal * 2 * (direction * surface_normal));
            if (ray2.traceObject(objects) > -1) reflection += ray2.traceLight(lights, objects);
            else reflection += settings.environment_color * settings.environment_intensity;
        }
    }
    
    if (settings.render_special == RENDER_LIGHT || settings.render_special == RENDER_SHADOWS) return (diffuse * object->material.albedo + specular);
    return ((object->material.color * diffuse) * (object->material.albedo * (1 - object->material.Ks)) + (specular * object->material.Ks)) * (1 - object->material.reflection) + reflection * object->material.reflection;
    
    // I = Ka + Ke + sum(Vi * Li * (Kd * max(li . n, 0) + Ks * (max(hi . n, 0))) ^ s) + Ks * Ir;
}
