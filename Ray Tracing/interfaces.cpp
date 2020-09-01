//
//  interfaces.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "interfaces.hpp"

void NativeInterface::getDimensions(int &w, int &h) {
    w = width;
    h = height;
}


// MARK: - X11Interface
X11Interface::X11Interface() {
    display = XOpenDisplay(nullptr);
    scr = DefaultScreen(display);
    gc = DefaultGC(display, scr);
    screen = ScreenOfDisplay(display, scr);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, screen->width, screen->height, 1, BlackPixel(display, scr), WhitePixel(display, scr));

    XSetStandardProperties(display, window, "Ray Tracing", "XTerm", None, nullptr, 0, nullptr);
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);

    XClearWindow(display, window);
    XMapRaised(display, window);

    width = screen->width / settings.resolution_decrease;
    height = screen->height / settings.resolution_decrease;
    
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == Expose && event.xexpose.count == 0) break;
    }
}

X11Interface::~X11Interface() {
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

void X11Interface::drawPixel(int x, int y, Color c) {
    XSetForeground(display, gc, c);
    XFillRectangle(display, window, gc, x * settings.resolution_decrease, y * settings.resolution_decrease, settings.resolution_decrease, settings.resolution_decrease);
}

void X11Interface::drawDebugBox(int x, int y, Input mask) {
    const auto size = 0.3;

    XPoint points[4][3] = {{
        {(short)((x * settings.resolution_decrease + size) * settings.render_region_size), (short)(y * settings.resolution_decrease * settings.render_region_size)},
        {(short)(x * settings.resolution_decrease * settings.render_region_size), (short)(y * settings.resolution_decrease * settings.render_region_size)},
        {(short)(x * settings.resolution_decrease * settings.render_region_size), (short)((y * settings.resolution_decrease + size) * settings.render_region_size)}
    }, {
        {(short)(((x + 1) * settings.resolution_decrease - size) * settings.render_region_size - 2), (short)(y * settings.resolution_decrease * settings.render_region_size)},
        {(short)((x + 1) * settings.resolution_decrease * settings.render_region_size - 2), (short)(y * settings.resolution_decrease * settings.render_region_size)},
        {(short)((x + 1) * settings.resolution_decrease * settings.render_region_size - 2), (short)((y * settings.resolution_decrease + size) * settings.render_region_size)}
    }, {
        {(short)((x * settings.resolution_decrease + size) * settings.render_region_size), (short)((y + 1) * settings.resolution_decrease * settings.render_region_size - 2)},
        {(short)(x * settings.resolution_decrease * settings.render_region_size), (short)((y + 1) * settings.resolution_decrease * settings.render_region_size - 2)},
        {(short)(x * settings.resolution_decrease * settings.render_region_size), (short)(((y + 1) * settings.resolution_decrease - size) * settings.render_region_size - 2)}
    }, {
        {(short)(((x + 1) * settings.resolution_decrease - size) * settings.render_region_size - 2), (short)((y + 1) * settings.resolution_decrease * settings.render_region_size - 2)},
        {(short)((x + 1) * settings.resolution_decrease * settings.render_region_size - 2), (short)((y + 1) * settings.resolution_decrease * settings.render_region_size - 2)},
        {(short)((x + 1) * settings.resolution_decrease * settings.render_region_size - 2), (short)(((y + 1) * settings.resolution_decrease - size) * settings.render_region_size - 2)}
    }};

    int npoints = sizeof(points[0]) / sizeof(XPoint);

    if (mask.bounce_count < 0) {
        XSetForeground(display, gc, Color::Orange);
        for (int i = 0; i < 4; i++) XDrawLines(display, window, gc, points[i], npoints, CoordModeOrigin);
        return;
    }

    if (mask.transmission) XSetForeground(display, gc, Color::Green);
    else XSetForeground(display, gc, Color::Gray.dark());
    XDrawLines(display, window, gc, points[0], npoints, CoordModeOrigin);

    if (mask.reflections) XSetForeground(display, gc, Color::Blue);
    else if (any_of(mask.shadows.begin(), mask.shadows.end(), [](bool b) { return b; })) XSetForeground(display, gc, Color::Yellow);
    else XSetForeground(display, gc, Color::Gray.dark());
    XDrawLines(display, window, gc, points[1], npoints, CoordModeOrigin);

    if (any_of(mask.shadows.begin(), mask.shadows.end(), [](bool b) { return b; })) XSetForeground(display, gc, Color::Yellow);
    else if (mask.reflections) XSetForeground(display, gc, Color::Blue);
    else XSetForeground(display, gc, Color::Gray.dark());
    XDrawLines(display, window, gc, points[2], npoints, CoordModeOrigin);

    if (mask.transmission) XSetForeground(display, gc, Color::Green);
    else XSetForeground(display, gc, Color::Gray.dark());
    XDrawLines(display, window, gc, points[3], npoints, CoordModeOrigin);
}

string formatTime(int milliseconds) {
    time_t seconds = milliseconds / 1000;
    milliseconds -= seconds * 1000;

    struct tm result;
    gmtime_r(&seconds, &result);

    stringstream ss;
    ss << put_time(&result, "%T") << '.' << setfill('0') << setw(3) << milliseconds;
    return ss.str();
}

inline void X11Interface::drawInfoString(int x, int y, stringstream &ss, Color color) {
    XSetForeground(display, gc, color);
    XDrawString(display, window, gc, x * 6, y * 15, ss.str().c_str(), (int)ss.str().length());
    ss.str("");
}

void X11Interface::renderInfo(DebugStats stats) {
    XSetForeground(display, gc, Color::Black);
    XFillRectangle(display, window, gc, 2, 2, 1 + 6 * 28, 4 + 15 * 6);
    
    stringstream ss;
    
    ss << "Render time: ";
    drawInfoString(1, 1, ss, Color::Green);
    ss << formatTime(stats.render_time);
    drawInfoString(14, 1, ss, Color::Yellow);
    
    ss << "Region: " << stats.region_current << "/" << stats.region_count << " @ " << settings.render_region_size << " px";
    drawInfoString(1, 2, ss, Color::Green);
    
    ss << "Prog:";
    drawInfoString(1, 3, ss, Color::Green);
    XSetForeground(display, gc, Color::Gray.dark());
    XFillRectangle(display, window, gc, 42, 35, 6 * 15, 12);
    const auto progress = 6 * 15.f * stats.region_current / stats.region_count;
    const auto total = accumulate(stats.timer.times.begin(), stats.timer.times.end(), 0.f);
    float current = 0;
    for (short i = 0; i < stats.timer.c; i++) {
        XSetForeground(display, gc, stats.timer.colors[i]);
        const auto temp = round(progress * stats.timer.times[i] / total);
        XFillRectangle(display, window, gc, current + 42, 35, temp, 12);
        current += temp;
    }
    if (stats.region_current >= stats.region_count) ss << "100%";
    else ss << fixed << setprecision(1) << min(100.f * stats.region_current / stats.region_count, 99.9f) << defaultfloat << "%";
    drawInfoString(23, 3, ss, Color::Orange);
    
    ss << "Quality: " << width << "x" << height << " (0," << settings.max_render_distance << "]";
    drawInfoString(1, 4, ss, Color::Green);
    
    ss << "Complexity: " << stats.object_count << " x " << settings.max_light_bounces;
    drawInfoString(1, 5, ss, Color::Green);
    
    ss << "Mode: " << settings.render_mode << "/" << RenderTypes - 1 << " ";
    drawInfoString(1, 6, ss, Color::Green);
    ss << " (" << stats.render_mode_name << ")";
    drawInfoString(10, 6, ss, Color::Red);
    
    refresh();
}

void X11Interface::refresh() {
    XFlush(display);
}

char X11Interface::getChar() {
    XEvent event;
    KeySym key;
    char ch;
    while(true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress && XLookupString(&event.xkey, &ch, 1, &key, 0) == 1) return ch;
    }
}

bool X11Interface::loadFile(string filename, stringstream &buffer) {
    ifstream ifile(filename, ios::in);
    buffer.str("");
    
    if (ifile.is_open()) {
        buffer << ifile.rdbuf();
        ifile.close();
        return true;
    }
    
    return false;
}


bool X11Interface::saveFile(string filename, const stringstream &buffer) {
    ofstream ofile(filename, ios::out);
    
    if (ofile.is_open()) {
        ofile << buffer.rdbuf();
        ofile.close();
        return true;
    }
    
    return false;
}

void X11Interface::log(string message) {
    cout << message << endl;
}
