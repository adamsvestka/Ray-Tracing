//
//  camera.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "camera.hpp"

// MARK: - NCURSES
#ifdef using_ncurses
Camera::Camera(Vector3 position, int fov, int region_size) {
    float screenHeight, screenWidth;
    
    initscr();
    cbreak();
    noecho();
//    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    getmaxyx(stdscr, screenHeight, screenWidth);

    init_pair(0, COLOR_WHITE, COLOR_BLACK);
    init_pair(1, COLOR_WHITE, COLOR_WHITE);
    
    attrset(COLOR_PAIR(1));
    for (int x = 0; x < screenWidth; x++) for (int y = 0; y < screenHeight; y++) mvaddch(y, x, ' ');
    refresh();
    attrset(COLOR_PAIR(0));
    
    this->width = screenWidth / 2;
    this->height = screenHeight;
    this->position = position;
    fovFactor = 1 / tan(fov / 2);
    this->region_size = region_size;
    region_count = ceil((float)this->width / region_size) * ceil((float)this->height / region_size);
    region_current = 0;
    
    switch (settings.render_pattern) {
        case 1:
            x = round(this->width / region_size / 2) * region_size;
            y = round(this->height / region_size / 2) * region_size;
            r = i = 0;
            l = 1;
            break;
            
        default:
            x = y = 0;
            break;
    }
}

Camera::~Camera() {
    while (getch() != 'q');
    
    endwin();
}

//const char *intensity = " .:-=+*#%@";
//const char *intensity = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
// 21 M
// 20 W8B
// 19 &
// 18 #$
// 17 mwdb%
// 16 XQZ
// 15 Uqpkh
// 14 0O@
// 13 I1nua*
// 12 lrxzYJCo
// 11 itfv
// 10 ?][}{L
// 9 ><+)(jc
// 8 ^|\/
// 7 "!
// 5 :;~-
// 3 '
// 2 .`,
// 0 _
const char *intensity = " .':\"^>?ilI0UXm#&WM";
//const char *intensity = " _.';:\"!^>?ilI0UXm#&WM";
const unsigned short depth = strlen(intensity);
void pixel(int x, int y, float power) {
    int i = floor(power * depth);
    i = max(min(i, depth - 1), 0);

    mvaddch(y, 2 * x, intensity[i]);
    mvaddch(y, 2 * x + 1, intensity[i]);
}

void Camera::render(std::vector<Shape*> objects, std::vector<Light> lights) {
    clock_t time = clock();
    do {
        for (int x = minX(); x < maxX(); x++) {
            for (int y = minY(); y < maxY(); y++) {
                Ray ray = getCameraRay(x, y);
                if (ray.traceObject(objects) > 0) {
                    pixel(x, y, ray.traceLight(lights, objects));
                } else pixel(x, y, Color{0, 0, 0});
            }
        }
        refresh();
    } while (next());
    
    mvprintw(0, 0, "Render time: %f", (double)(clock() - time) / CLOCKS_PER_SEC);
    
    refresh();
}

// MARK: - X11
#else
Camera::Camera(Vector3 position, int fov, int region_size) {
    float screenHeight, screenWidth;
    
    display = XOpenDisplay(NULL);
    scr = DefaultScreen(display);
    gc = DefaultGC(display, scr);
    screen = ScreenOfDisplay(display, scr);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, screenWidth = screen->width, screenHeight = screen->height, 1, BlackPixel(display, scr), WhitePixel(display, scr));

    XSetStandardProperties(display, window, "Ray Tracing", "XTerm", None, NULL, 0, NULL);
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);

    XClearWindow(display, window);
    XMapRaised(display, window);
    
    this->width = screenWidth / settings.resolution;
    this->height = screenHeight / settings.resolution;
    this->position = position;
    fovFactor = 1 / tan(fov / 2);
    this->region_size = region_size;
    region_count = ceil((float)this->width / region_size) * ceil((float)this->height / region_size);
    region_current = 0;
    
    switch (settings.render_pattern) {
        case 1:
            x = round(this->width / region_size / 2) * region_size;
            y = round(this->height / region_size / 2) * region_size;
            r = i = 0;
            l = 1;
            break;
           
        default:
            x = y = 0;
            break;
   }
}

Camera::~Camera() {
    XEvent event;
    KeySym key;
    char text[255];

    while(true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) if (text[0] == 'q') break;
    }
    
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

