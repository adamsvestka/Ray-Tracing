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
#include "objects.hpp"
#include "light_sources.hpp"
#include "camera.hpp"
#include "interfaces.hpp"

using namespace std;
using json = nlohmann::json;

class Parser {
private:
    NativeInterface &interface;
    map<string, Shader> shaders;
    
    Vector3 parseVector(json);
    Color parseColor(string);
    Shader parseShader(json);
    Material parseMaterial(json);
    Object *parseObject(json);
    Light *parseLight(json);
    Camera parseCamera(json);
    
    void parseGeometry_obj(string, vector<array<Vector3, 3>> &, vector<array<VectorUV, 3>> &, vector<array<Vector3, 3>> &);
    
public:
    explicit Parser(NativeInterface &);
    
    void parseSettings(string, Settings &);
    void parseScene(string, Camera &, vector<Object *> &, vector<Light *> &);
};
