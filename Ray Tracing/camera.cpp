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
    
    this->width = screenWidth / settings.resolution_decrease;
    this->height = screenHeight / settings.resolution_decrease;
    this->position = position;
    fovFactor = 1 / tan(settings.field_of_view / 2);
    region_current = 0;
    
    resetPosition();
}

Camera::~Camera() {
    XEvent event;
    KeySym key;
    char text[255];
    for (short i = 0; i < timer.c; i++) cout << "Calculating " << timer.names[i] << " took " << timer.times[i] / 1000.f << " seconds" << endl;
    cout << "Total time was " << chrono::duration<float, milli>(end - start).count() / 1000.f << " seconds";
    
    while(true) {
        XNextEvent(display, &event);
        if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
            if (text[0] == 'q') break;
            
            if (!settings.save_render) continue;
            int n = text[0] - '0';
            if (n >= 0 && n < RenderTypes) settings.render_mode = n;
            else continue;
            
            for (int x = 0; x < result[settings.render_mode].size(); x++) {
                for (int y = 0; y < result[settings.render_mode][x].size(); y++) {
                    XSetForeground(display, gc, result[settings.render_mode][x][y]);
                    XFillRectangle(display, window, gc, x * settings.resolution_decrease, y * settings.resolution_decrease, settings.resolution_decrease, settings.resolution_decrease);
                }
            }
            renderInfo();
        }
    }
    
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

// MARK: - Helper functions
string formatTime(int milliseconds) {
    time_t seconds = milliseconds / 1000;
    milliseconds -= seconds * 1000;
    
    struct tm result;
    gmtime_r(&seconds, &result);
    
    stringstream ss;
    ss << put_time(&result, "%T") << '.' << setfill('0') << setw(3) << milliseconds;
    return ss.str();
}

Color getPixel(Intersection data, int mode) {
    switch (mode) {
        case RENDER_COLOR: return data.texture;
        case RENDER_REFLECTION: return data.reflection;
        case RENDER_TRANSMISSION: return data.transmission;
        case RENDER_LIGHT: return data.hit ? data.light : Color::Black;
        case RENDER_SHADOWS: return data.hit ? (any_of(data.shadows.begin(), data.shadows.end(), [](bool b) { return b; }) ? Color{255, 0, 0} : data.light) : Color::Black;
        case RENDER_NORMALS: return data.hit ? data.normal.toColor() : Color::Black;
        case RENDER_INORMALS: return data.hit ? -data.normal.toColor() : Color::Black;
        case RENDER_DEPTH: return Color::White * (1 - data.distance / settings.max_render_distance);
        case RENDER_UNIQUE: { if (data.id < 0) return Color::Black;
            float hh, ff, p, q, t, v = 0.7, s = 1;
            
            hh = data.id * 6;
            ff = hh - floor(hh);
            p = v * (1.0 - s);
            q = v * (1.0 - (s * ff));
            t = v * (1.0 - (s * (1.0 - ff)));
            
            switch ((int)hh) {
                case 0: return {v, t, p};
                case 1: return {q, v, p};
                case 2: return {p, v, t};
                case 3: return {p, q, v};
                case 4: return {t, p, v};
                case 5:
                default: return {v, p, q};
            }
        }
        case RENDER_SHADED:
        default: return data.shaded();
    }
}

