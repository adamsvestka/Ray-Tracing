//
//  settings.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/16/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#pragma once

#include "data_types.hpp"

enum RenderType {
    RENDER_SHADED, RENDER_COLOR, RENDER_REFLECTION, RENDER_TRANSMISSION, RENDER_LIGHT, RENDER_SHADOWS, RENDER_NORMALS, RENDER_DEPTH, RenderTypes
};

enum RenderPattern {
    PATTERN_HORIZONTAL, PATTERN_VERTICAL, PATTERN_SPIRAL, RenderPatterns
};

struct Settings {
    
    // MARK: Ray
    short max_render_distance = 1e2;
    
    float surface_bias = 1e-3;
    
    short max_light_bounces = 5;
    
    short surface_smoothing = 4;
    
    // MARK: Camera
    short render_mode = RENDER_SHADED;
    
    short render_pattern = PATTERN_VERTICAL;
    
    bool show_debug = true;
    
    bool preprocess = false;
    
    bool save_render = true;
    
    short field_of_view = 120;
    
    short resolution_decrese = 1;
    
    short render_region_size = 10;
    
    short rendering_threads = 25;
    
    Color ambient_lighting = Color::Gray / 2;
    
    Color background_color = Color::Gray;
    
};

extern Settings settings;
