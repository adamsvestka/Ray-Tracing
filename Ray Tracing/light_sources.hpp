//
//  light_sources.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Light;
struct PointLight;
struct LinearLight;
struct GlobalLight;
struct DirectionalLight;

#pragma once

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"

// Obecný zdroj osvětlení ve scéně
class Light {
protected:
    Color color;    // Barva světla
    
public:
    bool shadow;    // Jestli světlo tvoří stíny
    /***/ virtual Vector3 getVector(Vector3 point) = 0; // Vektor ke světlu z daného bodu
    /***/ virtual Color getDiffuseValue(Vector3 point, Vector3 normal) = 0; // Difúzní charakter světla
    /***/ virtual Color getSpecularValue(Vector3 point, Vector3 normal, Vector3 direction, int n) = 0;  // Odleskový charakter světla
};


class PointLight : public Light {
private:
    Vector3 position;
    int intensity;
    
public:
    PointLight(Vector3, Color, int);
    Vector3 getVector(Vector3);
    Color getDiffuseValue(Vector3, Vector3);
    Color getSpecularValue(Vector3, Vector3, Vector3, int);
};


class LinearLight : public Light {
private:
    Vector3 position;
    int intensity;
    
public:
    LinearLight(Vector3, Color, int);
    Vector3 getVector(Vector3);
    Color getDiffuseValue(Vector3, Vector3);
    Color getSpecularValue(Vector3, Vector3, Vector3, int);
};


class GlobalLight : public Light {
private:
    float intensity;
    
public:
    GlobalLight(Color, float);
    Vector3 getVector(Vector3);
    Color getDiffuseValue(Vector3, Vector3);
    Color getSpecularValue(Vector3, Vector3, Vector3, int);
};


class DirectionalLight : public Light {
private:
    Vector3 direction;
    float intensity;
    
public:
    DirectionalLight(Vector3, Color, float);
    Vector3 getVector(Vector3);
    Color getDiffuseValue(Vector3, Vector3);
    Color getSpecularValue(Vector3, Vector3, Vector3, int);
};
