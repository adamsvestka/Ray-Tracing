//
//  data_types.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "data_types.hpp"

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
