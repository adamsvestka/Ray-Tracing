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
Parser::Parser(NativeInterface &interface) : interface(interface) {};

void Parser::parseSettings(string filename, Settings &settings) {
    interface.log("Parsing " + filename);
    
    // MARK: Bind options
//    map<string, SettingValue> bindings;
    map<string, pair<short, void *>> bindings;
    // Ray
    bindings["max_render_distance"] = {1, &settings.max_render_distance};
    bindings["surface_bias"] = {2, &settings.surface_bias};
    bindings["max_light_bounces"] = {1, &settings.max_light_bounces};
    
    // Camera
    bindings["render_mode"] = {1, &settings.render_mode};
    bindings["render_pattern"] = {1, &settings.render_pattern};
    bindings["show_debug"] = {0, &settings.show_debug};
    bindings["preprocess"] = {0, &settings.preprocess};
    bindings["save_render"] = {0, &settings.save_render};
    bindings["resolution_decrease"] = {1, &settings.resolution_decrease};
    bindings["render_region_size"] = {1, &settings.render_region_size};
    bindings["rendering_threads"] = {1, &settings.rendering_threads};
    bindings["ambient_lighting"] = {3, &settings.ambient_lighting};
    bindings["background_color"] = {3, &settings.background_color};
    
    // MARK: Parse file to structure
    stringstream buffer;
    if (interface.loadFile(filename, buffer)) {
        regex pair("\\s*(.+?)\\s*=\\s*(.+)");
        regex ignore("\\[.*\\]|#.*");
        
        smatch matches;
        string line;
        
        while (getline(buffer, line)) {
            line = regex_replace(line, ignore, "");
            if (all_of(line.begin(), line.end(), ::isspace)) continue;
            
            if (regex_search(line, matches, pair)) {
                try {
                    auto it = bindings.find(matches[1].str());
                    if (it == bindings.end()) {
                        interface.log("Unused key: " + line);
                        continue;
                    }
                    
                    auto &value = bindings[matches[1].str()];
                    switch (value.first) {
                        case 0: *(bool *)(value.second) = matches[2].str() == "true"; break;
                        case 1: *(short *)(value.second) = (short)stoi(matches[2].str()); break;
                        case 2: *(float *)(value.second) = stof(matches[2].str()); break;
                        case 3: *(Color *)(value.second) = parseColor(matches[2].str()); break;
                    }
                    
                    bindings.erase(it);
                } catch(...) {
                    interface.log("Invalid value: " + line);
                }
            } else interface.log("Invalid entry: " + line);
        }
    } else interface.log("Unable to open file");
    
    // MARK: Add missing tags to file
    stringstream append;
    append.setf(ios_base::boolalpha);
    for (auto it : bindings) {
        interface.log("Missing entry: " + it.first);
        
        append << it.first << "=";
        switch (it.second.first) {
            case 0: append << *(bool *)(it.second.second); break;
            case 1: append << *(short *)(it.second.second); break;
            case 2: append << *(float *)(it.second.second); break;
            case 3: {
                Color c = *(Color *)(it.second.second);
                ios_base::fmtflags f(append.flags());
                append << 'x' << setfill('0') << setw(2) << std::hex << (int)round(c.r * 255) << setw(2) << (int)round(c.g * 255) << setw(2) << (int)round(c.b * 255);
                append.flags(f);
            } break;
        }
        append << endl;
    }
    if (!interface.saveFile(filename, append)) interface.log("Unable to update file");
}


// MARK: - Scene
Vector3 Parser::parseVector(json j) {
    if (!j.is_null()) return {j.value("x", 0.f), j.value("y", 0.f), j.value("z", 0.f)};
    
    return Vector3::Zero;
}

