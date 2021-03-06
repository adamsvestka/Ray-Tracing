//
//  camera.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Camera;

#pragma once

#include "settings.hpp"

#include "data_types.hpp"

class Camera {
private:
    Vector3 position;
    Matrix3x3 rotation;
    int width, height;
    float scale;
    
public:
    Camera();
    Camera(Vector3, Vector3, float, float, float);
    void getDimensions(int, int);
    Vector3 getPosition();
    Vector3 getRay(int, int);
};