#include <sstream>
#include <iomanip>

using namespace std;

string formatTime(float seconds) {
    int milliseconds = (int)(seconds * 1000) % 1000;
    seconds = floor(seconds);
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    int hours = minutes / 60;
    minutes -= hours * 60;
    
    stringstream ss;
    ss << setfill('0') << setw(2) << hours << ':' << setw(2) << minutes << ':' << setw(2) << seconds << '.' << setw(3) << milliseconds;
    return ss.str();
}

void Camera::render(std::vector<Shape*> objects, std::vector<Light> lights) {
    clock_t start = clock();
    while (true) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == Expose && event.xexpose.count == 0) {
            do {
                for (int x = minX(); x < maxX(); x++) {
                    for (int y = minY(); y < maxY(); y++) {
                        Ray ray = getCameraRay(x, y);
                        
                        if (ray.traceObject(objects) > 0) XSetForeground(display, gc, ray.traceLight(lights, objects));
                        else XSetForeground(display, gc, settings.environment_color);
//                        XDrawPoint(display, window, gc, x, y);
                        XFillRectangle(display, window, gc, x * settings.resolution, y * settings.resolution, settings.resolution, settings.resolution);
                    }
                }
                
                XSetForeground(display, gc, Black);
                XFillRectangle(display, window, gc, 2, 2, 158, 48);
                
                XSetForeground(display, gc, Green);
                
                float elapsed = (float)(clock() - start) / CLOCKS_PER_SEC;
                string time = "Render time: " + formatTime(elapsed);
                XDrawString(display, window, gc, 5, 15, time.c_str(), (int)time.length());

                string str = "Regions: " + to_string(region_current) + "/" + to_string(region_count) + " @ " + to_string(region_size) + " px";
                XDrawString(display, window, gc, 5, 30, str.c_str(), (int)str.length());
                
                string time2 = "ETC: " + formatTime(elapsed / region_current * region_count - elapsed);
                XDrawString(display, window, gc, 5, 45, time2.c_str(), (int)time2.length());
            } while (next());

            XSetForeground(display, gc, Black);
            XFillRectangle(display, window, gc, 2, 2, 158, 48);
            
            XSetForeground(display, gc, Green);
            
            float elapsed = (float)(clock() - start) / CLOCKS_PER_SEC;
            string time = "Render time: " + formatTime(elapsed);
            XDrawString(display, window, gc, 5, 15, time.c_str(), (int)time.length());

            string str = "Regions: " + to_string(region_current) + "/" + to_string(region_count) + " @ " + to_string(region_size) + " px";
            XDrawString(display, window, gc, 5, 30, str.c_str(), (int)str.length());
            
            string time2 = "ETC: " + formatTime(elapsed / region_current * region_count - elapsed);
            XDrawString(display, window, gc, 5, 45, time2.c_str(), (int)time2.length());
            
            break;
        }
    }
}
#endif
// MARK: -

Ray Camera::getCameraRay(int x, int y) {
#ifdef using_ncurses
    float u = (4 * (float)x / width - 1) / 2 * width / height;
#else
    float u = (2 * (float)x / width - 1) * width / height;
#endif
    float v = 1 - (2 * (float)y / height);
    
    Vector3 direction{fovFactor, u, v};
    Ray ray(position, direction.normal());
    
    return ray;
}

bool Camera::next() {
    region_current++;
    switch (settings.render_pattern) {
        case PATTERN_SPIRAL: // MARK: Spiral
            do {
                switch (r) {
                    case 0: x += region_size; break;
                    case 1: y -= region_size; break;
                    case 2: x -= region_size; break;
                    case 3: y += region_size; break;
                }
                if (++i >= l) {
                    if (++r % 2 == 0) l++;
                    if (r > 3) r = 0;
                    i = 0;
                }
                
                if (x >= width && y >= height) return false;
            } while (x >= width || x + region_size <= 0 || y >= height || y + region_size <= 0);
            
            return true;
            
        default: // MARK: Default
            x += region_size;
            if (x >= width) {
                x = 0;
                y += region_size;
            }
            
            return y < height;
    }
}

int Camera::minX() { return fmax(x, 0); };
int Camera::maxX() { return fmin(x + region_size, width); };
int Camera::minY() { return fmax(y, 0); };
int Camera::maxY() { return fmin(y + region_size, height); };
