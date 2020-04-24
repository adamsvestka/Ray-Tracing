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
    objects.push_back(new Sphere({15, 0, -1}, 1, {Magenta, 0.7, 100, 0.003, false}));
    objects.push_back(new Cube({15, 0, -7}, 5, {0, -30, 20}, {Green, 0.6, false, false, 0.6}));
    objects.push_back(new Sphere({19, -4, 3}, 2, {White, 0.6, false, false, 0.9}));
    
    vector<Light> lights;
    lights.push_back(Light({10, 5, 5}, White, 1000));  // FIXME: High brightness messes up hue
//    lights.push_back(Light({20, 2, -6}, Blue, 1000));
    
    camera.render(objects, lights);
    
    return 0;
}
