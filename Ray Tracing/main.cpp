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

using namespace std;

Settings settings;

int main(int argc, const char *argv[]) {
    settings.ambient_lighting = Color::Gray.dark();
    settings.background_color = Color::Gray;
    
    parseSettings("settings.ini", settings);
        
    vector<Shape *> objects;
    vector<Light *> lights;
    parseScene("scene.json", objects, lights);
    
    Camera camera({0, 0, 0});
    camera.render(objects, lights);
    
    return 0;
}
