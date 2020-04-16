//
//  data_types.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "data_types.hpp"


// MARK:- Vector3
float Vector3::length() {
    return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

Vector3 Vector3::normal() {
    return this->operator/(length());
}

Vector3 Vector3::operator+(Vector3 v) {
    return Vector3{this->x + v.x, this->y + v.y, this->z + v.z};
}

Vector3 Vector3::operator-(Vector3 v) {
    return Vector3{this->x - v.x, this->y - v.y, this->z - v.z};
}

Vector3 Vector3::operator*(float n) {
    return Vector3{this->x * n, this->y * n, this->z * n};
}

Vector3 Vector3::operator/(float n) {
    return Vector3{this->x / n, this->y / n, this->z / n};
}

float Vector3::operator*(Vector3 v) {
    return this->x * v.x + this->y * v.y + this->z * v.z;
}


// MARK:- Color
Color::Color() {
    r = g = b = 0;
}

Color::Color(float r, float g, float b) {
    this->r = fmin(fmax(r, 0.0), 1.0);
    this->g = fmin(fmax(g, 0.0), 1.0);
    this->b = fmin(fmax(b, 0.0), 1.0);
}

int Color::hex() {
    return ((int)(r * 255) << 16) + ((int)(g * 255) << 8) + (int)(b * 255);
}

Color Color::operator+(Color c) {
//    float c = 1 - r;
//    float m = 1 - g;
//    float y = 1 - b;
//    
//    r + r2 - 1;
    
    return Color(this->r + c.r, this->g + c.g, this->b + c.b);
}

Color Color::operator-(Color c) {
    return Color(this->r - c.r, this->g - c.g, this->b - c.b);
}

Color Color::operator*(float n) {
    return Color(this->r * n, this->g * n, this->b * n);
}

Color Color::operator/(float n) {
    return Color(this->r / n, this->g / n, this->b / n);
}
