//
//  renderer.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct RenderRegion;
class Renderer;

#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"
#include "objects.hpp"
#include "light_sources.hpp"
#include "camera.hpp"
#include "interfaces.hpp"

using namespace std;

// Strktura uchovávající bitmapu jednoho vykresleného regionu
struct RenderRegion {
    int x, y, w, h;
    Buffer buffer;
    Timer timer;
    
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

// Třída starající se o vysílání paprsků a celkové vykreslování
class Renderer {
private:
    NativeInterface &display;
    Camera &camera;
    vector<Object *> &objects;
    vector<Light *> &lights;
    
    int width, height, x, y;
    int region_count, region_current;
    chrono::steady_clock::time_point start, end;
    Timer timer;
    ObjectInfo info;
    
    int r, l, i;
    int minX, maxX, minY, maxY;
    vector<Buffer> result;
    
    
    vector<vector<RayIntersection>> preRender();
    vector<vector<RayInput>> processPreRender(const vector<vector<RayIntersection>> &);
    
    RenderRegion renderRegion(RenderRegion, const RayInput &, const RayIntersection &);
    
    void generateRange();
    void resetPosition();
    bool next(const vector<vector<RayInput>> &);
    
public:
    Renderer(NativeInterface &, Camera &, vector<Object *> &, vector<Light *> &);
    
    void renderInfo();
    void render();
    Buffer getResult(short);
};
