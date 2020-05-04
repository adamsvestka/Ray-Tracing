//
//  ray.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "ray.hpp"

// MARK: Intersection
Color Intersection::shaded() {
    if (!hit) return settings.background_color;
    else {
        Color light = Black;
        for (int i = 0; i < shadows.size(); i++) if (!shadows[i]) light += texture * diffuse[i] * (1 - object->material.Ks) + specular[i] * object->material.Ks;
        
        return (ambient * (1 - object->material.Ks) + light) * !object->material.transparent + reflection * object->material.Ks * kr + transmission * object->material.transparent * (1 - kr);
    }
    
    // I = Ka + Ke + sum(Vi * Li * (Kd * max(li . n, 0) + Ks * (max(hi . n, 0))) ^ s) + Ks * Ir + Kt * It;
    /*
     Ka ... ambient (environment) lighting
   - Ke ... emission lighting
     Vi ... is not in shadow?
     Li ... light value
     Kd ... object diffuse ratio
     Ks ... object specular (reflectivity) ratio
     s  ... shininess
     Ir ... recursive reflection
     Kt ... object transmission ratio
     It ... recursive transmission
     */
}

// MARK: reflect
Vector3 reflect(Vector3 direction, Vector3 normal) {
    return direction - normal * 2 * (direction * normal);
}

// MARK: fresnel
float fresnel(Vector3 direction, Vector3 normal, float ior) {
    float cosi = min(max(direction * normal, -1.f), 1.f);
    float etai = 1, etat = ior;
    if (cosi > 0) swap(etai, etat);
    float sint = etai / etat * sqrt(max(0.f, 1 - cosi * cosi));
    if (sint >= 1) return 1;
    else {
        float cost = sqrt(max(0.f, 1 - sint * sint));
        cosi = abs(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
}

// MARK: refract
Vector3 refract(Vector3 direction, Vector3 normal, float ior) {
    float cosi = min(max(direction * normal, -1.f), 1.f);
    float etai = 1, etat = ior;
    if (cosi < 0) cosi = -cosi;
    else {
        swap(etai, etat);
        normal = -normal;
    }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return (k < 0 ? Zero : direction * eta + normal * (eta * cosi - sqrt(k))).normal();
}

// MARK: castRay
Intersection castRay(Vector3 origin, Vector3 direction, const vector<Shape *> &objects, const vector<Light *> &lights, Input mask) {
    Intersection info;
    
    info.hit = false;
    info.position = origin;
    info.distance = settings.max_render_distance;
    info.object = nullptr;
    info.normal = Zero;
    info.light = 0;
    info.kr = 1;
    info.shadows = vector<bool>(lights.size(), false);
    info.diffuse = info.specular = valarray<Color>(Black, objects.size());
    info.ambient = settings.ambient_lighting;
    info.texture = info.reflection = info.transmission = Black;
    
    if (++mask.bounce_count > settings.max_light_bounces) return info;
    
    Vector3 increment = direction * mask.step_size;
    const int repetitions = settings.max_render_distance / mask.step_size;
    
    vector<pair<Vector3, Vector3>> pos(objects.size());
    vector<bool> inter(objects.size());
    for (int j = 0; j < objects.size(); j++) {
        pos[j] = {objects[j]->getRotation() * (origin - objects[j]->getCenter()), objects[j]->getRotation() * increment};
        inter[j] = objects[j]->intersects(pos[j].first + pos[j].second.normal() * 0.001);
    }
    
    for (int i = 0; i < repetitions && !info.hit; i++) {
        for (int j = 0; j < objects.size(); j++) {
            pos[j].first += pos[j].second;
            if (objects[j]->intersects(pos[j].first) != inter[j] && !(!mask.lighting && objects[j]->material.transparent)) {
                info.hit = true;
                info.object = objects[j];
                // Don't waste time calculating light if not necessary
                if (!mask.lighting) {
                    info.position = origin + increment * i;
                    info.distance = (info.position - origin).length();
                    return info;
                }
                // Smooth surfaces
                Vector3 a = pos[j].first - pos[j].second, b = pos[j].first, c;
                for (int k = 0; k < settings.surface_smoothing; k++) {
                    c = (a + b) / 2;
                    if (info.object->intersects(c) != inter[j]) b = c;
                    else a = c;
                }
                info.texture = info.object->getTexture(a);
                info.position = info.object->getSurface(info.object->getInverseRotation() * a + info.object->getCenter());
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
    
    // MARK: Diffuse, Specular
    if (mask.diffuse && !info.object->material.transparent) {
        auto shadow_mask = mask;
        shadow_mask.step_size = settings.quick_step_size;
        shadow_mask.lighting = false;
        for (int i = 0; i < lights.size(); i++) {
            auto light = lights[i];
            Vector3 vector_to_light = light->position - info.position;
            
            const float cosine_term = vector_to_light.normal() * info.normal;
            
            if (info.object->material.Ks < 1) info.diffuse[i] = light->color * light->intensity * (float)(fmax(cosine_term, 0.f) / (vector_to_light * vector_to_light * 4 * M_PI));
            if (info.object->material.Ks > 0) info.specular[i] = light->color * (float)pow(light->intensity, 0.3) * (pow(fmax(-(info.normal * (cosine_term * 2) - vector_to_light.normal()) * direction, 0.f), info.object->material.n));
            
            if (mask.shadows[i] && castRay(info.position, vector_to_light.normal(), objects, lights, shadow_mask).hit) {
                info.shadows[i] = true;
                continue;
            }
            
            info.light += info.diffuse[i].value() * (1 - info.object->material.Ks) + info.specular[i].value() * info.object->material.Ks;
        }
    }
    
    // MARK: Reflection
    auto reflect_mask = mask;
    reflect_mask.diffuse = reflect_mask.reflections = reflect_mask.transmission = true;
    reflect_mask.shadows = vector<bool>(lights.size(), true);
    
    if (mask.reflections && (info.object->material.Ks > 0 || info.object->material.transparent)) {
        reflect_mask.step_size = settings.quick_step_size;
        
        auto ray = castRay(info.position, reflect(direction, info.normal), objects, lights, reflect_mask);
        info.reflection = ray.shaded();
    }
    
    // MARK: Transmission
    if (info.object->material.transparent) {
        if ((info.kr = fresnel(direction, info.normal, info.object->material.ior)) < 1 && mask.transmission) {
            reflect_mask.step_size = settings.precise_step_size;
            
            auto ray = castRay(info.position, refract(direction, info.normal, info.object->material.ior), objects, lights, reflect_mask);
            info.transmission = ray.shaded();
        }
    }
    
    return info;
}
