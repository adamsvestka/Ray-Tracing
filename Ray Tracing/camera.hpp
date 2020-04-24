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

using namespace std;

class Camera {
private:
    Vector3 position;
    float fovFactor;
    int width, height, x, y;
    int region_count, region_current;
    int r, l, i;
    clock_t time;
#ifndef using_ncurses
    Display *display;
    Screen *screen;
    int scr;
    Window window;
    GC gc;
#endif
    
public:
    Camera(Vector3);
    ~Camera();
    
    void drawPixel(int, int, int, Intersection);
    vector<vector<float>> preRender(vector<Shape *>, vector<Light>);
    vector<vector<bool>> processPreRender(vector<vector<float>>);
    void renderRegions(vector<Shape *>, vector<Light>, vector<vector<bool>>);
    void renderInfo();
    void render(vector<Shape *>, vector<Light>);
//    Ray getCameraRay(int, int);
    Vector3 getCameraRay(int, int);
    
    bool next();
    int minX();
    int maxX();
    int minY();
    int maxY();
};
