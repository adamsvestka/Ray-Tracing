//
//  shaders.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 5/4/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "shaders.hpp"

// MARK: - Interpolation
float smoothstep(float x) {
  return x * x * (3 - 2 * x);
}

Color colorRamp(float n, Color a, Color b) {
    return a * (1 - n) + b * n;
}


// MARK: - Textures

Checkerboard::Checkerboard(int scaleX, int scaleY) : scaleX(scaleX), scaleY(scaleY) {};

float Checkerboard::operator()(float x, float y) const {
    return (int)(x * scaleX) % 2 != (int)(y * scaleY) % 2;
}


Noise::Noise(int seed, int scale) {
   this->scale = scale;
   points.resize(scale, vector<pair<float, float>>(scale));
   
   default_random_engine engine(seed);
   
   uniform_real_distribution<float> dist(0, M_PI * 2);
   for (auto &i : points) {
       for (auto &j : i) {
           auto a = dist(engine);
           j.first = cos(a);
           j.second = sin(a);
       }
   }
}

float Noise::lerp(float a, float b, float w) const {
    return a + w * (b - a);
}

float Noise::dotGradient(int ix, int iy, float x, float y) const {
    float dx = x - ix;
    float dy = y - iy;
    
    return (dx * points[iy][ix].first + dy * points[iy][ix].second);
}

float Noise::operator()(float x, float y) const {
    x *= scale - 1; y *= scale - 1;
    int x0 = (int)x, x1 = x0 + 1;
    int y0 = (int)y, y1 = y0 + 1;
    float dx = smoothstep(x - x0);
    float dy = smoothstep(y - y0);
    
    float ix0 = lerp(dotGradient(x0, y0, x, y), dotGradient(x1, y0, x, y), dx);
    float ix1 = lerp(dotGradient(x0, y1, x, y), dotGradient(x1, y1, x, y), dx);
    
    return lerp(ix0, ix1, dy) / 2 + 0.5;
}
