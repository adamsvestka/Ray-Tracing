//
//  file_managers.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 7/23/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "file_managers.hpp"

// Hashovací funkce pro switch řetězce
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
    interface.log("Načítám " + filename);
    
    // MARK: Bind options
    // Přiřadí textové názvy polím v řetězci
    map<string, pair<short, void *>> bindings;
    // Nastavení paprsku
    bindings["max_render_distance"] = {1, &settings.max_render_distance};
    bindings["surface_bias"] = {2, &settings.surface_bias};
    bindings["max_light_bounces"] = {1, &settings.max_light_bounces};
    
    // Nastavení kamery
    bindings["render_mode"] = {1, &settings.render_mode};
    bindings["render_pattern"] = {1, &settings.render_pattern};
    bindings["show_debug"] = {0, &settings.show_debug};
    bindings["preprocess"] = {0, &settings.preprocess};
    bindings["save_render"] = {0, &settings.save_render};
    bindings["resolution_decrease"] = {1, &settings.resolution_decrease};
    bindings["render_region_size"] = {1, &settings.render_region_size};
    bindings["rendering_threads"] = {1, &settings.rendering_threads};
    bindings["background_color"] = {3, &settings.background_color};
    
    // MARK: Parse file to structure
    stringstream buffer;
    if (interface.loadFile(filename, buffer)) {
        regex pair("\\s*(.+?)\\s*=\\s*(.+)");   // Regex na pár 'klíč=hodnota'
        regex ignore("\\[.*\\]|#.*");   // Regex na komentáře a sekce
        
        smatch matches;
        string line;
        
        while (getline(buffer, line)) {
            // Ignoruje komentáře, sekce a prázdné řádky
            line = regex_replace(line, ignore, "");
            if (all_of(line.begin(), line.end(), ::isspace)) continue;
            
            if (regex_search(line, matches, pair)) {
                try {
                    auto it = bindings.find(matches[1].str());  // Najde element struktury pro klíč
                    if (it == bindings.end()) { // Pokud je v souboru klíč navíc
                        interface.log("Nepoužitý klíč: " + line);
                        continue;
                    }
                    
                    // Podle očekávaného datového typy přiřadí hodnotu
                    auto &value = bindings[matches[1].str()];
                    switch (value.first) {
                        case 0: *(bool *)(value.second) = matches[2].str() == "true"; break;
                        case 1: *(short *)(value.second) = (short)stoi(matches[2].str()); break;
                        case 2: *(float *)(value.second) = stof(matches[2].str()); break;
                        case 3: *(Color *)(value.second) = parseColor(matches[2].str()); break;
                    }
                    
                    bindings.erase(it);
                } catch(...) {
                    interface.log("Neplatná hodnota: " + line);
                }
            } else interface.log("Neplatný záznam: " + line);
        }
    } else interface.log("Soubor nelze otevřít");
    
    // MARK: Add missing tags to file
    // Pokud v souboru chybí klíče, tak se tam dopíší
    stringstream append;
    append.setf(ios_base::boolalpha);
    for (auto it : bindings) {
        interface.log("Chybějící záznam: " + it.first);
        
        // Podle datového typu se vybere způsob záznamu
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
    // Uložení souboru
    if (!interface.saveFile(filename, append)) interface.log("Soubor nelze aktualizovat");
}


// MARK: - Scene
// Načte z řetězce vektor
Vector3 Parser::parseVector(json j) {
    if (!j.is_null()) return {j.value("x", 0.f), j.value("y", 0.f), j.value("z", 0.f)};
    
    return Vector3::Zero;
}

