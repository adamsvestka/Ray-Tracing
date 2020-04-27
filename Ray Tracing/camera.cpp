//
//  camera.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "camera.hpp"

// MARK: Initialization
Camera::Camera(Vector3 position) {
    float screenHeight, screenWidth;
    
    display = XOpenDisplay(nullptr);
    scr = DefaultScreen(display);
    gc = DefaultGC(display, scr);
    screen = ScreenOfDisplay(display, scr);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, screenWidth = screen->width, screenHeight = screen->height, 1, BlackPixel(display, scr), WhitePixel(display, scr));

    XSetStandardProperties(display, window, "Ray Tracing", "XTerm", None, nullptr, 0, nullptr);
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);

    XClearWindow(display, window);
    XMapRaised(display, window);
    
    this->width = screenWidth / settings.resolution_decrese;
    this->height = screenHeight / settings.resolution_decrese;
    this->position = position;
    fovFactor = 1 / tan(settings.field_of_view / 2);
    region_count = 0;
    region_current = 0;
    
    resetPosition();
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

// MARK: - Helper functions
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

void Camera::drawPixel(int x, int y, int s, Intersection data) {
    switch (settings.render_mode) {
        case RENDER_COLOR: XSetForeground(display, gc, data.object->material.color); break;
        case RENDER_LIGHT: XSetForeground(display, gc, data.hit ? data.light : Black); break;
        case RENDER_SHADOWS: XSetForeground(display, gc, data.hit ? (data.shadow ? Red : data.light) : Black); break;
        case RENDER_NORMALS: XSetForeground(display, gc, data.hit ? data.normal.toColor() : Black); break;
        case RENDER_DEPTH: XSetForeground(display, gc, White * (1 - data.distance / settings.max_render_distance)); break;
        case RENDER_SHADED:
        default: XSetForeground(display, gc, data.shaded()); break;
    }
    
    XFillRectangle(display, window, gc, x * settings.resolution_decrese * s, y * settings.resolution_decrese * s, settings.resolution_decrese * s, settings.resolution_decrese * s);
}

void Camera::drawBox(int x, int y) {
    const auto size = 0.3;

    XPoint points[4][3] = {{
        {(short)((x * settings.resolution_decrese + size) * settings.render_region_size), (short)(y * settings.resolution_decrese * settings.render_region_size)},
        {(short)(x * settings.resolution_decrese * settings.render_region_size), (short)(y * settings.resolution_decrese * settings.render_region_size)},
        {(short)(x * settings.resolution_decrese * settings.render_region_size), (short)((y * settings.resolution_decrese + size) * settings.render_region_size)}
    }, {
        {(short)(((x + 1) * settings.resolution_decrese - size) * settings.render_region_size - 2), (short)(y * settings.resolution_decrese * settings.render_region_size)},
        {(short)((x + 1) * settings.resolution_decrese * settings.render_region_size - 2), (short)(y * settings.resolution_decrese * settings.render_region_size)},
        {(short)((x + 1) * settings.resolution_decrese * settings.render_region_size - 2), (short)((y * settings.resolution_decrese + size) * settings.render_region_size)}
    }, {
        {(short)((x * settings.resolution_decrese + size) * settings.render_region_size), (short)((y + 1) * settings.resolution_decrese * settings.render_region_size - 2)},
        {(short)(x * settings.resolution_decrese * settings.render_region_size), (short)((y + 1) * settings.resolution_decrese * settings.render_region_size - 2)},
        {(short)(x * settings.resolution_decrese * settings.render_region_size), (short)(((y + 1) * settings.resolution_decrese - size) * settings.render_region_size - 2)}
    }, {
        {(short)(((x + 1) * settings.resolution_decrese - size) * settings.render_region_size - 2), (short)((y + 1) * settings.resolution_decrese * settings.render_region_size - 2)},
        {(short)((x + 1) * settings.resolution_decrese * settings.render_region_size - 2), (short)((y + 1) * settings.resolution_decrese * settings.render_region_size - 2)},
        {(short)((x + 1) * settings.resolution_decrese * settings.render_region_size - 2), (short)(((y + 1) * settings.resolution_decrese - size) * settings.render_region_size - 2)}
    }};

    int npoints = sizeof(points[0]) / sizeof(XPoint);
    for (int i = 0; i < 4; i++) XDrawLines(display, window, gc, points[i], npoints, CoordModeOrigin);
}