// MARK: Draw debug box
void Camera::drawDebugBox(int x, int y, Input mask) {
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


// MARK: - Preprocessing
vector<vector<Intersection>> Camera::preRender(const vector<Shape *> &objects, const vector<Light *> &lights) {
    const int regions_x = ceil((float)width / settings.render_region_size);
    const int regions_y = ceil((float)height / settings.render_region_size);
    
    vector<vector<Intersection>> buffer(regions_x, vector<Intersection>(regions_y));
    if (!settings.preprocess) return buffer;
    
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            auto ray = castRay(position, getCameraRay((x + 0.5) * settings.render_region_size, (y + 0.5) * settings.render_region_size), objects, lights, {true, 0, true, true, true, true, vector<bool>(objects.size(), true)});
            
            XSetForeground(display, gc, getPixel((buffer[x][y] = ray), settings.render_mode));
            XFillRectangle(display, window, gc, x * settings.resolution_decrease * settings.render_region_size, y * settings.resolution_decrease * settings.render_region_size, settings.resolution_decrease * settings.render_region_size, settings.resolution_decrease * settings.render_region_size);
            
            if (!settings.save_render) continue;
            for (auto mode = 0; mode < RenderTypes; mode++)
                for (int dx = 0; dx < settings.render_region_size; dx++)
                    fill(&result[mode][x * settings.render_region_size + dx][y * settings.render_region_size], &result[mode][x * settings.render_region_size + dx][(y + 1) * settings.render_region_size], getPixel(ray, mode));
        }
    }
    
    return buffer;
}

vector<vector<Input>> Camera::processPreRender(const vector<vector<Intersection>> &buffer) {
    const int regions_x = (int)buffer.size();
    const int regions_y = (int)buffer[0].size();
    vector<vector<Input>> processed(regions_x, vector<Input>(regions_y, Input{}));
    
    vector<vector<vector<float>>> depth_nodes = {{{0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}}};
    vector<vector<vector<float>>> edge_nodes = {{{-0.5}, {-1.5}, {-2.5}, {-3.5}, {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5}, {-4.5}, {-5.5}, {-6.5}, {-7.5}}, {{0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}}};
    NeuralNetwork depth_filter(depth_nodes);
    NeuralNetwork edge_filter(edge_nodes);
    
    region_count = 0;
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            vector<float> depth_matrix(9);
            vector<float> reflect_matrix(9);
            vector<float> transmit_matrix(9);
            vector<vector<float>> shadow_matrix(buffer[0][0].shadows.size());
            for (auto it = shadow_matrix.begin(); it != shadow_matrix.end(); it++) it->resize(9);
            
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    auto item = buffer[min(max(x + i - 1, 0), regions_x - 1)][min(max(y + j - 1, 0), regions_y - 1)];
                    auto index = 3 * j + i;
                    
                    depth_matrix[index] = item.hit;

                    if (item.hit && item.object->material.Ks) reflect_matrix[index] = item.reflection;
                    else reflect_matrix[3*i+j] = settings.background_color;
                    
                    if (item.hit && item.object->material.transparent) transmit_matrix[index] = item.transmission;
                    else transmit_matrix[3*i+j] = settings.background_color;
                    
                    for (int k = 0; k < shadow_matrix.size(); k++) shadow_matrix[k][index] = item.shadows[k];
                }
            }
            
            // MARK: Filters
            if (depth_filter.eval(depth_matrix)[0] == 0) processed[x][y].render = false;
            else {
                processed[x][y] = {
                    true, 0, true, true,
                    edge_filter.eval(reflect_matrix)[0] != 0,
                    edge_filter.eval(transmit_matrix)[0] != 0,
                    vector<bool>(buffer[0][0].shadows.size(), true)
                };
                
                for (int i = 0; i < processed[x][y].shadows.size(); i++) processed[x][y].shadows[i] = edge_filter.eval(shadow_matrix[i])[0];
                
                switch (settings.render_mode) {
                    case RENDER_REFLECTION: if (!processed[x][y].reflections) processed[x][y].render = false; processed[x][y].diffuse = processed[x][y].transmission = false; break;
                    case RENDER_TRANSMISSION: if (!processed[x][y].transmission) processed[x][y].render = false; processed[x][y].diffuse = processed[x][y].reflections = false; break;
                    case RENDER_LIGHT:
                    case RENDER_SHADOWS: processed[x][y].reflections = processed[x][y].transmission = false; break;
                    case RENDER_COLOR:
                    case RENDER_NORMALS:
                    case RENDER_DEPTH: processed[x][y].diffuse = processed[x][y].reflections = processed[x][y].transmission = false;
                    case RENDER_SHADED: break;
                }
                
                if (processed[x][y].render) {
                    region_count++;
                    if (settings.show_debug) drawDebugBox(x, y, processed[x][y]);
                }
            }
        }
    }
    
    return processed;
}

