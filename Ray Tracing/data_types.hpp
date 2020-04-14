//
//  data_types.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Vector3;

#pragma once

#include <cmath>

struct Vector3 {
    float x, y, z;
    
    float length();
    Vector3 normal();
    
    Vector3 operator+(Vector3);
    Vector3 operator-(Vector3);
    Vector3 operator*(float);
    Vector3 operator/(float);
    float operator*(Vector3);
};