// Načte z řetězce barvu
Color Parser::parseColor(string s) {
    // Buď může být v hexadecimálním formátu
    static regex hex("x([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
    // Nebo jako název barvy
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

// Načte z řetězce shader – z parametrů vygeneruje lambda funkci
Shader Parser::parseShader(json j) {
    if (!j.is_null()) {
        switch (::hash(j.value("type", "").c_str())) {
            // Barva
            case "color"_h: return [color = parseColor(j.value("value", ""))](VectorUV t) { return color; };
            // Obrázek
            case "image"_h: {
                Buffer buffer;
                if (!interface.loadImage(j.value("value", "image.png"), buffer)) interface.log("Obrázek se nepodařilo otevřít");
                return [image = Image(buffer)](VectorUV t) { return image(t); };
            }
            // Šachovnice
            case "checkerboard"_h: return [checkerboard = Checkerboard(j.value("scale", 2), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")))](VectorUV t) { return checkerboard(t); };
            // Cihly
            case "bricks"_h: return [bricks = Bricks(j.value("scale", 4), j.value("ratio", 2.f), j.value("mortar", 0.1f), parseColor(j.value("primary", "")), parseColor(j.value("secondary", "")), parseColor(j.value("tertiary", "")), j.value("seed", 0))](VectorUV t) { return bricks(t); };
            // Šum
            case "noise"_h: return [noise = PerlinNoise(j.value("scale", 1), j.value("seed", 0), parseColor(j.value("primary", "")))](VectorUV t) { return noise(t); };
                
            // Předdefinovaný
            case "named"_h: {
                if (!j["value"].is_string()) break;
                string name = j["value"];
                if (shaders.find(name) == shaders.end()) break;
                return shaders[name];
            }
            
            // Šedý
            case "grayscale"_h:
                if (!j["value"].is_object()) break;
                return [shader = parseShader(j["value"])](VectorUV t) { return Color::White * shader(t).asValue(); };
            // Inverzní
            case "negate"_h:
                if (!j["value"].is_object()) break;
                return [shader = parseShader(j["value"])](VectorUV t) { return -shader(t); };
            // Součet
            case "add"_h: {
                if (!j["values"].is_array()) break;
                vector<Shader> operands;
                for (auto &shader : j["values"]) operands.push_back(parseShader(shader));
                return [=](VectorUV t) { Color result = Color::Black; for (auto &shader : operands) result += shader(t); return result; };
            }
            // Součin
            case "multiply"_h: {
                if (!j["values"].is_array()) break;
                vector<Shader> operands;
                for (auto &shader : j["values"]) operands.push_back(parseShader(shader));
                return [=](VectorUV t) { Color result = Color::White; for (auto &shader : operands) result *= shader(t); return result; };
            }
            // Vážený součet
            case "mix"_h: {
                if (!j["values"].is_array() || !j["weights"].is_array() || j["values"].size() != j["weights"].size()) break;
                vector<pair<Shader, float>> operands;
                for (int i = 0; i < j["values"].size(); i++) operands.push_back(pair<Shader, float>(parseShader(j["values"][i]), j["weights"][i]));
                return [=](VectorUV t) { Color result = Color::Black; for (auto &[shader, weight] : operands) result += shader(t) * weight; return result; };
            }
        }
    }
    
    return [](VectorUV t) { return Color::Black; };
}

// Načte z řetězce materiál
Material Parser::parseMaterial(json j) {
    if (!j.is_null()) return {parseShader(j["shader"]), j.value("n", 0.f), j.value("Ks", 0.f), j.value("ior", 1.f), j.value("transparent", false)};
    
    return {parseShader(j["shader"]), 0.f, 0.f, 1.f, false};
}

// Načte z řetězce objekt
Object *Parser::parseObject(json j) {
    switch (::hash(j.value("type", "").c_str())) {
        case "sphere"_h: return new Sphere(parseVector(j["position"]), j.value("diameter", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube"_h: return new Cuboid(parseVector(j["position"]), j.value("size", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-2"_h: return new Cuboid(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), j.value("size_z", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "cube-3"_h: return new Cuboid(parseVector(j["corner_min"]), parseVector(j["corner_max"]), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "plane"_h: return new Plane(parseVector(j["position"]), j.value("size_x", 1.f), j.value("size_y", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"]));
        case "object"_h: {
            vector<array<Vector3, 3>> vertices;
            vector<array<VectorUV, 3>> textures;
            vector<array<Vector3, 3>> normals;
            parseGeometry_obj(j.value("name", "object.obj"), vertices, textures, normals);  // Předá práci funkci 'parseGeometry_obj'
            return new Mesh({vertices, textures, normals, parseVector(j["position"]), j.value("scale", 1.f), parseVector(j["rotation"]), parseMaterial(j["material"])});
        }
    }
    return nullptr;
}

// Načte z řetězce světlo
Light *Parser::parseLight(json j) {
    switch (::hash(j.value("type", "").c_str())) {
        case "point"_h: return new PointLight(parseVector(j["position"]), parseColor(j["color"]), j.value("intensity", 1000));
        case "linear"_h: return new LinearLight(parseVector(j["position"]), parseColor(j["color"]), j.value("intensity", 300));
        case "global"_h: return new GlobalLight(parseColor(j["color"]), j.value("intensity", 0.5f));
        case "directional"_h: return new DirectionalLight(parseVector(j["direction"]), parseColor(j["color"]), j.value("intensity", 0.5f));
    }
    return nullptr;
}

// Načte z řetězce kameru
Camera Parser::parseCamera(json j) {
    return Camera(parseVector(j["position"]), parseVector(j["rotation"]), 0, 0, j.value("fov", 120));
}

// Načte ze souboru scénu
void Parser::parseScene(string filename, Camera &camera, vector<Object *> &objects, vector<Light *> &lights) {
    interface.log("Načítám " + filename);
    
    stringstream buffer;
    if (interface.loadFile(filename, buffer)) { // Pokusí se otevřít soubor
        json jfile;
        
        const string camera_key = "camera", shaders_key = "shaders", objects_key = "objects", lights_key = "lights";
        
        buffer >> jfile;
        
        // Předá práci podfunkcím pro každou kategorii
        // MARK: Parse camera from file
        if (jfile[camera_key].is_object()) {
            camera = parseCamera(jfile[camera_key]);
        } else interface.log("Chybějící záznam: " + camera_key);
        
        // MARK: Parse shaders from file
        if (jfile[shaders_key].is_object()) {
            for (const auto &[name, jshader] : jfile[shaders_key].items()) {
                shaders[name] = parseShader(jshader);
            }
        } else interface.log("Chybějící záznam: " + shaders_key);
        
        // MARK: Parse objects from file
        if (jfile[objects_key].is_array()) {
            for (const auto &jobject : jfile[objects_key]) {
                auto object = parseObject(jobject);
                if (object != nullptr) objects.push_back(object);
            }
        } else interface.log("Chybějící záznam: " + objects_key);
        
        // MARK: Parse lights from file
        if (jfile[lights_key].is_array()) {
            for (const auto &jlight : jfile[lights_key]) {
                auto light = parseLight(jlight);
                if (light != nullptr) lights.push_back(light);
            }
        } else interface.log("Chybějící záznam: " + lights_key);
    } else interface.log("Soubor nelze otevřít");
}


// MARK: - Wavefront .obj
// Načte trojúhelníkovou síť ze souboru OBJ
void Parser::parseGeometry_obj(string filename, vector<array<Vector3, 3>> &overtices, vector<array<VectorUV, 3>> &otextures, vector<array<Vector3, 3>> &onormals) {
    interface.log("Načítám " + filename);
    
    stringstream buffer;
    if (interface.loadFile(filename, buffer)) { // Pokusí se otevřít soubor
        vector<Vector3> vertices;
        vector<VectorUV> textures;
        vector<Vector3> normals;
        
        regex ignore("#.*");    // Komentáře
        string line;
        
        while (getline(buffer, line)) { // Zpracovává soubor po řádcích
            line = regex_replace(line, ignore, "");
            if (all_of(line.begin(), line.end(), ::isspace)) continue;
            
            stringstream ss(line);
            string type;
            ss >> type;
            switch (::hash(type.c_str())) {
                // Vrchol - 3D vektor
                case "v"_h: {
                    float x, y, z;
                    ss >> x >> y >> z;
                    vertices.push_back({x, y, z});
                } break;
                // Vrcholová texturová souřadnice - 2D vektor
                case "vt"_h: {
                    float u, v;
                    ss >> u >> v;
                    textures.push_back({u, v});
                } break;
                // Vrcholová normálová - 3D vektor
                case "vn"_h: {
                    float x, y, z;
                    ss >> x >> y >> z;
                    normals.push_back({x, y, z});
                } break;
                // Plocha - 3x indexy vrcholu, vrcholových souřadnic a vrcholových normál
                case "f"_h: {
                    vector<int> vindices;
                    vector<int> tindices;
                    vector<int> nindices;
                    regex pattern("(\\d+)(?:/(\\d+)(?:/(\\d+))?)?");
                    smatch matches;
                    string ids;
                    while (ss >> ids) {
                        regex_search(ids, matches, pattern);
                        if (!matches[1].str().empty()) vindices.push_back(stoi(matches[1].str()) - 1);  // Pokud má trojúhelník údaje o vrcholech, tak se načtou
                        if (!matches[2].str().empty()) tindices.push_back(stoi(matches[2].str()) - 1);  // Pokud má trojúhelník údaje o vrcholových texturových souřadnicích, tak se načtou
                        if (!matches[3].str().empty()) nindices.push_back(stoi(matches[3].str()) - 1);  // Pokud má trojúhelník údaje o vrcholových normálách, tak se načtou
                    }
                    for (int i = 1; i < vindices.size() - 1; i++) {
                        // Vygenerované údaje se uloží do výsledných polí
                        overtices.push_back({vertices.at(vindices[0]), vertices.at(vindices[i]), vertices.at(vindices[i + 1])});
                        if (vindices.size() == tindices.size()) otextures.push_back({textures.at(tindices[0]), textures.at(tindices[i]), textures.at(tindices[i + 1])});
                        if (vindices.size() == nindices.size()) onormals.push_back({normals.at(nindices[0]), normals.at(nindices[i]), normals.at(nindices[i + 1])});
                    }
                } break;
            }
        }
    } else interface.log("Soubor nelze otevřít");
}
