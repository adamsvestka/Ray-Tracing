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
    RENDER_SHADED, RENDER_COLOR, RENDER_LIGHT, RENDER_SHADOWS, RENDER_NORMALS, RENDER_DEPTH
};

enum RenderPattern {
    PATTERN_DEFAULT, PATTERN_SPIRAL
};

struct Settings {
    
    // MARK: Ray
    short max_render_distance = 20;
    
    float probe_step_size = 0.1;
    
    float quick_step_size = 0.05;
    
    float precise_step_size = 0.01;
    
    short max_light_bounces = 5;
    
    short surface_smoothing = 5;
        
    // MARK: Camera
    short render_mode = RENDER_SHADED;
    
    short render_pattern = PATTERN_SPIRAL;
    
    short resolution_decrese = 1;
    
    short field_of_view = 120;
    
    short render_region_size = 20;
    
    Color environment_color = Gray;

    float environment_intensity = 0.4;
    
};

extern Settings settings;
