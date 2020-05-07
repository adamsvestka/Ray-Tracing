//
//  light_sources.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "light_sources.hpp"


// MARK: - Point
/// @param position Vector3{x, y, z}
/// @param color Color{r, g, b} ~ (0 - 1)
/// @param intensity int ~ 1000
PointLight::PointLight(Vector3 position, Color color, int intensity) : position(position), intensity(intensity) {
    this->color = color;
    shadow = true;
}

Vector3 PointLight::getVector(Vector3 point) {
    return position - point;
}

Color PointLight::getDiffuseValue(Vector3 point, Vector3 normal) {
    Vector3 vector_to_light = getVector(point);
    
    const float cosine_term = vector_to_light.normal() * normal;
    
    return color * intensity * (float)(fmax(cosine_term, 0.f) / (vector_to_light * vector_to_light * 4 * M_PI));
}

Color PointLight::getSpecularValue(Vector3 point, Vector3 normal, Vector3 direction, int n) {
    Vector3 vector_to_light = getVector(point);
    
    const float cosine_term = vector_to_light.normal() * normal;
    
    return color * pow(intensity, 0.3) * (pow(fmax(-(normal * (cosine_term * 2) - vector_to_light.normal()) * direction, 0.f), n));
}


// MARK: - Linear
/// @param position Vector3{x, y, z}
/// @param color Color{r, g, b} ~ (0 - 1)
/// @param intensity int ~ 300
LinearLight::LinearLight(Vector3 position, Color color, int intensity) : position(position), intensity(intensity) {
    this->color = color;
    shadow = true;
}

Vector3 LinearLight::getVector(Vector3 point) {
    return position - point;
}

Color LinearLight::getDiffuseValue(Vector3 point, Vector3 normal) {
    Vector3 vector_to_light = getVector(point);
    
    const float cosine_term = vector_to_light.normal() * normal;
    
    return color * intensity * (float)(fmax(cosine_term, 0.f) / (vector_to_light.length() * 4 * M_PI));
}

Color LinearLight::getSpecularValue(Vector3 point, Vector3 normal, Vector3 direction, int n) {
    Vector3 vector_to_light = getVector(point);
    
    const float cosine_term = vector_to_light.normal() * normal;
    
    return color * pow(intensity, 0.3) * (pow(fmax(-(normal * (cosine_term * 2) - vector_to_light.normal()) * direction, 0.f), n));
}



// MARK: - Global
/// @param color Color{r, g, b} ~ (0 - 1)
/// @param intensity int ~ (0 - 1)
GlobalLight::GlobalLight(Color color, float intensity) {
    this->color = color;
    this->intensity = intensity;
    shadow = false;
}

Vector3 GlobalLight::getVector(Vector3) {
    return Unit;
}

Color GlobalLight::getDiffuseValue(Vector3, Vector3) {
    return color * intensity;
}

Color GlobalLight::getSpecularValue(Vector3, Vector3, Vector3, int) {
    return Black;
}


// MARK: - Beam
/// @param position Vector3{x, y, z}
/// @param color Color{r, g, b} ~ (0 - 1)
/// @param intensity int ~ 1000
//BeamLight::BeamLight(Vector3 position, Vector3 orientation, Color color, int intensity) {
//    this->position = position;
//    this->color = color;
//    this->intensity = intensity;
//}

//Vector3 BeamLight::getVector(Vector3) { 
//    <#code#>;
//}
//
//Color BeamLight::getDiffuseValue() { 
//    <#code#>;
//}
//
//Color BeamLight::getSpecularValue() { 
//    <#code#>;
//}
