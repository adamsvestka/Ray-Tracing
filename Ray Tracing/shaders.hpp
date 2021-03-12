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

// Vlastní datový typ pro textury objektů / na generování barvy z UV souřadnice
typedef function<Color(VectorUV)> Shader;

// MARK: - Material
// Struktura materiálu objektu
struct Material {
    Shader texture; // Funkce, která vrátí barvu pro danou UV souřadnici
    float n, Ks, ior;   // Parametry stínování: ostrost odrazu, odrazivost, index lomu světla
    bool transparent;   // Průhlednost
};


// MARK: - Textures
// Šablona pro texturu objektu
class Texture {
public:
    virtual Color operator()(VectorUV) const = 0;
};


// Obrázek načtený ze souboru jako textura
class Image : public Texture {
private:
    Buffer image;
    
public:
    explicit Image(Buffer &);
    
    Color operator()(VectorUV) const;
};


// Dynamicky vygenerovaná šachovnice dvou barev jako textura
class Checkerboard : public Texture {
private:
    int scale;
    Color primary, secondary;
    
public:
    Checkerboard(int, Color, Color);
    
    Color operator()(VectorUV) const;
};


// Dynamicky vygenerovaný cihlový vzor jako textura; na vstupu jsou 3 barvy: cihly náhodně interpolují mezi dvěmi, malta má třetí
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


// Dynamicky vygenerovaný Perlinův šum vstupní barvy jako textura
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
