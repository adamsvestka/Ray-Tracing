//
//  ray.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Timer;
struct RayInput;
struct RayIntersection;

#pragma once

#include <vector>
#include <valarray>

#include "settings.hpp"

#include "data_types.hpp"
#include "objects.hpp"
#include "light_sources.hpp"

using namespace std;

// Struktura na časování
struct Timer {
    static const short c = 4;
    static const array<string, c> names;
    static const array<Color, c> colors;
    
    chrono::steady_clock::time_point start;
    short last;
    array<float, c> times;
    
    Timer();
    
    void operator()();
    void operator+=(const Timer);
};

// Struktura, která se chová jako maska - diktuje, co se bude vykreslovat pro daný region
struct RayInput {
    bool render;
    short bounce_count;
    
    bool lighting, diffuse, reflections, transmission;
    vector<bool> shadows;
};

// Struktura držící informace o průniku paprsku s objektem
struct RayIntersection {
    bool hit;               // Byl střet?
    Vector3 position;       // Poloha střetu
    float distance;         // Vzdálenost střetu od počátku
    const Object *object;   // Ukazatel na protnutý objekt
    float id;               // Jedinečné číslo pro daný objekt
    
    Vector3 normal;                                             // Povrchová normála v bodě průniku
    float kr;                                                   // Fresnelův faktor
    vector<bool> shadows;                                       // Pole udávající jestli je v bodě průniku stín pro každé světlo
    valarray<Color> diffuse, specular;                          // Pole difúzních/spekulárních osvětlení pro každé světlo
    Color light, ambient, texture, reflection, transmission;    // Pole barev intenzity světla/ambientního světla/textury/odrazu/průhledu pro každé světlo
    
    Color shaded();     // Výsledná barva v průniku
    
    Timer timer;        // Časovač
};

// Samotná funkce pro vysílání paprsků
RayIntersection castRay(Vector3, Vector3, const vector<Object *> &, const vector<Light *> &, RayInput mask);
