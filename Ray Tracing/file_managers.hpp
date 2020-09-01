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
#include <sstream>
#include <iomanip>
#include <regex>
#include <map>

#include <nlohmann/json.hpp>

#include "settings.hpp"
#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"
#include "interfaces.hpp"

using namespace std;
using json = nlohmann::json;

typedef variant<bool *, short *, float *, Color *> SettingValue;

class Parser {
private:
    NativeInterface *interface;
    map<string, Shader> shaders;
    
    Vector3 parseVector(json);
    Color parseColor(string);
    Shader parseShader(json);
    Material parseMaterial(json);
    Shape *parseShape(json j);
    Light *parseLight(json j);
    
    vector<array<Vector3, 3>> parseOBJ(string);
    
public:
    Parser(NativeInterface *);
    void parseSettings(string, Settings &);
    void parseScene(string, vector<Shape *> &, vector<Light *> &);
};

