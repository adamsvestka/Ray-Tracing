//
//  file_managers.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 7/23/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#pragma once

#include <iostream>
#include <variant>
#include <fstream>
#include <iomanip>
#include <regex>
#include <map>

#include <nlohmann/json.hpp>

#include "settings.hpp"
#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"

using namespace std;
using json = nlohmann::json;

typedef variant<bool *, short *, float *, Color *> SettingValue;

void parseSettings(string, Settings &);
