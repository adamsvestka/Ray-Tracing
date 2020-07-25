//
//  file_managers.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 7/23/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "file_managers.hpp"

void parseSettings(string filename, Settings &settings) {
    cout << "Parsing settings.ini" << endl;
    
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
    
    regex pair("\\s*(.+?)\\s*=\\s*(.+)");
    regex hex("([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
    regex ignore("\\[.*\\]|#.*");
    
    smatch matches;
    string line;
    
    // MARK: Parse file to structure
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
                    ofile << setfill('0') << setw(2) << std::hex << (int)round(c.r * 255) << setw(2) << (int)round(c.g * 255) << setw(2) << (int)round(c.b * 255);
                    ofile.flags(f);
                } break;
                default: break;
            }
            ofile << endl;
        }
    } else cout << "Unable to create file" << endl;
}
