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
        case RENDER_COLOR: XSetForeground(display, gc, data.texture); break;
        case RENDER_REFLECTION: XSetForeground(display, gc, data.reflection); break;
        case RENDER_TRANSMISSION: XSetForeground(display, gc, data.transmission); break;
        case RENDER_LIGHT: XSetForeground(display, gc, data.hit ? data.light : Black); break;
        case RENDER_SHADOWS: XSetForeground(display, gc, data.hit ? (any_of(data.shadows.begin(), data.shadows.end(), [](bool b) { return b; }) ? Red : data.light) : Black); break;
        case RENDER_NORMALS: XSetForeground(display, gc, data.hit ? data.normal.toColor() : Black); break;
        case RENDER_DEPTH: XSetForeground(display, gc, White * (1 - data.distance / settings.max_render_distance)); break;
        case RENDER_SHADED:
        default: XSetForeground(display, gc, data.shaded()); break;
    }
    
    XFillRectangle(display, window, gc, x * settings.resolution_decrese * s, y * settings.resolution_decrese * s, settings.resolution_decrese * s, settings.resolution_decrese * s);
}

void Camera::drawBox(int x, int y, Input mask) {
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
    
    if (!mask.step_size) XSetForeground(display, gc, Orange);
    else if (any_of(mask.shadows.begin(), mask.shadows.end(), [](bool b) { return b; })) XSetForeground(display, gc, Yellow/2);
    else if (mask.step_size == settings.precise_step_size) XSetForeground(display, gc, DarkRed);
    else XSetForeground(display, gc, Gray);
    XDrawLines(display, window, gc, points[0], npoints, CoordModeOrigin); XDrawLines(display, window, gc, points[3], npoints, CoordModeOrigin);
    
    if (!mask.step_size) XSetForeground(display, gc, Orange);
    else if (mask.reflections) XSetForeground(display, gc, DarkBlue);
    else if (mask.step_size == settings.precise_step_size) XSetForeground(display, gc, DarkRed);
    else XSetForeground(display, gc, Gray);
    XDrawLines(display, window, gc, points[1], npoints, CoordModeOrigin); XDrawLines(display, window, gc, points[2], npoints, CoordModeOrigin);
}


// MARK: - Preprocessing
vector<vector<Intersection>> Camera::preRender(vector<Shape*> objects, vector<Light> lights) {
    const int regions_x = ceil((float)width / settings.render_region_size);
    const int regions_y = ceil((float)height / settings.render_region_size);
    
    vector<vector<Intersection>> buffer(regions_x);
    for (int x = 0; x < regions_x; x++) {
        buffer[x].resize(regions_y);
        for (int y = 0; y < regions_y; y++) {
            auto ray = castRay(position, getCameraRay((x + 0.5) * settings.render_region_size, (y + 0.5) * settings.render_region_size), objects, lights, {settings.probe_step_size, 0, true, true, vector<bool>(objects.size(), true)});
            
            drawPixel(x, y, settings.render_region_size, (buffer[x][y] = ray));
        }
    }
    
    return buffer;
}

vector<vector<Input>> Camera::processPreRender(vector<vector<Intersection>> buffer) {
    const int regions_x = (int)buffer.size();
    const int regions_y = (int)buffer[0].size();
    vector<vector<Input>> processed(regions_x, vector<Input>(regions_y, Input{settings.quick_step_size, 0, true, false, vector<bool>(buffer[0][0].shadows.size(), true)}));
    
    vector<vector<vector<float>>> depth_nodes = {{{0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}}};
    vector<vector<vector<float>>> edge_nodes = {{{-0.5}, {-1.5}, {-2.5}, {-3.5}, {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5}, {-4.5}, {-5.5}, {-6.5}, {-7.5}}, {{0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}}};
    NeuralNetwork depth_filter(depth_nodes);
    NeuralNetwork edge_filter(edge_nodes);
    
    vector<Shape *> ids;
    
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            vector<float> depth_matrix(9);
            vector<float> object_matrix(9);
            vector<float> reflect_matrix(9);
            vector<vector<float>> shadow_matrix(buffer[0][0].shadows.size());
            for (auto it = shadow_matrix.begin(); it != shadow_matrix.end(); it++) it->resize(9);
            
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    auto item = buffer[min(max(x + i - 1, 0), regions_x - 1)][min(max(y + j - 1, 0), regions_y - 1)];
                    auto index = 3 * j + i;
                    
                    depth_matrix[index] = item.hit;
                    
                    const auto it = find(ids.begin(), ids.end(), item.object);
                    if (it != ids.end()) object_matrix[index] = distance(ids.begin(), it);
                    else {
                        object_matrix[index] = ids.size();
                        ids.push_back(item.object);
                    }

                    if (item.hit && item.object->material.Ks) reflect_matrix[index] = item.reflection;
                    else reflect_matrix[3*i+j] = settings.background_color;
                    
                    for (int k = 0; k < shadow_matrix.size(); k++) shadow_matrix[k][index] = item.shadows[k];
                }
            }
            
            if (depth_filter.eval(depth_matrix)[0] == 0) processed[x][y].step_size = false;
            else {
                if (abs(edge_filter.eval(object_matrix)[0]) != 0) {
                    processed[x][y].step_size = settings.precise_step_size;
                    region_count += settings.quick_step_size / settings.precise_step_size;
                } else region_count++;
                
                processed[x][y].reflections = edge_filter.eval(reflect_matrix)[0];
                
                processed[x][y].shadows.resize(buffer[0][0].shadows.size());
                for (int i = 0; i < shadow_matrix.size(); i++) processed[x][y].shadows[i] = edge_filter.eval(shadow_matrix[i])[0];
                
                drawBox(x, y, processed[x][y]);
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

void Camera::renderRegion(vector<Shape *> objects, vector<Light> lights, Input mask, Intersection estimate) {
    for (int x = minX; x < maxX; x++) {
        for (int y = minY; y < maxY; y++) {
            auto ray = castRay(position, getCameraRay(x, y), objects, lights, mask);
            
            if (!mask.reflections && ray.hit && ray.object->material.Ks) ray.reflection = estimate.reflection == Black ? settings.background_color : estimate.reflection;
            for (int i = 0; i < mask.shadows.size(); i++) if (!mask.shadows[i] && ray.hit) ray.shadows[i] = estimate.shadows[i];
            
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
                drawBox(minX / settings.render_region_size, minY / settings.render_region_size, {0});
                XFlush(display);
                
                const auto type = mask[minX/settings.render_region_size][minY/settings.render_region_size];
                renderRegion(objects, lights, type, buffer[minX/settings.render_region_size][minY/settings.render_region_size]);
                
                if (type.step_size == settings.precise_step_size) region_current += settings.quick_step_size / settings.precise_step_size;
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

bool Camera::next(vector<vector<Input>> mask) {
    switch (settings.render_pattern) {
        case PATTERN_VERTICAL: // MARK: Vertical
            do {
                y += settings.render_region_size;
                if (y >= height) {
                    y = 0;
                    x += settings.render_region_size;
                }
            } while (x < width && !mask[x/settings.render_region_size][y/settings.render_region_size].step_size);
            
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
            } while (x >= width || x + settings.render_region_size <= 0 || y >= height || y + settings.render_region_size <= 0 || !mask[x/settings.render_region_size][y/settings.render_region_size].step_size);
            
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
            } while (y < height && !mask[x/settings.render_region_size][y/settings.render_region_size].step_size);
            
            generateRange();
            
            return y < height;
    }
}
