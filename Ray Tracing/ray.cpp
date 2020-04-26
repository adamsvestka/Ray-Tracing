//
//  ray.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "ray.hpp"

Intersection castRay(Vector3 origin, Vector3 direction, vector<Shape *> objects, vector<Light> lights, float step_size, bool detect, short bounces) {
    Intersection info;
    
    info.hit = false;
    info.position = origin;
    info.object = NULL;
    info.distance = settings.max_render_distance;
    info.color = settings.environment_color;
    info.shaded = settings.environment_color * settings.environment_intensity;
    info.shadow = info.diffuse = info.specular = info.reflection = info.refraction = false;
    
    if (++bounces > settings.max_light_bounces) return info;
    
    Vector3 increment = direction * step_size;
    const int repetitions = settings.max_render_distance / step_size;
    
    vector<pair<Vector3, Vector3>> pos(objects.size());
    for (int j = 0; j < objects.size(); j++) pos[j] = {objects[j]->getRotation() * (origin - objects[j]->getCenter()), objects[j]->getRotation() * increment};
    
    for (int i = 0; i < repetitions && !info.hit; i++) {
        for (int j = 0; j < objects.size(); j++) {
            pos[j].first += pos[j].second;
            if (objects[j]->intersects(pos[j].first)) {
                info.hit = true;
                info.object = objects[j];
                // Don't waste time calculating light if not necessary
                if (detect) {
                    info.position = origin + increment * i;
                    info.distance = (info.position - origin).length();
                    return info;
                }
                // Smooth surfaces
                Vector3 a = pos[j].first - pos[j].second, b = pos[j].first, c;
                for (int k = 0; k < settings.surface_smoothing; k++) {
                    c = (a + b) / 2;
                    if (objects[j]->intersects(c)) b = c;
                    else a = c;
                }
                info.position = objects[j]->getSurface(info.object->getInverseRotation() * a + info.object->getCenter());
                info.distance = (info.position - origin).length();
                
                break;
            }
        }
    }
    
    if (!info.hit) {
        info.position = origin + increment * repetitions;
        return info;
    }
    
    info.normal = info.object->getNormal(info.position);
    
    Color diffuse, specular, reflection;
    diffuse = specular = reflection = Black;
    
    diffuse += settings.environment_color * settings.environment_intensity;
    
    for (std::vector<Light>::iterator light = lights.begin(); light != lights.end(); ++light) {
        Vector3 vector_to_light = light->getPosition() - info.position;
        
        if (castRay(info.position, vector_to_light.normal(), objects, lights, step_size, true, bounces).hit) {
            info.shadow = true;
            continue;
        }
        
        float cosine_term = vector_to_light.normal() * info.normal;
        
        if (info.object->material.Ks < 1) diffuse += light->getValue() * (float)(fmax(cosine_term, 0.f) / (vector_to_light * vector_to_light * 4 * M_PI));
        if (info.object->material.Ks > 0) specular += light->getValue() * (pow(fmax(-(info.normal * (cosine_term * 2) - vector_to_light.normal()) * direction, 0.f), info.object->material.n));
        if (info.object->material.reflection > 0) reflection += castRay(info.position, direction - info.normal * 2 * (direction * info.normal), objects, lights, step_size, false, bounces).shaded;
    }
    
    if (diffuse != Black) info.diffuse = true;
    if (specular != Black) info.specular = true;
    if (reflection != Black) info.reflection = true;
//    if (refraction != Black) info.refraction = true;
    
    auto material = info.object->material;
    info.color = material.color;
    
    info.light = diffuse * material.albedo * (1 - material.Ks) + specular * material.Ks;
    info.shaded = (material.color * diffuse * material.albedo * (1 - material.Ks) + specular * material.Ks) * (1 - material.reflection) + reflection * material.reflection;
    
    // I = Ka + Ke + sum(Vi * Li * (Kd * max(li . n, 0) + Ks * (max(hi . n, 0))) ^ s) + Ks * Ir;
    
    return info;
}
