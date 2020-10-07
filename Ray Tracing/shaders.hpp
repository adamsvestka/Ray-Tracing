//
//  shaders.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 5/4/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Material;

class Image;
class Checkerboard;
class Bricks;
class PerlinNoise;

#pragma once

#include <iostream>
#include <array>
#include <random>
#include <algorithm>
#include <functional>

#include "data_types.hpp"

using namespace std;

typedef function<Color(VectorUV)> Shader;

// MARK: - Material
struct Material {
    Shader texture;
    float n, Ks, ior;
    bool transparent;
};


// MARK: - Textures
class Texture {
public:
    virtual Color operator()(VectorUV) const = 0;
};


class Image : public Texture {
private:
    Buffer image;
    
public:
    explicit Image(Buffer &);
    
    Color operator()(VectorUV) const;
};


class Checkerboard : public Texture {
private:
    int scale;
    Color primary, secondary;
    
public:
    Checkerboard(int, Color, Color);
    
    Color operator()(VectorUV) const;
};


class Bricks : public Texture {
private:
    Buffer colors;
    int scale;
    float ratio, mortar;
    Color primary, secondary, tertiary;
    
public:
    Bricks(int, float, float, Color, Color, Color, int);
    
    Color operator()(VectorUV) const;
};


class PerlinNoise : public Texture {
private:
    vector<vector<pair<float, float>>> points;
    int scale;
    Color primary;
    
public:
    PerlinNoise(int, int, Color);
    
    inline float smoothstep(float) const;
    inline float lerp(float, float, float) const;
    inline float dotGradient(int, int, float, float) const;
    
    Color operator()(VectorUV) const;
};
