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
#include "file_managers.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"
#include "ray.hpp"
#include "camera.hpp"

#define Col(col) [](float, float) { return col; }

using namespace std;

Settings settings;

// Material: {texture, n, Ks, ior, transparent}

int main(int argc, const char *argv[]) {
    settings.ambient_lighting = Color::Gray.dark();
    settings.background_color = Color::Gray;
    
    parseSettings("settings.ini", settings);
    
    Camera camera({0, 0, 0});
    
    vector<Shape *> objects;
    objects.push_back(new Cuboid({18.5, -6, -2}, 3, {45, 0, 0}, {[i = Image("arnold.jpg")](float u, float v) { return i(u, v); }}));
    objects.push_back(new Plane({50, -4, -4}, 80, 24, Vector3::Zero, {[c = Checkerboard(32), n = Noise(16, 0)](float u, float v) { return colorRamp(c(u, v), Color::Red, Color::Yellow) - Color::White * n(u, v); }}));
    objects.push_back(new Sphere({15, 0, 1}, 6, Vector3::Zero, {Col(Color::White), 0, 0, 0.95, true}));
    objects.push_back(new Sphere({10, -3.5, 1}, 2, Vector3::Zero, {Col(Color::Gray), 250, 0.8, 0, false}));
    objects.push_back(new Sphere({20, -5, -0.5}, 5, {90, 0, 0}, {[c = Checkerboard(32)](float u, float v) { return colorRamp(c(u, v), Color::Red, Color::Yellow); }, 250, 0.7}));
    objects.push_back(new Sphere({30, -7, 5}, 5, {0, 90, 0}, {[c = Checkerboard(32)](float u, float v) { return colorRamp(c(u, v), Color::Green, Color::Gray); }, 250, 0.7}));
    objects.push_back(new Sphere({30, 5, 0}, 4, {0, 0, 90}, {[c = Checkerboard(32)](float u, float v) { return colorRamp(c(u, v), Color::White, Color::Green); }, 250, 0.7}));
    objects.push_back(new Sphere({40, 0, 0.5}, 6, Vector3::Zero, {[c = Checkerboard(32)](float u, float v) { return colorRamp(c(u, v), Color::Purple, Color::Magenta); }, 250, 0.7}));
//    objects.push_back(new Cylinder({20, 0, -1.5}, 3, 3, {0, 25, 0}, {Col(Color::Azure), 0, 0, 1.3, 1}));
//    objects.push_back(new Cylinder({20, 0.5, 0}, 0.2, 5, {40, 25, 0}, {Col(Color::Gray / 2)}));
    objects.push_back(new Plane({25, 5, -3}, 10, 10, {25, 25, 0}, {[i = Image("broccoli.jpg")](float u, float v) { return i(u, v) / 1.3; }}));
    
    vector<Light *> lights;
    lights.push_back(new GlobalLight(Color::Lime, 0.05));
    lights.push_back(new LinearLight({10, -10, 25}, Color::White, 500));
//    lights.push_back(new DirectionalLight(Vector3::Down, Color::White, 0.5));
    
    camera.render(objects, lights);
    
    return 0;
}
