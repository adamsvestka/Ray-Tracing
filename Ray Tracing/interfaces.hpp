//
//  interfaces.hpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

struct DebugInfo;

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

// Informace pro vykreslení v levém horním rohu
struct DebugInfo {
    int region_current, region_count, render_time, object_count;
    Timer timer;
    string render_mode_name;
};


// Prototyp třídy pro interakci s OS (obrazovka, klávesnice, soubory, log)
class InterfaceTemplate {
protected:
    int width, height;
    
public:
    void getDimensions(int &, int &);   // Rozlišení obrazovky
    
    virtual void drawPixel(int, int, Color) = 0;        // Vykreslit pixel na souřadnicích
    virtual void drawDebugBox(int, int, RayInput) = 0;  // Vykreslit ladící kříž
    virtual void renderInfo(DebugInfo) = 0;             // Vykreslit dodatečné informace
    virtual void refresh() = 0; // Obnovit obrazovku
    virtual char getChar() = 0; // Vstup z klávesnice
    
    virtual bool loadFile(string, stringstream &) = 0;          // Načíst soubor
    virtual bool saveFile(string, const stringstream &) = 0;    // Uložit soubor
    
    virtual bool loadImage(string, Buffer &) = 0;       // Načíst obrázek
    virtual bool saveImage(string, const Buffer &) = 0; // Uložit obrázek
    
    virtual void log(const string &) = 0;   // Zapsat zprávu do logu
};

#ifndef __EMSCRIPTEN__

// Nativní verze pro macOS
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
    string path;
    
    inline string wrapFilename(string);
    inline void drawInfoString(int, int, stringstream &, Color);
    
public:
    X11Interface(int, const char **);
    ~X11Interface();
    
    void drawPixel(int, int, Color);
    void drawDebugBox(int, int, RayInput);
    void renderInfo(DebugInfo);
    void refresh();
    char getChar();
    
    bool loadFile(string, stringstream &);
    bool saveFile(string, const stringstream &);
    
    bool loadImage(string, Buffer &);
    bool saveImage(string, const Buffer &);
    
    void log(const string &);
};

#else

// Verze pro WebAssembly
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

using namespace emscripten;

#define WASMInterface NativeInterface
class WASMInterface : public InterfaceTemplate {
private:
    // HTML elementy
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
    WASMInterface(int, const char **);
    ~WASMInterface();
    
    void drawPixel(int, int, Color);
    void drawDebugBox(int, int, RayInput);
    void renderInfo(DebugInfo);
    void refresh();
    char getChar();
    
    bool loadFile(string, stringstream &);
    bool saveFile(string, const stringstream &);
    
    bool loadImage(string, Buffer &);
    bool saveImage(string, const Buffer &);
    
    void log(const string &);
    
    // Pomocné funkce
    static EM_BOOL key_callback(int, const EmscriptenKeyboardEvent *, void *);
    void push_key(int);
    static void init_image(val, val);
};

#endif
