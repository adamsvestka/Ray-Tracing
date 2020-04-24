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
    return Color(x > 0 ? x / len : -x / len, y > 0 ? y / len : -y / len, z > 0 ? z / len : -z / len);
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

void Vector3::operator+=(Vector3 v) {
    x = x + v.x; y = y + v.y; z = z + v.z;
}

void Vector3::operator-=(Vector3 v) {
    x = x - v.x; y = y - v.y; z = z - v.z;
}


// MARK: - Matrix3x3
Matrix3x3 Matrix3x3::inverse() {
    Matrix3x3 minors;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float m[2][2];
            for (int k = 0; k < 3; k++) {
                for (int l = 0; l < 3; l++) {
                    if (k == i || l == j) continue;
                    m[k > i ? k - 1 : k][l > j ? l - 1 : l] = n[k][l];
                }
            }
            if ((i + j) % 2) minors.n[j][i] = - m[0][0] * m[1][1] + m[0][1] * m[1][0];
            else minors.n[j][i] = m[0][0] * m[1][1] - m[0][1] * m[1][0];
        }
    }
    float det = 0;
    for (int i = 0; i < 3; i++) det += n[0][i] * minors(i, 0);
    
    return minors * (1.f / det);
}

float Matrix3x3::operator()(int i, int j) {
    return n[i][j];
}

#define add(i, j) this->n[i][j] + m(i, j)
#define sub(i, j) this->n[i][j] - m(i, j)
#define loop1(f, n) f(n, 0), f(n, 1), f(n, 2)
#define loop2(f) loop1(f, 0), loop1(f, 1), loop1(f, 2)
Matrix3x3 Matrix3x3::operator+(Matrix3x3 m) {
    return Matrix3x3{loop2(add)};
//    return Matrix3x3{this->n[0][0] + n[0][0], this->n[0][1] + n[0][1], this->n[0][2] + n[0][2], this->n[1][0] + n[1][0], this->n[1][1] + n[1][1], this->n[1][2] + n[1][2], this->n[2][0] + n[2][0], this->n[2][1] + n[2][1], this->n[2][2] + n[2][2]};
}

Matrix3x3 Matrix3x3::operator-(Matrix3x3 m) {
    return Matrix3x3{loop2(sub)};
//    return Matrix3x3{this->n[0][0] - n[0][0], this->n[0][1] - n[0][1], this->n[0][2] - n[0][2], this->n[1][0] - n[1][0], this->n[1][1] - n[1][1], this->n[1][2] - n[1][2], this->n[2][0] - n[2][0], this->n[2][1] - n[2][1], this->n[2][2] - n[2][2]};
}

#define num(i, j) this->n[i][j] * n
#define num1(n) num(n, 0), num(n, 1), num(n, 2)
#define num2 num1(0), num1(1), num1(2)
Matrix3x3 Matrix3x3::operator*(float n) {
    return Matrix3x3{num2};
//    return Matrix3x3this->n[0][0] * n, this->n[0][1] * n, this->n[0][2] * n, this->n[1][0] * n, this->n[1][1] * n, this->n[1][2] * n, this->n[2][0] * n, this->n[2][1] * n, this->n[2][2] * n};
}

#define mult(i, j, k) this->n[k][j] * m(i, k)
#define mult1(i, j) mult(i, j, 0) + mult(i, j, 1) + mult(i, j, 2)
#define mult2(i) mult1(i, 0), mult1(i, 1), mult1(i, 2)
#define mult3 mult2(0), mult2(1), mult2(2)
Matrix3x3 Matrix3x3::operator*(Matrix3x3 m) {
    return Matrix3x3{mult3};
//    return Matrix3x3{this->n[0][0] * m(0, 0) + this->n[1][0] * m(0, 1) + this->n[2][0] * m(0, 2), this->n[0][1] * m(0, 0) + this->n[1][1] * m(0, 1) + this->n[2][1] * m(0, 2), this->n[0][2] * m(0, 0) + this->n[1][2] * m(0, 1) + this->n[2][2] * m(0, 2), this->n[0][0] * m(1, 0) + this->n[1][0] * m(1, 1) + this->n[2][0] * m(1, 2), this->n[0][1] * m(1, 0) + this->n[1][1] * m(1, 1) + this->n[2][1] * m(1, 2), this->n[0][2] * m(1, 0) + this->n[1][2] * m(1, 1) + this->n[2][2] * m(1, 2), this->n[0][0] * m(2, 0) + this->n[1][0] * m(2, 1) + this->n[2][0] * m(2, 2), this->n[0][1] * m(2, 0) + this->n[1][1] * m(2, 1) + this->n[2][1] * m(2, 2), this->n[0][2] * m(2, 0) + this->n[1][2] * m(2, 1) + this->n[2][2] * m(2, 2)};
}

#define vec(i) this->n[0][i] * v.x + this->n[1][i] * v.y + this->n[2][i] * v.z
#define vec1 vec(0), vec(1), vec(2)
Vector3 Matrix3x3::operator*(Vector3 v) {
    return Vector3{vec1};
//    return Vector3{this->n[0][0] * v.x + this->n[1][0] * v.y + this->n[2][0] * v.z, this->n[0][1] * v.x + this->n[1][1] * v.y + this->n[2][1] * v.z, this->n[0][2] * v.x + this->n[1][2] * v.y + this->n[2][2] * v.z};
}


// MARK: - Color
Color::Color() {
    r = g = b = 0;
}

Color::Color(float r, float g, float b) {
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


// MARK: - NeuralNetwork
NeuralNetwork::NeuralNetwork(vector<vector<float>> nodes) {
    this->nodes = nodes;
}

float NeuralNetwork::eval(vector<vector<float>> input) {
    float d = 0;
    for (int i = 0; i < nodes.size(); i++) {
        for (int j = 0; j < nodes.size(); j++) {
            d += nodes[i][j] * input[i][j];
        }
    }
    
    return d;
}
