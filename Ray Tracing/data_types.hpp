//
//  data_types.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Vector3;
struct Color;
struct Material;

#pragma once

#include <cmath>

struct Vector3 {
    float x, y, z;
    
    float length();
    Vector3 normal();
    
    Color toColor();
    Vector3 operator+(Vector3);
    Vector3 operator-(Vector3);
    Vector3 operator-();
    Vector3 operator*(float);
    Vector3 operator/(float);
    float operator*(Vector3);
};

#define Zero    Vector3{0, 0, 0}
#define Unit    Vector3{1, 1, 1}

struct Color {
    float r, g, b;
    
    Color();
    Color(float, float, float);
        
    operator int();
    Color operator+(Color);
    Color operator-(Color);
    Color operator*(float);
    Color operator/(float);
    Color operator*(Color);
    void operator+=(Color);
    void operator-=(Color);
};

#define White       Color{1.0, 1.0, 1.0}
#define Gray        Color{0.1, 0.1, 0.1}
#define Black       Color{0.0, 0.0, 0.0}

#define Red         Color{1.0, 0.1, 0.1}
#define Orange      Color{1.0, 0.5, 0.1}
#define Yellow      Color{1.0, 1.0, 0.1}
#define Lime        Color{0.5, 1.0, 0.1}

#define Green       Color{0.1, 1.0, 0.1}
#define Turquoise   Color{0.1, 0.7, 0.7}
#define Cyan        Color{0.1, 1.0, 1.0}

#define Blue        Color{0.1, 0.1, 1.0}
#define Purple      Color{0.6, 0.1, 1.0}
#define Magenta     Color{1.0, 0.1, 1.0}
#define Pink        Color{1.0, 0.4, 0.7}

struct Material {
    Color color;
    float albedo;
    float n, Ks;
    float reflection;
//    float refraction;
};
