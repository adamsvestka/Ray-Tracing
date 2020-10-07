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
    scale = 1;
}

Camera::Camera(Vector3 position, Vector3 angles, float width, float height, float fov) {
    this->position = position;
    rotation = Matrix3x3::RotationMatrix(angles.x * (float)M_PI / 180, angles.y * (float)M_PI / 180, angles.z * (float)M_PI / 180);
    this->width = width;
    this->height = height;
    scale = tan((fov * 0.5) / 180 * M_PI);
}

void Camera::getDimensions(int width, int height) {
    this->width = width;
    this->height = height;
}

Vector3 Camera::getPosition() {
    return position;
}

Vector3 Camera::getRay(int x, int y) {
    float u = (2 * (x + 0.5) / width - 1) * width / height * scale;
    float v = (1 - 2 * (y + 0.5) / height) * scale;
    
    return rotation * Vector3{1, u, v}.normalized();
}
