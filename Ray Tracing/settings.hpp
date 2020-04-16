//
//  settings.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/16/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#pragma once

struct {
    
    // MARK: Ray
    int render_distance = 20;

    float step_size = 0.05;
    
    int repetitions = render_distance / step_size;
    
    // MARK: Camera
    int render_pattern = 1;
    
    int resolution = 5;

} settings;
