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
string formatTime(int milliseconds) {
    time_t seconds = milliseconds / 1000;
    milliseconds -= seconds * 1000;
    
    stringstream ss;
    ss << put_time(gmtime(&seconds), "%T") << '.' << setfill('0') << setw(3) << milliseconds;
    return ss.str();
}

Color getPixel(int x, int y, Intersection data) {
    switch (settings.render_mode) {
        case RENDER_COLOR: return data.texture; break;
        case RENDER_REFLECTION: return data.reflection; break;
        case RENDER_TRANSMISSION: return data.transmission; break;
        case RENDER_LIGHT: return data.hit ? White * data.light : Black; break;
        case RENDER_SHADOWS: return data.hit ? (any_of(data.shadows.begin(), data.shadows.end(), [](bool b) { return b; }) ? Red : White * data.light) : Black; break;
        case RENDER_NORMALS: return data.hit ? data.normal.toColor() : Black; break;
        case RENDER_DEPTH: return White * (1 - data.distance / settings.max_render_distance); break;
        case RENDER_SHADED:
        default: return data.shaded(); break;
    }
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
    
    if (!mask.step_size) {
        XSetForeground(display, gc, Orange);
        for (int i = 0; i < 4; i++) XDrawLines(display, window, gc, points[i], npoints, CoordModeOrigin);
        return;
    }
    
    if (mask.step_size == settings.precise_step_size) XSetForeground(display, gc, DarkRed);
    else if (mask.transmission) XSetForeground(display, gc, Green / 2);
    else XSetForeground(display, gc, Gray);
    XDrawLines(display, window, gc, points[0], npoints, CoordModeOrigin);
    
    if (mask.reflections) XSetForeground(display, gc, DarkBlue);
    else if (any_of(mask.shadows.begin(), mask.shadows.end(), [](bool b) { return b; })) XSetForeground(display, gc, Yellow / 2);
    else XSetForeground(display, gc, Gray);
    XDrawLines(display, window, gc, points[1], npoints, CoordModeOrigin);
    
    if (any_of(mask.shadows.begin(), mask.shadows.end(), [](bool b) { return b; })) XSetForeground(display, gc, Yellow / 2);
    else if (mask.reflections) XSetForeground(display, gc, DarkBlue);
    else XSetForeground(display, gc, Gray);
    XDrawLines(display, window, gc, points[2], npoints, CoordModeOrigin);
    
    if (mask.transmission) XSetForeground(display, gc, Green / 2);
    else if (mask.step_size == settings.precise_step_size) XSetForeground(display, gc, DarkRed);
    else XSetForeground(display, gc, Gray);
    XDrawLines(display, window, gc, points[3], npoints, CoordModeOrigin);
}


