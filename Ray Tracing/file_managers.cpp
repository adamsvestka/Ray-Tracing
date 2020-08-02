//
//  file_managers.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 7/23/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "file_managers.hpp"

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
                        default: break;
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
                default: break;
            }
            ofile << endl;
        }
    } else cout << "Unable to create file" << endl;
}


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

// MARK: - Scene
Vector3 parseVector(json j) {
    if (!j.is_null()) return {j.value("x", 0.f), j.value("y", 0.f), j.value("z", 0.f)};
    
    return Vector3::Zero;
}

Color parseColor(string s) {
    static regex hex("x([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
    static map<string, Color> colors{{"white", Color::White}, {"gray", Color::Gray}, {"black", Color::Black},
            {"red", Color::Red}, {"orange", Color::Orange}, {"yellow", Color::Yellow}, {"lime", Color::Lime},
            {"green", Color::Green}, {"turquoise", Color::Turquoise}, {"cyan", Color::Cyan},
            {"blue", Color::Blue}, {"azure", Color::Azure}, {"purple", Color::Purple}, {"magenta", Color::Magenta}, {"pink", Color::Pink}};
    smatch matches;
    map<string, Color>::iterator it;
    
    if (regex_search(s, matches, hex)) return Color(stoi(matches[1].str(), 0, 16) / 255.f, stoi(matches[2].str(), 0, 16) / 255.f, stoi(matches[3].str(), 0, 16) / 255.f);
    if ((it = colors.find(s)) != colors.end()) return it->second;
    
    cout << "Invalid Color" << endl;
    return Color::Black;
    
}

function<Color(float, float)> parseShader(json j) {
    if (!j.is_null()) {
        switch (::hash(j.value("type", "").c_str())) {
            case "color"_h: return [color = parseColor(j.value("color", ""))](float u, float v) { return color; };
            case "image"_h: return [image = Image(j.value("name", "image.png"))](float u, float v) { return image(u, v); };
            case "checkerboard"_h: return [checkerboard = Checkerboard(j.value("scale", 2), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")))](float u, float v) { return checkerboard(u, v); };
            case "bricks"_h: return [bricks = Brick(j.value("scale", 2), j.value("ratio", 2.f), j.value("mortar", 0.1f), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")))](float u, float v) { return bricks(u, v); };
            case "noise"_h: return [noise = Noise(j.value("scale", 1), j.value("seed", 0), parseColor(j.value("primary", "")))](float u, float v) { return noise(u, v); };
        }
    }
    
    return [](float u, float v) { return Color::Black; };
}

Material parseMaterial(json j) {
    if (!j.is_null()) return {parseShader(j["texture"]), j.value("n", 0.f), j.value("Ks", 0.f), j.value("ior", 1.f), j.value("transparent", false)};
    
    return {parseShader(j["texture"]), 0.f, 0.f, 1.f, false};
}

Shape *parseShape(json j) {
    switch (::hash(j.value("type", "").c_str())) {
        case "sphere"_h: return new Sphere(parseVector(j["position"]), j.value("diameter", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube"_h: return new Cuboid(parseVector(j["position"]), j.value("size", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-2"_h: return new Cuboid(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), j.value("size_z", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-3"_h: return new Cuboid(parseVector(j["corner_min"]), parseVector(j["corner_max"]), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "plane"_h: return new Plane(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
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
    
    ifstream ifile(filename, ios::in | ios::out);
    if (ifile.is_open()) {
        json jfile;
        
        const string objects_key = "objects", lights_key = "lights";
        
        ifile >> jfile;
        
        // MARK: Parse objects from file
        if (jfile.contains(objects_key)) {
            if (jfile[objects_key].is_array()) {
                for (const auto &jobject : jfile[objects_key]) {
                    cout << jobject << endl;
                    auto object = parseShape(jobject);
                    if (object != nullptr) objects.push_back(object);
                }
            } else cout << "Invalid type for " << objects_key << ": " << jfile[objects_key].type_name() << endl;
        } else cout << "Missing entry: " << objects_key << endl;
        
        // MARK: Parse lights from file
        if (jfile.contains(lights_key)) {
            if (jfile[lights_key].is_array()) {
                for (const auto &jlight : jfile[lights_key]) {
                    cout << jlight << endl;
                    auto light = parseLight(jlight);
                    if (light != nullptr) lights.push_back(light);
                }
            } else cout << "Invalid type for " << lights_key << ": " << jfile[lights_key].type_name() << endl;
        } else cout << "Missing entry: " << lights_key << endl;
    } else cout << "Unable to open file" << endl;
}
