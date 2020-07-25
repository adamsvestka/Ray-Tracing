//
//  file_managers.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 7/23/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "file_managers.hpp"

SettingsParser::SettingsParser(string filename) : filename(filename) {};

void SettingsParser::parse() {
    cout << "Reading settings.ini" << endl;
    initBindings();
    
    regex pair("\\s*(.+?)\\s*=\\s*(.+)");
    regex hex("([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
    regex ignore("\\[.*\\]|#.*");
    
    smatch matches;
    string line;
    
    ifstream ifile(filename, ios::in | ios::out);
    if (ifile.is_open()) {
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
                        case 3:
                            regex_search(line, matches, hex);
                            *get<Color *>(value) = Color(stoi(matches[1].str(), 0, 16) / 255.f, stoi(matches[2].str(), 0, 16) / 255.f, stoi(matches[3].str(), 0, 16) / 255.f);
                            break;
                        default: break;
                    }
                    
                    bindings.erase(it);
                } catch(...) {
                    cout << "Invalid value: " << line << endl;
                }
            } else cout << "Invalid entry: " << line << endl;
        }
        
        ifile.close();
    } else cout << "File doesn't exist" << endl;
    
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
                    ofile << setfill('0') << setw(2) << std::hex << (int)round(c.r * 255) << setw(2) << (int)round(c.g * 255) << setw(2) << (int)round(c.b * 255);
                    ofile.flags(f);
                } break;
                default: break;
            }
            ofile << endl;
        }
    } else cout << "Unable to create file" << endl;
}

void SettingsParser::initBindings() {
    // Ray
    bindOption(&settings.max_render_distance, "max_render_distance");
    bindOption(&settings.surface_bias, "surface_bias");
    bindOption(&settings.max_light_bounces, "max_light_bounces");
    
    // Camera
    bindOption(&settings.render_mode, "render_mode");
    bindOption(&settings.render_pattern, "render_pattern");
    bindOption(&settings.show_debug, "show_debug");
    bindOption(&settings.preprocess, "preprocess");
    bindOption(&settings.save_render, "save_render");
    bindOption(&settings.field_of_view, "field_of_view");
    bindOption(&settings.resolution_decrease, "resolution_decrease");
    bindOption(&settings.render_region_size, "render_region_size");
    bindOption(&settings.rendering_threads, "rendering_threads");
    bindOption(&settings.ambient_lighting, "ambient_lighting");
    bindOption(&settings.background_color, "background_color");
}

void SettingsParser::bindOption(SettingValue value, string key) {
    bindings[key] = value;
}
