//
//  shaders.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 5/4/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "shaders.hpp"

// MARK: - Functions
float smoothstep(float x) {
    return x * x * (3 - 2 * x);
}

Color colorRamp(float n, Color a, Color b) {
    return a * (1 - n) + b * n;
}


// MARK: - Textures
///
Image::Image(string path) {
    cout << "Opening " << path << endl;
    try {
        image.load(path.c_str());
    } catch(...) {
        image = CImg<unsigned char>(64, 64, 1, 3);
        
        cimg_forXYC(image, x, y, c) { image(x, y, c) = (x / 8 % 2) != (y / 8 % 2) ? Color::Black.array().data()[c] : Color::Magenta.array().data()[c]; }
        image.draw_text(16, 8, "Image", Color::White.array().data(), 0, 1, 13);
        image.draw_text(24, 24, "not", Color::White.array().data(), 0, 1, 13);
        image.draw_text(16, 40, "found", Color::White.array().data(), 0, 1, 13);
        
        cout << "Couldn't open file" << endl;
    }
}

Color Image::operator()(float x, float y) const {
    return Color{image(y * image.width(), (1 - x) * image.height(), 0, 0) / 255.f,
        image(y * image.width(), (1 - x) * image.height(), 0, 1) / 255.f,
        image(y * image.width(), (1 - x) * image.height(), 0, 2) / 255.f
    };
}


///
Checkerboard::Checkerboard(int scale, Color primary, Color secondary) : scale(scale), primary(primary), secondary(secondary) {}

Color Checkerboard::operator()(float x, float y) const {
    return (int)(x * scale) % 2 != (int)(y * scale) % 2 ? primary : secondary;
}


///
Brick::Brick(int scale, float ratio, float mortar, Color primary, Color secondary, Color tertiary, int seed) : scale(scale), ratio(ratio), mortar(mortar), primary(primary), secondary(secondary), tertiary(tertiary) {
    colors.resize(scale, vector<Color>(ceil(scale / ratio) + 1));
    
    default_random_engine engine(seed);

    uniform_real_distribution<float> dist(0, 1);
    for (auto &i : colors) {
        for (auto &j : i) {
            auto a = dist(engine);
            j = primary * a + secondary * (1.f - a);
         }
     }
}

Color Brick::operator()(float x, float y) const {
    float dx = x * scale;
    float dy = y * scale / ratio + !((int)(x * scale) % 2) / 2.f;
    float ix = fmod(dx, 1);
    float iy = fmod(dy, 1);
    return iy * ratio < mortar || ix < mortar ? tertiary : colors[floor(dx)][floor(dy)];
}


///
Noise::Noise(int scale, int seed, Color primary) : scale(scale), primary(primary) {
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

Color Noise::operator()(float x, float y) const {
    x *= scale - 1; y *= scale - 1;
    int x0 = (int)x, x1 = x0 + 1;
    int y0 = (int)y, y1 = y0 + 1;
    float dx = smoothstep(x - x0);
    float dy = smoothstep(y - y0);
    
    float ix0 = lerp(dotGradient(x0, y0, x, y), dotGradient(x1, y0, x, y), dx);
    float ix1 = lerp(dotGradient(x0, y1, x, y), dotGradient(x1, y1, x, y), dx);
    
    return primary * (lerp(ix0, ix1, dy) / 2 + 0.5);
}
