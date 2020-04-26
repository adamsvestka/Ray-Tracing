//
//  camera.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

class Camera;

#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

using namespace std;

class Camera {
private:
    Vector3 position;
    float fovFactor;
    int width, height, x, y;
    int region_count, region_current;
    clock_t time;
    
    int r, l, i;
    int minX, maxX, minY, maxY;
    
    Display *display;
    Screen *screen;
    int scr;
    Window window;
    GC gc;
    
public:
    Camera(Vector3);
    ~Camera();
    
    void drawPixel(int, int, int, Intersection);
    vector<vector<Intersection>> preRender(vector<Shape *>, vector<Light>);
    vector<vector<short>> processPreRender(vector<vector<Intersection>>);
    void renderInfo();
    void renderRegion(vector<Shape *>, vector<Light>, vector<vector<short>>);
    void render(vector<Shape *>, vector<Light>);
    Vector3 getCameraRay(int, int);
    
    void generateRange();
    bool next();
};