Color Parser::parseColor(string s) {
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

Shader Parser::parseShader(json j) {
    if (!j.is_null()) {
        switch (::hash(j.value("type", "").c_str())) {
            case "color"_h: return [color = parseColor(j.value("color", ""))](TCoords t) { return color; };
            case "image"_h: {
                Buffer buffer;
                if (!interface.loadImage(j.value("name", "image.png"), buffer)) interface.log("Couldn't open image");
                return [image = Image(buffer)](TCoords t) { return image(t); };
            }
            case "checkerboard"_h: return [checkerboard = Checkerboard(j.value("scale", 2), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")))](TCoords t) { return checkerboard(t); };
            case "bricks"_h: return [bricks = Brick(j.value("scale", 4), j.value("ratio", 2.f), j.value("mortar", 0.1f), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")), parseColor(j.value("tertiary", "")), j.value("seed", 0))](TCoords t) { return bricks(t); };
            case "noise"_h: return [noise = Noise(j.value("scale", 1), j.value("seed", 0), parseColor(j.value("primary", "")))](TCoords t) { return noise(t); };
                
            case "named"_h: {
                if (!j["name"].is_string()) break;
                string name = j["name"];
                if (shaders.find(name) == shaders.end()) break;
                return shaders[name];
            }
                
            case "negate"_h:
                if (!j["value"].is_object()) break;
                return [shader = parseShader(j["value"])](TCoords t) { return -shader(t); };
            case "add"_h: {
                if (!j["values"].is_array()) break;
                vector<Shader> operands;
                for (auto &shader : j["values"]) operands.push_back(parseShader(shader));
                return [=](TCoords t) { Color result = Color::Black; for (auto &shader : operands) result += shader(t); return result; };
            }
            case "multiply"_h: {
                if (!j["values"].is_array()) break;
                vector<Shader> operands;
                for (auto &shader : j["values"]) operands.push_back(parseShader(shader));
                return [=](TCoords t) { Color result = Color::White; for (auto &shader : operands) result *= shader(t); return result; };
            }
            case "mix"_h: {
                if (!j["values"].is_array() || !j["weights"].is_array() || j["values"].size() != j["weights"].size()) break;
                vector<pair<Shader, float>> operands;
                for (int i = 0; i < j["values"].size(); i++) operands.push_back(pair<Shader, float>(parseShader(j["values"][i]), j["weights"][i]));
                return [=](TCoords t) { Color result = Color::Black; for (auto &[shader, weight] : operands) result += shader(t) * weight; return result; };
            }
        }
    }
    
    return [](TCoords t) { return Color::Black; };
}

Material Parser::parseMaterial(json j) {
    if (!j.is_null()) return {parseShader(j["shader"]), j.value("n", 0.f), j.value("Ks", 0.f), j.value("ior", 1.f), j.value("transparent", false)};
    
    return {parseShader(j["shader"]), 0.f, 0.f, 1.f, false};
}

Shape *Parser::parseShape(json j) {
    switch (::hash(j.value("type", "").c_str())) {
        case "sphere"_h: return new Sphere(parseVector(j["position"]), j.value("diameter", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube"_h: return new Cuboid(parseVector(j["position"]), j.value("size", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-2"_h: return new Cuboid(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), j.value("size_z", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-3"_h: return new Cuboid(parseVector(j["corner_min"]), parseVector(j["corner_max"]), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "plane"_h: return new Plane(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "object"_h: {
            vector<array<Vector3, 3>> vertices;
            vector<array<TCoords, 3>> textures;
            vector<array<Vector3, 3>> normals;
            parseOBJ(j.value("name", "object.obj"), vertices, textures, normals);
            return new Object({vertices, textures, normals, parseVector(j["position"]), j.value("scale", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"])});
        }
    }
    return nullptr;
}

Light *Parser::parseLight(json j) {
    switch (::hash(j.value("type", "").c_str())) {
        case "point"_h: return new PointLight(parseVector(j["position"]), parseColor(j["color"]), j.value("intensity", 1000));
        case "linear"_h: return new LinearLight(parseVector(j["position"]), parseColor(j["color"]), j.value("intensity", 300));
        case "global"_h: return new GlobalLight(parseColor(j["color"]), j.value("intensity", 0.5f));
        case "directional"_h: return new DirectionalLight(parseVector(j["direction"]), parseColor(j["color"]), j.value("intensity", 0.5f));
    }
    return nullptr;
}

Camera Parser::parseCamera(json j) {
    return Camera(parseVector(j["position"]), parseVector(j["rotation"]), 0, 0, j.value("fov", 120));
}

void Parser::parseScene(string filename, Camera &camera, vector<Shape *> &objects, vector<Light *> &lights) {
    interface.log("Parsing " + filename);
    
    stringstream buffer;
    if (interface.loadFile(filename, buffer)) {
        json jfile;
        
        const string camera_key = "camera", shaders_key = "shaders", objects_key = "objects", lights_key = "lights";
        
        buffer >> jfile;
        
        // MARK: Parse camera from file
        if (jfile[camera_key].is_object()) {
            camera = parseCamera(jfile[camera_key]);
        } else interface.log("Missing entry: " + camera_key);
        
        // MARK: Parse shaders from file
        if (jfile[shaders_key].is_object()) {
            for (const auto &[name, jshader] : jfile[shaders_key].items()) {
                shaders[name] = parseShader(jshader);
            }
        } else interface.log("Missing entry: " + shaders_key);
        
        // MARK: Parse objects from file
        if (jfile[objects_key].is_array()) {
            for (const auto &jobject : jfile[objects_key]) {
                auto object = parseShape(jobject);
                if (object != nullptr) objects.push_back(object);
            }
        } else interface.log("Missing entry: " + objects_key);
        
        // MARK: Parse lights from file
        if (jfile[lights_key].is_array()) {
            for (const auto &jlight : jfile[lights_key]) {
                auto light = parseLight(jlight);
                if (light != nullptr) lights.push_back(light);
            }
        } else interface.log("Missing entry: " + lights_key);
    } else interface.log("Unable to open file");
}


// MARK: - Wavefront .obj
void Parser::parseOBJ(string filename, vector<array<Vector3, 3>> &overtices, vector<array<TCoords, 3>> &otextures, vector<array<Vector3, 3>> &onormals) {
    interface.log("Parsing " + filename);
    
    stringstream buffer;
    if (interface.loadFile(filename, buffer)) {
        vector<Vector3> vertices;
        vector<TCoords> textures;
        vector<Vector3> normals;
        
        regex ignore("#.*");
        string line;
        
        while (getline(buffer, line)) {
            line = regex_replace(line, ignore, "");
            if (all_of(line.begin(), line.end(), ::isspace)) continue;
            
            stringstream ss(line);
            string type;
            ss >> type;
            switch (::hash(type.c_str())) {
                case "v"_h: {
                    float x, y, z;
                    ss >> x >> y >> z;
                    vertices.push_back({x, y, z});
                } break;
                case "vt"_h: {
                    float u, v;
                    ss >> u >> v;
                    textures.push_back({u, v});
                } break;
                case "vn"_h: {
                    float x, y, z;
                    ss >> x >> y >> z;
                    normals.push_back({x, y, z});
                } break;
                case "f"_h: {
                    vector<int> vindices;
                    vector<int> tindices;
                    vector<int> nindices;
                    regex pattern("(\\d+)(?:/(\\d+)(?:/(\\d+))?)?");
                    smatch matches;
                    string ids;
                    while (ss >> ids) {
                        regex_search(ids, matches, pattern);
                        if (!matches[1].str().empty()) vindices.push_back(stoi(matches[1].str()) - 1);
                        if (!matches[2].str().empty()) tindices.push_back(stoi(matches[2].str()) - 1);
                        if (!matches[3].str().empty()) nindices.push_back(stoi(matches[3].str()) - 1);
                    }
                    for (int i = 1; i < vindices.size() - 1; i++) {
                        overtices.push_back({vertices.at(vindices[0]), vertices.at(vindices[i]), vertices.at(vindices[i + 1])});
                        if (vindices.size() == tindices.size()) otextures.push_back({textures.at(tindices[0]), textures.at(tindices[i]), textures.at(tindices[i + 1])});
                        if (vindices.size() == nindices.size()) onormals.push_back({normals.at(nindices[0]), normals.at(nindices[i]), normals.at(nindices[i + 1])});
                    }
                } break;
            }
        }
    } else interface.log("Unable to open file");
}