// MARK: - Preprocessing
vector<vector<Intersection>> Camera::preRender(const vector<Shape *> &objects, const vector<Light *> &lights) {
    const int regions_x = ceil((float)width / settings.render_region_size);
    const int regions_y = ceil((float)height / settings.render_region_size);
    
    vector<vector<Intersection>> buffer(regions_x);
    for (int x = 0; x < regions_x; x++) {
        buffer[x].resize(regions_y);
        for (int y = 0; y < regions_y; y++) {
            auto ray = castRay(position, getCameraRay((x + 0.5) * settings.render_region_size, (y + 0.5) * settings.render_region_size), objects, lights, {settings.probe_step_size, 0, true, true, true, true, vector<bool>(objects.size(), true)});
            
            XSetForeground(display, gc, getPixel(x, y, (buffer[x][y] = ray)));
            XFillRectangle(display, window, gc, x * settings.resolution_decrese * settings.render_region_size, y * settings.resolution_decrese * settings.render_region_size, settings.resolution_decrese * settings.render_region_size, settings.resolution_decrese * settings.render_region_size);
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
    
    vector<const Shape *> ids;
    
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            vector<float> depth_matrix(9);
            vector<float> object_matrix(9);
            vector<float> reflect_matrix(9);
            vector<float> refract_matrix(9);
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
                    
                    if (item.hit && item.object->material.transparent) refract_matrix[index] = item.transmission;
                    else refract_matrix[3*i+j] = settings.background_color;
                    
                    for (int k = 0; k < shadow_matrix.size(); k++) shadow_matrix[k][index] = item.shadows[k];
                }
            }
            
            // MARK: Filters
            if (depth_filter.eval(depth_matrix)[0] == 0) processed[x][y].step_size = false;
            else {
                processed[x][y] = {
                    edge_filter.eval(object_matrix)[0] ? settings.precise_step_size : settings.quick_step_size,
                    0, true, true,
                    processed[x][y].reflections = edge_filter.eval(reflect_matrix)[0],
                    processed[x][y].transmission = edge_filter.eval(refract_matrix)[0],
                    vector<bool>(buffer[0][0].shadows.size(), edge_filter.eval(shadow_matrix[i])[0])
                };
                
                switch (settings.render_mode) {
                    case RENDER_REFLECTION: if (!processed[x][y].reflections) processed[x][y].step_size = false; processed[x][y].diffuse = processed[x][y].transmission = false; break;
                    case RENDER_TRANSMISSION: if (!processed[x][y].transmission) processed[x][y].step_size = false; processed[x][y].diffuse = processed[x][y].reflections = false; break;
                    case RENDER_LIGHT:
                    case RENDER_SHADOWS: processed[x][y].reflections = processed[x][y].transmission = false; break;
                    case RENDER_COLOR:
                    case RENDER_NORMALS:
                    case RENDER_DEPTH: processed[x][y].diffuse = processed[x][y].reflections = processed[x][y].transmission = false;
                    case RENDER_SHADED:
                    default: break;
                }
                
                if (settings.show_debug && processed[x][y].step_size) {
                    region_count++;
                    drawBox(x, y, processed[x][y]);
                }
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
    
    const float elapsed = chrono::duration<float, milli>(chrono::high_resolution_clock::now() - time).count();
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

RenderRegion Camera::renderRegion(const vector<Shape *> &objects, const vector<Light *> &lights, RenderRegion region, Input mask, const Intersection &estimate) {
    for (int x = 0; x < region.w; x++) {
        for (int y = 0; y < region.h; y++) {
            auto ray = castRay(position, getCameraRay(region.x + x, region.y + y), objects, lights, mask);
            
            if (!mask.reflections && ray.hit && ray.object->material.Ks) ray.reflection = estimate.reflection == Black ? settings.background_color : estimate.reflection;
            if (!mask.transmission && ray.hit && ray.object->material.transparent) ray.transmission = estimate.transmission == Black ? settings.background_color : estimate.transmission;
            for (int i = 0; i < mask.shadows.size(); i++) if (!mask.shadows[i] && ray.hit) ray.shadows[i] = estimate.shadows[i];
            
            region.buffer[x][y] = getPixel(x, y, ray);
        }
    }
    return region;
}

void Camera::render(const vector<Shape *> &objects, const vector<Light *> &lights) {
    while (true) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == Expose && event.xexpose.count == 0) {
            time = chrono::high_resolution_clock::now();
            
            // Aproximate regions
            const auto buffer = preRender(objects, lights);
            XFlush(display);
            
            // Process what to render
            const auto mask = processPreRender(buffer);
            XFlush(display);
            
            ConcurrentQueue<RenderRegion> task_queue;
            do {
                task_queue.push(RenderRegion(minX, maxX, minY, maxY));
//                XSetForeground(display, gc, Orange);
//                drawBox(minX / settings.render_region_size, minY / settings.render_region_size, {0});
            } while (next(mask));
            renderInfo();
            XFlush(display);
            
            // Render
            ConcurrentQueue<RenderRegion> result_queue;
            
            auto func = [&]() {
                while (true) {
                    RenderRegion task;
                    if (!task_queue.pop(task)) return;
                    int x = task.x / settings.render_region_size, y = task.y / settings.render_region_size;
                    result_queue.push(renderRegion(objects, lights, task, mask[x][y], buffer[x][y]));
                }
            };
            vector<thread> threads(settings.rendering_threads);
            for (auto& it : threads) it = thread(func);
            do {
                RenderRegion region;
                result_queue.pop(region);
                for (int x = 0; x < region.w; x++) {
                    for (int y = 0; y < region.h; y++) {
                        XSetForeground(display, gc, region.buffer[x][y]);
                        XFillRectangle(display, window, gc, (region.x + x) * settings.resolution_decrese, (region.y + y) * settings.resolution_decrese, settings.resolution_decrese, settings.resolution_decrese);
                    }
                }
                
                region_current++;
                renderInfo();
                XFlush(display);
            } while (region_current < region_count);
            
            task_queue.stop();
            for (auto& it : threads) it.join();
            
//            // Render stats
//            renderInfo();
//            
//            XFlush(display);
            
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

bool Camera::next(const vector<vector<Input>> &mask) {
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
