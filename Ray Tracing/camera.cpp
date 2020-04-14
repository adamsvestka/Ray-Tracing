//
//  camera.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "camera.hpp"

Camera::Camera(int width, int height, Vector3 position, int fov, int region_size) {
    this->position = position;
    fovFactor = 1 / tan(fov / 2);
    this->width = width / 2;
    this->height = height;
//        x = y = 0;
    region = region_size;
    
    x = round(this->width / region / 2) * region;
    y = round(this->height / region / 2) * region;
    r = 0;
    l = 1;
    i = 0;
}

Ray Camera::getCameraRay(int x, int y) {
    float u = (4 * (float)x / width - 1) / 2 * width / height;
    float v = 1 - (2 * (float)y / height);
    
    Vector3 direction{fovFactor, u, v};
    Ray ray(position, direction.normal());
    
    return ray;
}

bool Camera::nextSpiral() {
    i++;
    switch (r) {
        case 0: x += region; break;
        case 1: y -= region; break;
        case 2: x -= region; break;
        case 3: y += region; break;
    }
    if (i >= l) {
        if (++r % 2 == 0) l++;
        if (r > 3) r = 0;
        i = 0;
    }
    
    return x < width || y < height;
}

bool Camera::next() {
    x += region;
    if (x >= width) {
        x = 0;
        y += region;
    }
    
    return y < height;
}

int Camera::minX() { return fmax(x, 0); };
int Camera::maxX() { return fmin(x + region, width); };
int Camera::minY() { return fmax(y, 0); };
int Camera::maxY() { return fmin(y + region, height); };
