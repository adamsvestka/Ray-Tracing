//
//  file_managers.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 7/23/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <regex>
#include <map>

#include "settings.hpp"
#include "data_types.hpp"

using namespace std;

struct SettingValue {
    short type;
    void *data;
};

class SettingsParser {
private:
    fstream file;
    map<string, SettingValue> bindings;
    
public:
    SettingsParser(string);
    void initBindings();
    void bindOption(SettingValue, string);
};
