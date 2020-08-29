//
//  ray.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "ray.hpp"

// MARK: Intersection
Color Intersection::shaded() const {
    if (!hit) return settings.background_color;
    else {
        Color lighting = Color::Black;
        for (int i = 0; i < shadows.size(); i++) if (!shadows[i]) lighting += texture * diffuse[i] * (1 - object->material.Ks) + specular[i] * object->material.Ks;
        return (ambient * (1 - object->material.Ks) + lighting) * !object->material.transparent + reflection * object->material.Ks * kr + transmission * object->material.transparent * (1 - kr);
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
    return (k < 0 ? Vector3::Zero : direction * eta + normal * (eta * cosi - sqrt(k))).normal();
}

// MARK: castRay
Intersection castRay(Vector3 origin, Vector3 direction, const vector<Shape *> &objects, const vector<Light *> &lights, Input mask) {
    Intersection info;
    
    info.hit = false;
    info.position = origin;
    info.distance = settings.max_render_distance;
    info.object = nullptr;
    info.id = -1;
    info.normal = Vector3::Zero;
    info.kr = 1;
    info.shadows = vector<bool>(lights.size(), false);
    info.diffuse = info.specular = valarray<Color>(Color::Black, objects.size());
    info.ambient = settings.ambient_lighting;
    info.light = info.texture = info.reflection = info.transmission = Color::Black;
    
    if (++mask.bounce_count > settings.max_light_bounces) return info;
    
    Hit hit;
    for (const auto &object : objects) {
        Hit temp = object->intersect(origin, direction);
        if (temp.distance > 0 && temp.distance < info.distance && (mask.lighting || !object->material.transparent)) {
            info.object = object;
            info.distance = temp.distance;
            hit = temp;
        }
    }
    
    info.position = origin + direction * info.distance;
    if ((info.hit = info.object != nullptr)) {
        info.id = (float)distance(objects.begin(), find(objects.begin(), objects.end(), info.object)) / objects.size();
        info.normal = hit.getNormal();
        info.texture = hit.getTexture();
        
        // Offset to avoid self-intersection
        if (info.normal * direction < 0 && info.object->material.transparent) info.position -= info.normal * settings.surface_bias;
        else info.position += info.normal * settings.surface_bias;
        
        // Return if only testing for clear line of sight
        if (!mask.lighting) return info;
    } else return info;
    
    // MARK: Diffuse, Specular
    if (mask.diffuse && !info.object->material.transparent) {
        auto shadow_mask = mask;
        shadow_mask.lighting = false;
        for (int i = 0; i < lights.size(); i++) {
            const auto &light = lights[i];
            
            if (info.object->material.Ks < 1) info.diffuse[i] = light->getDiffuseValue(info.position, info.normal);
            if (info.object->material.Ks > 0) info.specular[i] = light->getSpecularValue(info.position, info.normal, direction, info.object->material.n);
            
            // Check clear line of sight to light
            if (mask.shadows[i] && light->shadow && castRay(info.position, light->getVector(info.position).normal(), objects, lights, shadow_mask).hit) {
                info.shadows[i] = true;
                continue;
            }
            
            info.light += info.diffuse[i] * (1 - info.object->material.Ks) + info.specular[i] * info.object->material.Ks;
        }
    }
    
    // MARK: Reflection
    auto reflect_mask = mask;
    reflect_mask.diffuse = reflect_mask.reflections = reflect_mask.transmission = true;
    reflect_mask.shadows = vector<bool>(lights.size(), true);
    
    if (mask.reflections && (info.object->material.Ks > 0 || info.object->material.transparent)) {
        const auto ray = castRay(info.position, reflect(direction, info.normal), objects, lights, reflect_mask);
        info.reflection = ray.shaded();
    }
    
    // MARK: Transmission
    if (info.object->material.transparent) {    // Nested ifs to fill info.kr but not waste computation
        if ((info.kr = fresnel(direction, info.normal, info.object->material.ior)) < 1 && mask.transmission) {
            const auto ray = castRay(info.position, refract(direction, info.normal, info.object->material.ior), objects, lights, reflect_mask);
            info.transmission = ray.shaded();
        }
    }
    
    return info;
}
