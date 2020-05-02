//
//  data_types.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Vector3;
struct Matrix3x3;
struct Color;
struct Material;

#pragma once

#include <cmath>
#include <vector>
#include <functional>

using namespace std;

struct Vector3 {
    float x, y, z;
    
    float length();
    Vector3 normal();
    
    Color toColor();
    Vector3 operator+(Vector3);
    Vector3 operator-(Vector3);
    Vector3 operator-();
    Vector3 operator*(double);
    Vector3 operator*(float);
    Vector3 operator*(int);
    Vector3 operator/(double);
    Vector3 operator/(float);
    Vector3 operator/(int);
    float operator*(Vector3);
    void operator+=(Vector3);
    void operator-=(Vector3);
};

#define Zero    Vector3{0, 0, 0}
#define Unit    Vector3{1, 1, 1}

struct Matrix3x3 {
    float n[3][3];
    
    Matrix3x3 inverse();
    
    float operator()(int, int);
    Matrix3x3 operator+(Matrix3x3);
    Matrix3x3 operator-(Matrix3x3);
    Matrix3x3 operator*(float);
    Matrix3x3 operator*(Matrix3x3);
    Vector3 operator*(Vector3);
};

#define Identity    Matrix3x3{1, 0, 0, 0, 1, 0, 0, 0, 1}
#define rot_mat_x(a)    Matrix3x3{1, 0, 0, 0, cos(a), -sin(a), 0, sin(a), cos(a)}
#define rot_mat_y(b)    Matrix3x3{cos(b), 0, sin(b), 0, 1, 0, -sin(b), 0, cos(b)}
#define rot_mat_z(c)    Matrix3x3{cos(c), -sin(c), 0, sin(c), cos(c), 0, 0, 0, 1}
#define rot_mat(yaw, pitch, roll)   (rot_mat_z(roll) * rot_mat_y(pitch) * rot_mat_x(yaw))

struct Color {
    float r, g, b;
    
    Color();
    Color(float, float, float);
    
    operator int();
    Color operator+(Color);
    Color operator-(Color);
    Color operator*(double);
    Color operator*(float);
    Color operator*(int);
    Color operator/(double);
    Color operator/(float);
    Color operator/(int);
    Color operator*(Color);
    void operator+=(Color);
    void operator-=(Color);
};

#define White       Color{1.0, 1.0, 1.0}
#define Gray        Color{0.1, 0.1, 0.1}
#define Black       Color{0.0, 0.0, 0.0}

#define DarkRed     Color{0.3, 0.03, 0.03}
#define Red         Color{1.0, 0.1, 0.1}
#define Orange      Color{1.0, 0.5, 0.1}
#define Yellow      Color{1.0, 1.0, 0.1}
#define Lime        Color{0.5, 1.0, 0.1}

#define DarkGreen     Color{0.03, 0.3, 0.03}
#define Green       Color{0.1, 1.0, 0.1}
#define Turquoise   Color{0.1, 0.7, 0.7}
#define Cyan        Color{0.1, 1.0, 1.0}

#define DarkBlue     Color{0.03, 0.03, 0.3}
#define Blue        Color{0.1, 0.1, 1.0}
#define Purple      Color{0.6, 0.1, 1.0}
#define Magenta     Color{1.0, 0.1, 1.0}
#define Pink        Color{1.0, 0.4, 0.7}

struct NeuralNetwork {
    vector<vector<vector<float>>> nodes;
    
    NeuralNetwork(vector<vector<vector<float>>>);
    
    vector<float> eval(vector<float>);
};

struct Material {
//    Color color;
    function<Color(float, float)> texture;
    float n, Ks, ior;
    bool transparent;
};
