//
//  main.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/13/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include <iostream>
#include <cmath>
#include <ncurses.h>

#include "data_types.hpp"
#include "shapes.hpp"
#include "light_sources.hpp"
#include "ray.hpp"
#include "camera.hpp"

using namespace std;

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


int main(int argc, const char * argv[]) {
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

    Camera camera(screenWidth, screenHeight, {0, 0, 0}, 120, 50);
    
#define COUNT 2
    Shape *objects[COUNT] = {new Sphere({20, -2, 0}, 5), new Sphere({16, 6, 0}, 1)};
    Light lights[] = {Light({15, 15, 0}, 1000)};
    
    clock_t time;
    time = clock();
    do {
        for (int x = camera.minX(); x < camera.maxX(); x++) {
            for (int y = camera.minY(); y < camera.maxY(); y++) {
                Ray ray = camera.getCameraRay(x, y);
                
                if (ray.traceObject(objects, COUNT) > 0) {
                    pixel(x, y, ray.traceLight(lights[0], objects, COUNT));
                } else pixel(x, y, -1);
            }
        }
        refresh();
    } while (camera.nextSpiral());
    
    double time_taken = (double)(clock() - time) / CLOCKS_PER_SEC;
    mvprintw(0, 0, "Render time: %f", time_taken);
    
    refresh();
    getch();
    
    endwin();

    return 0;
}
