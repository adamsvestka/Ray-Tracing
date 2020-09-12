//
//  camera.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "camera.hpp"

Camera::Camera() {
    position = Vector3::Zero;
    rotation = Matrix3x3::Identity;
    width = height = 0;
}

Camera::Camera(Vector3 position, Vector3 angles, float width, float height, float fov) {
    this->position = position;
    rotation = Matrix3x3::RotationMatrix(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    this->width = width;
    this->height = height;
    fovFactor = 1 / tan(fov / 2);
}

void Camera::getDimensions(int width, int height) {
    this->width = width;
    this->height = height;
}

Vector3 Camera::getPosition() {
    return position;
}

Vector3 Camera::getRay(int x, int y) {
    float u = (2 * (float)x / width - 1) * width / height;
    float v = 1 - (2 * (float)y / height);
    
    return rotation * Vector3{fovFactor, u, v}.normal();
}
