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
#include "renderer.hpp"
#include "interfaces.hpp"

using namespace std;

Settings settings;

int main(int argc, const char *argv[]) {
    NativeInterface *interface = new X11Interface();
    
    vector<Shape *> objects;
    vector<Light *> lights;
    
    Parser parser(interface);
    parser.parseSettings("settings.ini", settings);
    parser.parseScene("scene.json", objects, lights);
    
    Renderer renderer(interface, {0, 0, 0}, {0, 0, 0}, objects, lights);
    renderer.render();
    
    char c = '\0';
    while ((c = interface->getChar()) != 'q') {
        if (!settings.save_render) continue;
        
        int n = c - '0';
        if (n >= 0 && n < RenderTypes) settings.render_mode = n;
        else continue;
        
        renderer.redraw();
    }
    
    return 0;
}
