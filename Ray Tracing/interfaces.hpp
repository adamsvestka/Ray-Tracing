//
//  interfaces.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct DebugStats;

class InterfaceTemplate;
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

#include "settings.hpp"

#include "data_types.hpp"
#include "ray.hpp"

using namespace std;

struct DebugStats {
    int region_current, region_count, render_time, object_count;
    Timer timer;
    string render_mode_name;
};


class InterfaceTemplate {
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
    
    virtual bool loadImage(string, Buffer &) = 0;
    virtual bool saveImage(string, const Buffer &) = 0;
    
    virtual void log(string) = 0;
};

#ifndef __EMSCRIPTEN__

#include <png.h>
#include <jpeglib.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define cimg_use_png
#define cimg_use_jpeg
#include <CImg.h>
#pragma clang diagnostic pop

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

using namespace cimg_library;

#define X11Interface NativeInterface
class X11Interface : public InterfaceTemplate {
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
    
    bool loadImage(string, Buffer &);
    bool saveImage(string, const Buffer &);
    
    void log(string);
};

#else

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

using namespace emscripten;

#define WASMInterface NativeInterface
class WASMInterface : public InterfaceTemplate {
private:
    val window = val::undefined();
    val document = val::undefined();
    val canvas = val::undefined();
    val context = val::undefined();
    val hud = val::undefined();
    val keys = val::undefined();
    static val image;
    
    inline void drawInfoString(int, int, stringstream &, Color);
    inline void drawBoxCorner(vector<array<short, 2>>);
    
public:
    WASMInterface();
    ~WASMInterface();
    
    void drawPixel(int, int, Color);
    void drawDebugBox(int, int, Input);
    void renderInfo(DebugStats);
    void refresh();
    char getChar();
    
    bool loadFile(string, stringstream &);
    bool saveFile(string, const stringstream &);
    
    bool loadImage(string, Buffer &);
    bool saveImage(string, const Buffer &);
    
    void log(string);
    
    static EM_BOOL key_callback(int, const EmscriptenKeyboardEvent *, void *);
    void push_key(int);
    static void init_image(val, val);
};

#endif
