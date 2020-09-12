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
    NativeInterface interface;
    
    Camera camera;
    vector<Shape *> objects;
    vector<Light *> lights;
    
    Parser parser(interface);
    parser.parseSettings("settings.ini", settings);
    parser.parseScene("scene.json", camera, objects, lights);
    
    Renderer renderer(interface, camera, objects, lights);
    renderer.render();
    
    if (!settings.save_render) { while (interface.getChar() != 'q') continue; return 0; }
    
    auto buffer = renderer.getResult(settings.render_mode);
    
    char c = '\0';
    while ((c = interface.getChar()) != 'q') {
        int n = c - '0';
        if (n >= 0 && n < RenderTypes) {
            buffer = renderer.getResult(n);
            for (int x = 0; x < buffer.size(); x++) {
                for (int y = 0; y < buffer[x].size(); y++) {
                    interface.drawPixel(x, y, buffer[x][y]);
                }
            }
            renderer.renderInfo();
        } else if (c == 's') interface.saveImage("output.png", buffer);
        else if (c == 'r') {
            objects.clear();
            lights.clear();
            parser.parseSettings("settings.ini", settings);
            parser.parseScene("scene.json", camera, objects, lights);
            renderer.render();
        }
    }
    
    interface.log("Exiting...");
    
    return 0;
}