// MARK: - Preprocessing
vector<vector<Intersection>> Camera::preRender(vector<Shape*> objects, vector<Light> lights) {
    const int regions_x = ceil((float)width / settings.render_region_size);
    const int regions_y = ceil((float)height / settings.render_region_size);
    
    vector<vector<Intersection>> buffer(regions_x);
    for (int x = 0; x < regions_x; x++) {
        buffer[x].resize(regions_y);
        for (int y = 0; y < regions_y; y++) {
            auto ray = castRay(position, getCameraRay((x + 0.5) * settings.render_region_size, (y + 0.5) * settings.render_region_size), objects, lights, Input(settings.probe_step_size));
            
            buffer[x][y] = ray;
            
            drawPixel(x, y, settings.render_region_size, ray);
        }
    }
    
    return buffer;
}

vector<vector<short>> Camera::processPreRender(vector<vector<Intersection>> buffer) {
    const int regions_x = (int)buffer.size();
    const int regions_y = (int)buffer[0].size();
    vector<vector<short>> processed(regions_x);
    
    vector<vector<vector<float>>> nodesDepth = {{{0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}}};
    vector<vector<vector<float>>> nodesEdge = {{{-0.5}, {-1.5}, {-2.5}, {-3.5}, {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5}, {-4.5}, {-5.5}, {-6.5}, {-7.5}}, {{0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}}};
    NeuralNetwork filterDepth(nodesDepth);
    NeuralNetwork filterEdge(nodesEdge);
    
    vector<Shape *> ids;
    
    for (int x = 0; x < regions_x; x++) {
        processed[x].resize(regions_y);
        for (int y = 0; y < regions_y; y++) {
            vector<float> matrixDepth(9);
            vector<float> matrixObjects(9);
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    const auto item = buffer[min(max(x + i - 1, 0), regions_x - 1)][min(max(y + j - 1, 0), regions_y - 1)];
                    matrixDepth[3*i+j] = 1 - item.distance / settings.max_render_distance;
                    
                    const auto it = find(ids.begin(), ids.end(), item.object);
                    if (it != ids.end()) matrixObjects[3*i+j] = distance(ids.begin(), it);
                    else {
                        matrixObjects[3*i+j] = ids.size();
                        ids.push_back(item.object);
                    }
                }
            }
            
            if (filterDepth.eval(matrixDepth)[0] == 0) processed[x][y] = false;
            else {
                if (abs(filterEdge.eval(matrixObjects)[0]) != 0) {
                    processed[x][y] = 2;
                    XSetForeground(display, gc, DarkRed);
                    
                    region_count += settings.quick_step_size / settings.precise_step_size;
                } else {
                    processed[x][y] = 1;
                    XSetForeground(display, gc, Gray);
                    
                    region_count++;
                }
                
                drawBox(x, y);
            }
        }
    }
    
    return processed;
}

// MARK: - Rendering
void Camera::renderInfo() {
    XSetForeground(display, gc, Black);
    XFillRectangle(display, window, gc, 2, 2, 158, 62);
    
    XSetForeground(display, gc, Green);
    
    const float elapsed = (float)(clock() - time) / CLOCKS_PER_SEC;
    stringstream ss;
    ss << "Render time: " << formatTime(elapsed);
    XDrawString(display, window, gc, 5, 15, ss.str().c_str(), (int)ss.str().length());
    
    ss.str("");
    ss << "Region: " << region_current << "/" << region_count << " @ " << settings.render_region_size << " px";
    XDrawString(display, window, gc, 5, 30, ss.str().c_str(), (int)ss.str().length());
    
    ss.str("");
    ss << "ETC: " << formatTime(fmax(elapsed / (region_current ? region_current : 1) * region_count - elapsed, 0.0)) << " - " << fixed << setprecision(region_current >= region_count ? 0 : 1) << 100.f * region_current / region_count << defaultfloat << "%";
    XDrawString(display, window, gc, 5, 45, ss.str().c_str(), (int)ss.str().length());
    
    ss.str("");
    ss << width << "x" << height << " px @ " << settings.precise_step_size << " / " << settings.max_render_distance;
    XDrawString(display, window, gc, 5, 60, ss.str().c_str(), (int)ss.str().length());
}

