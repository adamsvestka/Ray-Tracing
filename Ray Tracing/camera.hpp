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

#include <ncurses.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "data_types.hpp"
#include "ray.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

//#define using_ncurses
#define render_spiral
#define resolution 1

class Camera {
private:
    Vector3 position;
    float fovFactor;
    int width, height, x, y, region;
    int r, l, i;
#ifndef using_ncurses
    Display *display;
    Screen *screen;
    int scr;
    Window window;
    GC gc;
#endif
    
public:
    Camera(Vector3, int, int);
    ~Camera();
    
    Ray getCameraRay(int, int);
    
    bool nextSpiral();
    bool next();
    int minX();
    int maxX();
    int minY();
    int maxY();
    
    void render(std::vector<Shape*>, std::vector<Light>);
};
