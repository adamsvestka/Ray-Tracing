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
#include <ncurses.h>

#include "settings.hpp"
#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"
#include "ray.hpp"
#include "camera.hpp"

using namespace std;

Settings settings;

int main(int argc, const char * argv[]) {
    Camera camera({0, 0, 0});
    
    vector<Shape*> objects;
    objects.push_back(new Sphere({15, 0, -1}, 1, {0, -30, 0}, {[](float u, float v) { return ((int)(u * 10) % 2 != (int)(v * 20) % 2) ? Purple / 2 : Green / 2; }, false, false, 1, false}));
    objects.push_back(new Cube({15, 0, -7}, 5, {0, -30, 20}, {[](float u, float v) { return ((int)(u * 10) % 2 != (int)(v * 10) % 2) ? Red : Yellow; }, 100, 0.5, 1, false}));
    objects.push_back(new Sphere({19, -4, 3}, 2, Zero, {[](float, float) { return Gray; }, 250, 0.8, 1, false}));
    objects.push_back(new Sphere({12, -0.9, -0.3}, 1, Zero, {[](float, float) { return Red; }, false, false, 1.3, true}));
    
    vector<Light> lights;
    lights.push_back(Light{{10, 5, 5}, White, 1000});  // FIXME: High brightness messes up hue
//    lights.push_back(Light{{10, -2, 6}, Cyan, 500});
    
    camera.render(objects, lights);
    
    return 0;
}
