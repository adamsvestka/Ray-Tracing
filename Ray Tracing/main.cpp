//
//  main.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/13/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include <iostream>
#include <vector>
#include <cmath>

#include "settings.hpp"
#include "data_types.hpp"
#include "shaders.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"
#include "ray.hpp"
#include "camera.hpp"

#define Col(col) [](float, float) { return col; }

using namespace std;

Settings settings;

// Material: {texture, n, Ks, ior, transparent}

int main(int argc, const char * argv[]) {
    Camera camera({0, 0, 0});
    
    vector<Shape *> objects;
    objects.push_back(new Plane({50, -4, -4}, 40, 12, Zero, {[c = Checkerboard(32)](float u, float v) { return colorRamp(c(u, v), Red, Yellow); }}));
    objects.push_back(new Sphere({20, -5, -0.5}, 2.5, Zero, {Col(White), 250, 0.7}));
    objects.push_back(new Sphere({15, 0, 1}, 3, Zero, {Col(White), 0, 0, 0.95, true}));
    
    vector<Light *> lights;
    lights.push_back(new GlobalLight(White, 0.5));
    lights.push_back(new LinearLight({20, -4, 25}, White, 300));
    
    camera.render(objects, lights);
    
    return 0;
}