void Camera::renderRegion(vector<Shape *> objects, vector<Light> lights, short mask) {
    for (int x = minX; x < maxX; x++) {
        for (int y = minY; y < maxY; y++) {
            auto ray = castRay(position, getCameraRay(x, y), objects, lights, Input(mask == 1 ? settings.quick_step_size : settings.precise_step_size));
            
            drawPixel(x, y, 1, ray);
        }
    }
}

void Camera::render(std::vector<Shape*> objects, std::vector<Light> lights) {
    while (true) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == Expose && event.xexpose.count == 0) {
            time = clock();
            
            // Aproximate regions
            const auto buffer = preRender(objects, lights);
            
            // Process what to render
            const auto mask = processPreRender(buffer);
            
            // Render
            do {
                XSetForeground(display, gc, Orange);
                drawBox(minX / settings.render_region_size, minY / settings.render_region_size);
                XFlush(display);
                
                const auto type = mask[minX/settings.render_region_size][minY/settings.render_region_size];
                renderRegion(objects, lights, type);
                if (type == 2) region_current += settings.quick_step_size / settings.precise_step_size;
                else region_current++;
                renderInfo();
            } while (next(mask));
            
            // Render stats
            renderInfo();
            
            XFlush(display);
            
            break;
        }
    }
}

// MARK: - Region management
Vector3 Camera::getCameraRay(int x, int y) {
    float u = (2 * (float)x / width - 1) * width / height;
    float v = 1 - (2 * (float)y / height);
    
    return Vector3{fovFactor, u, v}.normal();
}

void Camera::generateRange() {
    minX = fmax(x, 0);
    maxX = fmin(x + settings.render_region_size, width);
    minY = fmax(y, 0);
    maxY = fmin(y + settings.render_region_size, height);
}

void Camera::resetPosition() {
    switch (settings.render_pattern) {
        case PATTERN_SPIRAL:
            x = round(this->width / settings.render_region_size / 2) * settings.render_region_size;
            y = round(this->height / settings.render_region_size / 2) * settings.render_region_size;
            r = i = 0;
            l = 1;
            break;

        case PATTERN_HORIZONTAL:
        case PATTERN_VERTICAL:
        default:
            x = y = 0;
            break;
    }
    
    minX = fmax(x, 0);
    maxX = fmin(x + settings.render_region_size, width);
    minY = fmax(y, 0);
    maxY = fmin(y + settings.render_region_size, height);
}

bool Camera::next(vector<vector<short>> mask) {
    switch (settings.render_pattern) {
        case PATTERN_VERTICAL: // MARK: Vertical
            do {
                y += settings.render_region_size;
                if (y >= height) {
                    y = 0;
                    x += settings.render_region_size;
                }
            } while (x < width && !mask[x/settings.render_region_size][y/settings.render_region_size]);
            
            generateRange();
            
            return x < width;
            
        case PATTERN_SPIRAL: // MARK: Spiral
            do {
                switch (r) {
                    case 0: x += settings.render_region_size; break;
                    case 1: y -= settings.render_region_size; break;
                    case 2: x -= settings.render_region_size; break;
                    case 3: y += settings.render_region_size; break;
                }
                if (++i >= l) {
                    if (++r % 2 == 0) l++;
                    if (r > 3) r = 0;
                    i = 0;
                }
                
                if (x >= width && y >= height) return false;
            } while (x >= width || x + settings.render_region_size <= 0 || y >= height || y + settings.render_region_size <= 0 || !mask[x/settings.render_region_size][y/settings.render_region_size]);
            
            generateRange();
            
            return true;

        case PATTERN_HORIZONTAL: // MARK: Horizontal
        default:
            do {
                x += settings.render_region_size;
                if (x >= width) {
                    x = 0;
                    y += settings.render_region_size;
                }
            } while (y < height && !mask[x/settings.render_region_size][y/settings.render_region_size]);
            
            generateRange();
            
            return y < height;
    }
}
