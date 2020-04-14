//
//  camera.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Camera;

#pragma once

#include "data_types.hpp"
#include "ray.hpp"

class Camera {
private:
    Vector3 position;
    float fovFactor;
    int width, height, x, y, region;
    int r, l, i;
    
public:
    Camera(int, int, Vector3, int, int);
    
    Ray getCameraRay(int, int);
    
    bool nextSpiral();
    bool next();
    int minX();
    int maxX();
    int minY();
    int maxY();
};
