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

//#define using_ncurses
#ifdef using_ncurses
#include <ncurses.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#endif

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"


class Camera {
private:
    Vector3 position;
    float fovFactor;
    int width, height, x, y;
    int region_size, region_count, region_current;
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
    
    void render(std::vector<Shape*>, std::vector<Light>);
    Ray getCameraRay(int, int);
    
    bool next();
    int minX();
    int maxX();
    int minY();
    int maxY();
};
