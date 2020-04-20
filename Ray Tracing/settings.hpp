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
    RENDER_ALL, RENDER_NORMALS, RENDER_LIGHT
};

enum RenderPattern {
    PATTERN_DEFAULT, PATTERN_SPIRAL
};

struct {
    
    // MARK: Ray
    int render_distance = 20;

    float step_size = 0.05;
    
    int repetitions = render_distance / step_size;
    
    short render_special = RENDER_ALL;
    
    // MARK: Camera
    int render_pattern = PATTERN_SPIRAL;
    
    int resolution = 5;
    
    Color environment_color = Gray;

    float environment_intensity = 0.4;
    
} settings;
