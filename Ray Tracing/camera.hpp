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
#include <thread>
#include <chrono>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

using namespace std;

struct RenderRegion {
    int x, y, w, h;
    vector<vector<Color>> buffer;
    
    RenderRegion() {
        x = y = w = h = 0;
    }
    
    RenderRegion(int minX, int maxX, int minY, int maxY) {
        x = minX;
        y = minY;
        w = maxX - minX;
        h = maxY - minY;
        buffer.resize(w, vector<Color>(h, Color::Black));
    }
};

class Camera {
private:
    Vector3 position;
    float fovFactor;
    int width, height, x, y;
    int region_count, region_current;
    chrono::steady_clock::time_point start, end;
    
    int r, l, i;
    int minX, maxX, minY, maxY;
    vector<vector<vector<Color>>> result;
    
    Display *display;
    Screen *screen;
    int scr;
    Window window;
    GC gc;
    
public:
    explicit Camera(Vector3);
    ~Camera();

    void drawDebugBox(int, int, Input);
    vector<vector<Intersection>> preRender(const vector<Shape *> &, const vector<Light *> &);
    vector<vector<Input>> processPreRender(const vector<vector<Intersection>> &);
    inline void drawInfoString(int, int, stringstream &, Color);
    void renderInfo();
    RenderRegion renderRegion(const vector<Shape *> &, const vector<Light *> &, RenderRegion, Input, const Intersection &);
    void render(const vector<Shape *> &, const vector<Light *> &);
    Vector3 getCameraRay(int, int);
    
    void generateRange();
    void resetPosition();
    bool next(const vector<vector<Input>> &);
};
