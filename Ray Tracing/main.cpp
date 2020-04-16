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

#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"
#include "ray.hpp"
#include "camera.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
    Camera camera({0, 0, 0}, 120, 50);
    
    vector<Shape*> objects;
    objects.push_back(new Sphere({20, -2, 0}, 5, {{0, 0, 0.5}}));
    objects.push_back(new Cube({15, 6, -3}, 1, {{0, 0.5, 0}}));
    objects.push_back(new Sphere({16, 8, 3}, 1, {{0.5, 0, 0}}));
    
    vector<Light> lights;
    lights.push_back(Light({0, 15, 2}, 1000));
    lights.push_back(Light({10, -15, -2}, 100));
    
    camera.render(objects, lights);
    
    return 0;
}
