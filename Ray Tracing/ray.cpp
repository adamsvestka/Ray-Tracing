//
//  ray.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "ray.hpp"


// MARK: Timer
const array<string, Timer::c> Timer::names{"průsečníků", "stínů", "odrazů", "průhlednosti"};
const array<Color, Timer::c> Timer::colors{Color::Green, Color::Red, Color::Blue, Color::Orange};

Timer::Timer() {
    start = chrono::high_resolution_clock::now();
    last = 0;
    for (auto &time : times) time = 0;
}

void Timer::operator()() {
    auto now = chrono::high_resolution_clock::now();
    times[last++] = chrono::duration<float, milli>(now - start).count();
    start = now;
}

void Timer::operator+=(const Timer t) {
    for (short i = 0; i < Timer::c; i++) times[i] += t.times[i];
}

// MARK: RayIntersection
// Výpočet stínované barvy v průniku
Color RayIntersection::shaded() {
    if (!hit) return settings.background_color;
    else {
        light = Color::Black;
        for (int i = 0; i < shadows.size(); i++) if (!shadows[i]) light += texture * diffuse[i] * (1 - object->material.Ks) + specular[i] * object->material.Ks;
        return light * !object->material.transparent + reflection * object->material.Ks * kr + transmission * object->material.transparent * (1 - kr);
    }
    
    //     .......   ..   ______________–––––––lambert––––––________–––––phong–––––_____________   ____________   __________________
    // I = Kd * Ka + Ke + sum(Vi * Li * (Kd * max(li . N, 0) + Ks * (max(hi . N, 0)) ^ n)) * !Kt + Ir * Ks * kr + It * Kt * (1 - kr);
    /*
   x Ka ... ambient (environment) lighting
   - Ke ... emission lighting
   / Vi ... clear line of sight to light?
   | Li ... light value
   | Kd ... object diffuse ratio
   | Ks ... object specular (reflectance) ratio
   | N  ... surface normal
   | li ... light direction
   | hi ... camera direction
   \ n  ... shininess
     Ir ... compound reflection
     kr ... fresnel ratio
     It ... compound transmission
     Kt ... object transparent?
     */
}

// MARK: reflect
// Odrazí vektor podle normály
Vector3 reflect(Vector3 direction, Vector3 normal) {
    return direction - normal * 2 * (direction * normal);
}

// MARK: fresnel
// Spočítá Fresnelův faktor (poměr míšení průhlednosti a odrazu u průhledných objektů)
float fresnel(Vector3 direction, Vector3 normal, float ior) {
    float cosi = clamp(direction * normal, -1.f, 1.f);
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
// Zlomí vektor podle indexu lomu a normály
Vector3 refract(Vector3 direction, Vector3 normal, float ior) {
    float cosi = clamp(direction * normal, -1.f, 1.f);
    float etai = 1, etat = ior;
    if (cosi < 0) cosi = -cosi;
    else {
        swap(etai, etat);
        normal = -normal;
    }
    float eta = etai / etat;
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return (k < 0 ? Vector3::Zero : direction * eta + normal * (eta * cosi - sqrt(k))).normalized();
}

// MARK: castRay
RayIntersection castRay(Vector3 origin, Vector3 direction, const vector<Object *> &objects, const vector<Light *> &lights, RayInput mask) {
    RayIntersection info;
    
    // MARK: Hit detection
    // Výchozí hodnoty pro nestřetnutí
    info.timer = Timer();
    info.hit = false;
    info.position = origin;
    info.distance = settings.max_render_distance;
    info.object = nullptr;
    info.id = -1;
    info.normal = Vector3::Zero;
    info.kr = 1;
    info.shadows = vector<bool>(lights.size(), false);
    info.diffuse = info.specular = valarray<Color>(Color::Black, objects.size());
    info.light = info.texture = info.reflection = info.transmission = Color::Black;
    
    if (++mask.bounce_count > settings.max_light_bounces) return info;  // Kontrola maximálního počtu odrazů
    
    // Smyčka přes všechny objekty ve scéně, aby se našel ten nejbližší k počátku paprsku
    ObjectHit hit;
    for (const auto &object : objects) {
        ObjectHit temp = object->intersect(origin, direction);
        if (temp.distance > 0 && temp.distance < info.distance && (mask.lighting || !object->material.transparent)) {   // Průhledné objekty netvoří stín
            info.object = object;
            info.distance = temp.distance;
            hit = temp;
        }
    }
    
    // Pro vybraný objekt se dopočítají dodatečné údaje
    info.position = origin + direction * info.distance;
    if ((info.hit = info.object != nullptr)) {
        // Pokud se jen testuje viditelnost, tak není třeba nic dál počítat
        if (!mask.lighting) return info;
        
        info.id = (float)distance(objects.begin(), find(objects.begin(), objects.end(), info.object)) / objects.size();
        info.normal = hit.getNormal();
        info.texture = hit.getTexture();
        
        // Posun, aby se zabránilo sebe-protnutí
        if (info.normal * direction < 0 && info.object->material.transparent) info.position -= info.normal * settings.surface_bias;
        else info.position += info.normal * settings.surface_bias;
    } else return info;
    
    // MARK: Diffuse, Specular
    info.timer();
    if (mask.diffuse && !info.object->material.transparent) {
        auto shadow_mask = mask;
        shadow_mask.lighting = false;
        for (int i = 0; i < lights.size(); i++) {
            const auto &light = lights[i];
            
            if (info.object->material.Ks < 1) info.diffuse[i] = light->getDiffuseValue(info.position, info.normal);
            if (info.object->material.Ks > 0) info.specular[i] = light->getSpecularValue(info.position, info.normal, direction, info.object->material.n);
            
            // Kontrola jestli bod není ve stínu
            const auto vector_to_light = light->getVector(info.position);
            if (mask.shadows[i] && light->shadow && castRay(info.position, vector_to_light.normalized(), objects, lights, shadow_mask).distance < vector_to_light.length()) {
                info.shadows[i] = true;
                continue;
            }
            
            info.light += info.diffuse[i] * (1 - info.object->material.Ks) + info.specular[i] * info.object->material.Ks;
        }
    }
    
    // MARK: Reflection
    info.timer();
    auto reflect_mask = mask;
    reflect_mask.diffuse = reflect_mask.reflections = reflect_mask.transmission = true;
    reflect_mask.shadows = vector<bool>(lights.size(), true);
    
    if (mask.reflections && (info.object->material.Ks > 0 || info.object->material.transparent)) {
        auto ray = castRay(info.position, reflect(direction, info.normal), objects, lights, reflect_mask);
        info.reflection = ray.shaded();
    }
    
    // MARK: Transmission
    info.timer();
    if (info.object->material.transparent) {    // Vnořené pokud aby se spočítalo kr, ale neplýtvalo zbytkem
        if ((info.kr = fresnel(direction, info.normal, info.object->material.ior)) < 1 && mask.transmission) {
            auto ray = castRay(info.position, refract(direction, info.normal, info.object->material.ior), objects, lights, reflect_mask);
            info.transmission = ray.shaded();
        }
    }
    
    info.timer();
    return info;
}
