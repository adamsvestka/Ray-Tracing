//
//  data_types.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "data_types.hpp"


// MARK: - Vector3
float Vector3::length() {
    return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

Color Vector3::toColor() {
    const float len = length();
    return Color(x / len, y / len, z / len);
//    return Color(x > 0 ? x / len : -x / len, y > 0 ? y / len : -y / len, z > 0 ? z / len : -z / len);
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

Vector3 Vector3::operator-() {
    return Vector3{-this->x, -this->y, -this->z};
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


// MARK: - Color
Color::Color() {
    r = g = b = 0;
}

Color::Color(float r, float g, float b) {
//    this->r = fmin(fmax(r, 0.0), 1.0);
//    this->g = fmin(fmax(g, 0.0), 1.0);
//    this->b = fmin(fmax(b, 0.0), 1.0);
    this->r = r;
    this->g = g;
    this->b = b;
}

Color::operator int() {
    return ((int)(fmin(fmax(r, 0.0), 1.0) * 255) << 16) + ((int)(fmin(fmax(g, 0.0), 1.0) * 255) << 8) + (int)(fmin(fmax(b, 0.0), 1.0) * 255);
}

Color Color::operator+(Color c) {
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

Color Color::operator*(Color c) {
    return Color(this->r * c.r, this->g * c.g, this->b * c.b);
}

void Color::operator+=(Color c) {
    *this = *this + c;
}

void Color::operator-=(Color c) {
    *this = *this - c;
}
