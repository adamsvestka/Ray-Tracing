//
//  light_sources.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct Light;

#pragma once

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"

struct Light {
    Vector3 position;
    Color color;
    int intensity;
};
