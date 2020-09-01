//
//  interfaces.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct DebugStats;

class NativeInterface;
class X11Interface;

#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"

struct DebugStats {
    int region_current, region_count, render_time, object_count;
    Timer timer;
    string render_mode_name;
};


class NativeInterface {
protected:
    int width, height;
    
public:
    void getDimensions(int &, int &);
    
    virtual void drawPixel(int, int, Color) = 0;
    virtual void drawDebugBox(int, int, Input) = 0;
    virtual void renderInfo(DebugStats) = 0;
    virtual void refresh() = 0;
    virtual char getChar() = 0;
    
    virtual bool loadFile(string, stringstream &) = 0;
    virtual bool saveFile(string, const stringstream &) = 0;
    
    virtual void log(string) = 0;
};

class X11Interface : public NativeInterface {
private:
    Display *display;
    Screen *screen;
    int scr;
    Window window;
    GC gc;
    
    inline void drawInfoString(int, int, stringstream &, Color);
    
public:
    X11Interface();
    ~X11Interface();
    
    void drawPixel(int, int, Color);
    void drawDebugBox(int, int, Input);
    void renderInfo(DebugStats);
    void refresh();
    char getChar();
    
    bool loadFile(string, stringstream &);
    bool saveFile(string, const stringstream &);
    
    void log(string);
};
