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
    RENDER_SHADED, RENDER_COLOR, RENDER_REFLECTION, RENDER_TRANSMISSION, RENDER_LIGHT, RENDER_SHADOWS, RENDER_NORMALS, RENDER_DEPTH
};

enum RenderPattern {
    PATTERN_HORIZONTAL, PATTERN_VERTICAL, PATTERN_SPIRAL
};

struct Settings {
    
    // MARK: Ray
    short max_render_distance = 100;
    
    float probe_step_size = 0.1;
    
    float quick_step_size = 0.1;
    
    float precise_step_size = 0.01;
    
    short max_light_bounces = 5;
    
    short surface_smoothing = 4;
    
    // MARK: Camera
    short render_mode = RENDER_SHADED;
    
    short render_pattern = PATTERN_SPIRAL;
    
    bool show_debug = true;
    
    short field_of_view = 120;
    
    short resolution_decrese = 1;
    
    short render_region_size = 10;
    
    short rendering_threads = 25;
    
    Color ambient_lighting = Gray / 2;
    
    Color background_color = Azure;
    
};

extern Settings settings;