// MARK: - Rendering
inline void Camera::drawInfoString(int x, int y, stringstream &ss, Color color) {
    XSetForeground(display, gc, color);
    XDrawString(display, window, gc, x * 6, y * 15, ss.str().c_str(), (int)ss.str().length());
    ss.str("");
}

void Camera::renderInfo() {
    XSetForeground(display, gc, Color::Black);
    XFillRectangle(display, window, gc, 2, 2, 1 + 6 * 28, 4 + 15 * 6);
    
    static vector<string> render_type_names = {"Shaded", "Textures", "Reflections", "Transmission", "Light", "Shadows", "Normals", "Inverse Normals", "Depth", "Objects"};
    if (region_current < region_count) end = chrono::high_resolution_clock::now();
    const float elapsed = chrono::duration<float, milli>(end - start).count();
    stringstream ss;
    
    ss << "Render time: ";
    drawInfoString(1, 1, ss, Color::Green);
    ss << formatTime(elapsed);
    drawInfoString(14, 1, ss, Color::Yellow);
    
    ss << "Region: " << region_current << "/" << region_count << " @ " << settings.render_region_size << " px";
    drawInfoString(1, 2, ss, Color::Green);
    
    ss << "Prog:";
    drawInfoString(1, 3, ss, Color::Green);
    XSetForeground(display, gc, Color::Gray.dark());
    XFillRectangle(display, window, gc, 42, 35, 6 * 15, 12);
    const auto progress = 6 * 15.f * region_current / region_count;
    const auto total = accumulate(timer.times.begin(), timer.times.end(), 0.f);
    float current = 0;
    for (short i = 0; i < timer.c; i++) {
        XSetForeground(display, gc, timer.colors[i]);
        const auto temp = round(progress * timer.times[i] / total);
        XFillRectangle(display, window, gc, current + 42, 35, temp, 12);
        current += temp;
    }
    if (region_current >= region_count) ss << "100%";
    else ss << fixed << setprecision(1) << min(100.f * region_current / region_count, 99.9f) << defaultfloat << "%";
    drawInfoString(23, 3, ss, Color::Orange);
    
    ss << "Quality: " << width << "x" << height << " (0," << settings.max_render_distance << "]";
    drawInfoString(1, 4, ss, Color::Green);
    
    ss << "Complexity: " << info.objects << " x " << settings.max_light_bounces;
    drawInfoString(1, 5, ss, Color::Green);
    
    ss << "Mode: " << settings.render_mode << "/" << RenderTypes - 1 << " ";
    drawInfoString(1, 6, ss, Color::Green);
    ss << " (" << render_type_names[settings.render_mode] << ")";
    drawInfoString(10, 6, ss, Color::Red);
    
    XFlush(display);
}

RenderRegion Camera::renderRegion(const vector<Shape *> &objects, const vector<Light *> &lights, RenderRegion region, Input mask, const Intersection &estimate) {
    for (int x = 0; x < region.w; x++) {
        for (int y = 0; y < region.h; y++) {
            auto ray = castRay(position, getCameraRay(region.x + x, region.y + y), objects, lights, mask);
            
            if (!mask.reflections && ray.hit && ray.object->material.Ks) ray.reflection = estimate.reflection == Color::Black ? settings.background_color : estimate.reflection;
            if (!mask.transmission && ray.hit && ray.object->material.transparent) ray.transmission = estimate.transmission == Color::Black ? settings.background_color : estimate.transmission;
            for (int i = 0; i < mask.shadows.size(); i++) if (!mask.shadows[i] && ray.hit) if ((ray.shadows[i] = estimate.shadows[i])) ray.light = estimate.light;
            
            region.buffer[x][y] = getPixel(ray, settings.render_mode);
            region.timer += ray.timer;
            
            if (!settings.save_render) continue;
            for (auto mode = 0; mode < RenderTypes; mode++) result[mode][region.x + x][region.y + y] = getPixel(ray, mode);
        }
    }
    
    return region;
}

