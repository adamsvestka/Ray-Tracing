//
//  settings.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/16/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#pragma once

#include "data_types.hpp"

// Seznam vykreslovacích režimů
enum RenderType {
    RENDER_SHADED, RENDER_COLOR, RENDER_REFLECTION, RENDER_TRANSMISSION, RENDER_LIGHT, RENDER_SHADOWS, RENDER_NORMALS, RENDER_INORMALS, RENDER_DEPTH, RENDER_UNIQUE, RenderTypes
};

// Seznam vykreslovacích vzorů
enum RenderPattern {
    PATTERN_HORIZONTAL, PATTERN_VERTICAL, PATTERN_SPIRAL, RenderPatterns
};

struct Settings {
    // MARK: Ray
    // Maximální dosah paprsků
    short max_render_distance = 1e2;
    
    // Posunutí paprsku po nárazu do povrchu (aby se předešlo tzn. stínové akné)
    float surface_bias = 1e-3;
    
    // Maximální počet odrazů paprsku (aby se předešlo nekonečné smyčce)
    short max_light_bounces = 5;
    
    // MARK: Camera
    // Vykreslovací režim
    short render_mode = RENDER_SHADED;
    
    // Vzor vykreslování
    short render_pattern = PATTERN_VERTICAL;
    
    // Zobrazení informací v horním levém rohu
    bool show_debug = true;
    
    // Zrychlení vykreslování tím, že se některé hodnoty odhadnou (nejspíš budou viditelné drobné chyby)
    bool preprocess = false;
    
    // Uložení výsledného obrazu do RAM
    bool save_render = true;
    
    // Umělé snížení vykreslovacího rozlišení
    short resolution_decrease = 1;
    
    // Velikost regionů, po kterých se scéna vykresluje
    short render_region_size = 10;
    
    // Počet vláken pro vykreslování
    short rendering_threads = 25;
    
    // Barva pixelů, na kterých se nic nenachází
    Color background_color = Color::Black;
};

extern Settings settings;
