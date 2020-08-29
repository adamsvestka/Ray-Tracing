//
//  file_managers.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 7/23/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "file_managers.hpp"

// Hash functions to switch string
typedef std::uint64_t hash_t;

constexpr hash_t prime = 0x100000001B3ull;
constexpr hash_t basis = 0xCBF29CE484222325ull;

hash_t hash(char const *str) {
    hash_t ret{basis};
    while (*str) {
        ret ^= *str;
        ret *= prime;
        str++;
    }
    return ret;
}

constexpr hash_t hash_compile_time(char const *str, hash_t last_value = basis) {
    return *str ? hash_compile_time(str + 1, (*str ^ last_value) * prime) : last_value;
}

constexpr unsigned long long operator ""_h(char const *p, size_t) {
    return hash_compile_time(p);
}


// MARK: - Settings
void parseSettings(string filename, Settings &settings) {
    cout << "Parsing " << filename << endl;
    
    // MARK: Bind options
    map<string, SettingValue> bindings;
    // Ray
    bindings["max_render_distance"] = &settings.max_render_distance;
    bindings["max_render_distance"] = &settings.max_render_distance;
    bindings["surface_bias"] = &settings.surface_bias;
    bindings["max_light_bounces"] = &settings.max_light_bounces;
    
    // Camera
    bindings["render_mode"] = &settings.render_mode;
    bindings["render_pattern"] = &settings.render_pattern;
    bindings["show_debug"] = &settings.show_debug;
    bindings["preprocess"] = &settings.preprocess;
    bindings["save_render"] = &settings.save_render;
    bindings["field_of_view"] = &settings.field_of_view;
    bindings["resolution_decrease"] = &settings.resolution_decrease;
    bindings["render_region_size"] = &settings.render_region_size;
    bindings["rendering_threads"] = &settings.rendering_threads;
    bindings["ambient_lighting"] = &settings.ambient_lighting;
    bindings["background_color"] = &settings.background_color;
    
    // MARK: Parse file to structure
    ifstream ifile(filename, ios::in);
    if (ifile.is_open()) {
        regex pair("\\s*(.+?)\\s*=\\s*(.+)");
        regex ignore("\\[.*\\]|#.*");
        
        smatch matches;
        string line;
        
        while (getline(ifile, line)) {
            line = regex_replace(line, ignore, "");
            if (all_of(line.begin(), line.end(), ::isspace)) continue;
            
            if (regex_search(line, matches, pair)) {
                try {
                    auto it = bindings.find(matches[1].str());
                    if (it == bindings.end()) {
                        cout << "Unused key: " << line << endl;
                        continue;
                    }
                    
                    auto &value = bindings[matches[1].str()];
                    switch (value.index()) {
                        case 0: *get<bool *>(value) = matches[2].str() == "true"; break;
                        case 1: *get<short *>(value) = (short)stoi(matches[2].str()); break;
                        case 2: *get<float *>(value) = stof(matches[2].str()); break;
                        case 3: *get<Color *>(value) = parseColor(matches[2].str()); break;
                    }
                    
                    bindings.erase(it);
                } catch(...) {
                    cout << "Invalid value: " << line << endl;
                }
            } else cout << "Invalid entry: " << line << endl;
        }
        
        ifile.close();
    } else cout << "Unable to open file" << endl;
    
    // MARK: Add missing tags to file
    ofstream ofile(filename, ios::out | ios::app);
    if (ofile.is_open()) {
        ofile.setf(ios_base::boolalpha);
        
        for (auto it : bindings) {
            cout << "Missing entry: " << it.first << endl;
            
            ofile << it.first << "=";
            switch (it.second.index()) {
                case 0: ofile << *get<bool *>(it.second); break;
                case 1: ofile << *get<short *>(it.second); break;
                case 2: ofile << *get<float *>(it.second); break;
                case 3: {
                    Color c = *get<Color *>(it.second);
                    ios_base::fmtflags f(ofile.flags());
                    ofile << 'x' << setfill('0') << setw(2) << std::hex << (int)round(c.r * 255) << setw(2) << (int)round(c.g * 255) << setw(2) << (int)round(c.b * 255);
                    ofile.flags(f);
                } break;
            }
            ofile << endl;
        }
        
        ofile.close();
    } else cout << "Unable to create file" << endl;
}