void Camera::render(const vector<Shape *> &objects, const vector<Light *> &lights) {
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == Expose && event.xexpose.count == 0) {
            start = chrono::high_resolution_clock::now();
            
            cout << "Starting render..." << endl;
            
            for (const auto &object : objects) info += object->getInfo();
            if (settings.save_render) result.resize(RenderTypes, vector<vector<Color>>(width, vector<Color>(height, Color::Black)));
            
            // Render at lower resolution
            const auto buffer = preRender(objects, lights);
            XFlush(display);
            
            // Process what to render
            region_count = (int)buffer.size() * (int)buffer[0].size(); 
            const auto mask = settings.preprocess ? processPreRender(buffer) : vector<vector<Input>>(buffer.size(), vector<Input>(buffer[0].size(), Input{true, 0, true, true, true, true, vector<bool>(lights.size(), true)}));
            XFlush(display);
            
            // Create job queue
            ConcurrentQueue<RenderRegion> task_queue;
            do {
                task_queue.push(RenderRegion(minX, maxX, minY, maxY));
//                drawDebugBox(minX / settings.render_region_size, minY / settings.render_region_size, {false, -1});
            } while (next(mask));
            renderInfo();
            
            ConcurrentQueue<RenderRegion> result_queue;
            auto func = [&]() {
                while (true) {
                    RenderRegion task;
                    if (!task_queue.pop(task)) return;
                    int x = task.x / settings.render_region_size, y = task.y / settings.render_region_size;
                    result_queue.push(this->renderRegion(objects, lights, task, mask[x][y], buffer[x][y]));
                }
            };
            
            // Startup sibling threads
            vector<thread> threads(settings.rendering_threads);
            for (auto& it : threads) it = thread(func);
            
            // Use main thread to render results
            do {
                RenderRegion region;
                result_queue.pop(region);
                for (int x = 0; x < region.w; x++) {
                    for (int y = 0; y < region.h; y++) {
                        XSetForeground(display, gc, region.buffer[x][y]);
                        XFillRectangle(display, window, gc, (region.x + x) * settings.resolution_decrease, (region.y + y) * settings.resolution_decrease, settings.resolution_decrease, settings.resolution_decrease);
                    }
                }
                
                timer += region.timer;
                region_current++;
                renderInfo();
            } while (region_current < region_count);
            
            // Terminate sibling threads
            task_queue.stop();
            for (auto &it : threads) it.join();
            
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
            x = floor(this->width / settings.render_region_size / 2) * settings.render_region_size;
            y = floor(this->height / settings.render_region_size / 2) * settings.render_region_size;
            r = i = 0;
            l = 1;
            break;

        case PATTERN_HORIZONTAL:
        case PATTERN_VERTICAL:
        default:
            x = y = 0;
            break;
    }
    
    generateRange();
}

bool Camera::next(const vector<vector<Input>> &mask) {
    switch (settings.render_pattern) {
        case PATTERN_VERTICAL: // MARK: Vertical
            do {
                y += settings.render_region_size;
                if (y >= height) {
                    y = 0;
                    x += settings.render_region_size;
                }
            } while (x < width && !mask[x/settings.render_region_size][y/settings.render_region_size].render);

            generateRange();

            return x < width;

        case PATTERN_SPIRAL: // MARK: Spiral
            do {
                switch (r) {
                    case 0: x -= settings.render_region_size; break;
                    case 1: y += settings.render_region_size; break;
                    case 2: x += settings.render_region_size; break;
                    case 3: y -= settings.render_region_size; break;
                }
                if (++i >= l) {
                    if (++r % 2 == 0) l++;
                    if (r > 3) r = 0;
                    i = 0;
                }

                if (x >= width && y >= height) return false;
            } while (x >= width || x + settings.render_region_size <= 0 || y >= height || y + settings.render_region_size <= 0 || !mask[x/settings.render_region_size][y/settings.render_region_size].render);

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
            } while (y < height && !mask[x/settings.render_region_size][y/settings.render_region_size].render);
            
            generateRange();
            
            return y < height;
    }
}