// MARK: - Scene
Vector3 parseVector(json j) {
    if (!j.is_null()) return {j.value("x", 0.f), j.value("y", 0.f), j.value("z", 0.f)};
    
    return Vector3::Zero;
}

Color parseColor(string s) {
    static regex hex("x([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
    static map<string, Color> colors{{"white", Color::White}, {"lightGray", Color::Gray.light()}, {"gray", Color::Gray}, {"darkGray", Color::Gray.dark()}, {"black", Color::Black},
        {"red", Color::Red}, {"orange", Color::Orange}, {"yellow", Color::Yellow}, {"pink", Color::Pink}, {"brown", Color::Brown}, {"mocha", Color::Mocha}, {"asparagus", Color::Asparagus},
        {"lime", Color::Lime}, {"green", Color::Green}, {"moss", Color::Moss}, {"fern", Color::Fern},
        {"blue", Color::Blue}, {"cyan", Color::Cyan}, {"magenta", Color::Magenta}, {"teal", Color::Teal}, {"indigo", Color::Indigo}, {"purple", Color::Purple}};
    
    smatch matches;
    map<string, Color>::iterator it;
    
    if (regex_search(s, matches, hex)) return Color(stoi(matches[1].str(), 0, 16) / 255.f, stoi(matches[2].str(), 0, 16) / 255.f, stoi(matches[3].str(), 0, 16) / 255.f);
    if ((it = colors.find(s)) != colors.end()) return it->second;
    
    return Color::Black;
}

map<string, Shader> shaders;
Shader parseShader(json j) {
    if (!j.is_null()) {
        switch (::hash(j.value("type", "").c_str())) {
            case "color"_h: return [color = parseColor(j.value("color", ""))](float u, float v) { return color; };
            case "image"_h: return [image = Image(j.value("name", "image.png"))](float u, float v) { return image(u, v); };
            case "checkerboard"_h: return [checkerboard = Checkerboard(j.value("scale", 2), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")))](float u, float v) { return checkerboard(u, v); };
            case "bricks"_h: return [bricks = Brick(j.value("scale", 4), j.value("ratio", 2.f), j.value("mortar", 0.1f), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")), parseColor(j.value("tertiary", "")), j.value("seed", 0))](float u, float v) { return bricks(u, v); };
            case "noise"_h: return [noise = Noise(j.value("scale", 1), j.value("seed", 0), parseColor(j.value("primary", "")))](float u, float v) { return noise(u, v); };
                
            case "named"_h: {
                if (!j["name"].is_string()) break;
                string name = j["name"];
                if (shaders.find(name) == shaders.end()) break;
                return shaders[name];
            }
                
            case "negate"_h:
                if (!j["value"].is_object()) break;
                return [shader = parseShader(j["value"])](float u, float v) { return -shader(u, v); };
            case "add"_h: {
                if (!j["values"].is_array()) break;
                vector<Shader> operands;
                for (auto &shader : j["values"]) operands.push_back(parseShader(shader));
                return [=](float u, float v) { Color result = Color::Black; for (auto &shader : operands) result += shader(u, v); return result; };
            }
            case "multiply"_h: {
                if (!j["values"].is_array()) break;
                vector<Shader> operands;
                for (auto &shader : j["values"]) operands.push_back(parseShader(shader));
                return [=](float u, float v) { Color result = Color::White; for (auto &shader : operands) result *= shader(u, v); return result; };
            }
            case "mix"_h: {
                if (!j["values"].is_array() || !j["weights"].is_array() || j["values"].size() != j["weights"].size()) break;
                vector<pair<Shader, float>> operands;
                for (int i = 0; i < j["values"].size(); i++) operands.push_back(pair<Shader, float>(parseShader(j["values"][i]), j["weights"][i]));
                return [=](float u, float v) { Color result = Color::Black; for (auto &[shader, weight] : operands) result += shader(u, v) * weight; return result; };
            }
        }
    }
    
    return [](float u, float v) { return Color::Black; };
}

Material parseMaterial(json j) {
    if (!j.is_null()) return {parseShader(j["shader"]), j.value("n", 0.f), j.value("Ks", 0.f), j.value("ior", 1.f), j.value("transparent", false)};
    
    return {parseShader(j["shader"]), 0.f, 0.f, 1.f, false};
}

Shape *parseShape(json j) {
    switch (::hash(j.value("type", "").c_str())) {
        case "sphere"_h: return new Sphere(parseVector(j["position"]), j.value("diameter", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube"_h: return new Cuboid(parseVector(j["position"]), j.value("size", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-2"_h: return new Cuboid(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), j.value("size_z", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-3"_h: return new Cuboid(parseVector(j["corner_min"]), parseVector(j["corner_max"]), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "plane"_h: return new Plane(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "object"_h: return new Object({parseOBJ(j.value("name", "object.obj")), parseVector(j["position"]), j.value("scale", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"])});
    }
    return nullptr;
}

Light *parseLight(json j) {
    switch (::hash(j.value("type", "").c_str())) {
        case "point"_h: return new PointLight(parseVector(j["position"]), parseColor(j["color"]), j.value("intensity", 1000));
        case "linear"_h: return new LinearLight(parseVector(j["position"]), parseColor(j["color"]), j.value("intensity", 300));
        case "global"_h: return new GlobalLight(parseColor(j["color"]), j.value("intensity", 0.5f));
        case "directional"_h: return new DirectionalLight(parseVector(j["direction"]), parseColor(j["color"]), j.value("intensity", 0.5f));
    }
    return nullptr;
}

void parseScene(string filename, vector<Shape *> &objects, vector<Light *> &lights) {
    cout << "Parsing " << filename << endl;
    
    ifstream ifile(filename, ios::in);
    if (ifile.is_open()) {
        json jfile;
        
        const string shaders_key = "shaders", objects_key = "objects", lights_key = "lights";
        
        ifile >> jfile;
        
        // MARK: Parse shaders from file
        if (jfile[shaders_key].is_object()) {
            for (const auto &[name, jshader] : jfile[shaders_key].items()) {
                shaders[name] = parseShader(jshader);
            }
        } else cout << "Missing entry: " << shaders_key << endl;
        
        // MARK: Parse objects from file
        if (jfile[objects_key].is_array()) {
            for (const auto &jobject : jfile[objects_key]) {
                auto object = parseShape(jobject);
                if (object != nullptr) objects.push_back(object);
            }
        } else cout << "Missing entry: " << objects_key << endl;
        
        // MARK: Parse lights from file
        if (jfile[lights_key].is_array()) {
            for (const auto &jlight : jfile[lights_key]) {
                auto light = parseLight(jlight);
                if (light != nullptr) lights.push_back(light);
            }
        } else cout << "Missing entry: " << lights_key << endl;
        
        ifile.close();
    } else cout << "Unable to open file" << endl;
}


// MARK: - Wavefront .obj
vector<array<Vector3, 3>> parseOBJ(string filename) {
    cout << "Parsing " << filename << endl;

    vector<array<Vector3, 3>> triangles;
    
    ifstream ifile(filename, ios::in);
    if (ifile.is_open()) {
        vector<Vector3> vertices;
        
        regex ignore("#.*");
        smatch matches;
        string line;
        
        while (getline(ifile, line)) {
            line = regex_replace(line, ignore, "");
            if (all_of(line.begin(), line.end(), ::isspace)) continue;
            
            stringstream ss(line);
            string type;
            ss >> type;
            switch (::hash(type.c_str())) {
                case "v"_h: {
                    float x, y, z;
                    ss >> x >> y >> z;
                    Vector3 vertex{x, y, z};
                    vertices.push_back(vertex);
                } break;
                case "f"_h: {
                    vector<int> indices;
                    int i;
                    while (ss >> i) indices.push_back(i - 1);
                    for (i = 1; i < indices.size() - 1; i++) {
                        array<Vector3, 3> triangle{vertices.at(indices[0]), vertices.at(indices[i]), vertices.at(indices[i + 1])};
                        triangles.push_back(triangle);
                    }
                } break;
            }
        }
        
        ifile.close();
    } else cout << "Unable to open file" << endl;
    
    return triangles;
}
