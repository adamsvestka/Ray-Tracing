//
//  renderer.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "renderer.hpp"

// MARK: Select render_mode
inline Color getPixel(Intersection data, int mode) {
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

Renderer::Renderer(NativeInterface *display, Vector3 position, Vector3 angles, vector<Shape *> objects, vector<Light *> lights) : objects(objects), lights(lights), display(display) {
    display->getDimensions(width, height);
    camera = Camera(position, angles, width, height);
    resetPosition();
}

// MARK: - Preprocessing
vector<vector<Intersection>> Renderer::preRender() {
    const int regions_x = ceil((float)width / settings.render_region_size);
    const int regions_y = ceil((float)height / settings.render_region_size);
    
    vector<vector<Intersection>> buffer(regions_x, vector<Intersection>(regions_y));
    if (!settings.preprocess) return buffer;
    
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            buffer[x][y] = castRay(camera.getPosition(), camera.getRay((x + 0.5) * settings.render_region_size, (y + 0.5) * settings.render_region_size), objects, lights, {true, 0, true, true, true, true, vector<bool>(objects.size(), true)});
            
            for (int dx = 0; dx < settings.render_region_size; dx++) {
                if (settings.save_render) for (auto mode = 0; mode < RenderTypes; mode++) fill(&result[mode][x * settings.render_region_size + dx][y * settings.render_region_size], &result[mode][x * settings.render_region_size + dx][(y + 1) * settings.render_region_size], getPixel(buffer[x][y], mode));
                for (int dy = 0; dy < settings.render_region_size; dy++) display->drawPixel(x * settings.render_region_size + dx, y * settings.render_region_size + dy, getPixel(buffer[x][y], settings.render_mode));
            }
        }
    }
    
    display->refresh();
    return buffer;
}

vector<vector<Input>> Renderer::processPreRender(const vector<vector<Intersection>> &buffer) {
    const int regions_x = (int)buffer.size();
    const int regions_y = (int)buffer[0].size();
    
    vector<vector<Input>> processed(buffer.size(), vector<Input>(buffer[0].size(), Input{true, 0, true, true, true, true, vector<bool>(lights.size(), true)}));
    if (!settings.preprocess) {
        region_count = regions_x * regions_y;
        return processed;
    } else region_count = 1;
    
    vector<vector<vector<float>>> depth_nodes = {{{0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}}};
    vector<vector<vector<float>>> edge_nodes = {{{-0.5}, {-1.5}, {-2.5}, {-3.5}, {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5}, {-4.5}, {-5.5}, {-6.5}, {-7.5}}, {{0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}}};
    NeuralNetwork depth_filter(depth_nodes);
    NeuralNetwork edge_filter(edge_nodes);
    
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            vector<float> depth_matrix(9);
            vector<float> reflect_matrix(9);
            vector<float> transmit_matrix(9);
            vector<vector<float>> shadow_matrix(lights.size(), vector<float>(9));
            
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
                    vector<bool>(lights.size(), true)
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
                    if (settings.show_debug) display->drawDebugBox(x, y, processed[x][y]);
                }
            }
        }
    }
    
    display->refresh();
    return processed;
}

// MARK: - Rendering
void Renderer::renderInfo() {
    static const vector<string> render_type_names = {"Shaded", "Textures", "Reflections", "Transmission", "Light", "Shadows", "Normals", "Inverse Normals", "Depth", "Objects"};
    
    if (region_current < region_count) end = chrono::high_resolution_clock::now();
    display->renderInfo({region_current, region_count, (int)chrono::duration<float, milli>(end - start).count(), info.objects, timer, render_type_names[settings.render_mode]});
    
    display->refresh();
}

RenderRegion Renderer::renderRegion(RenderRegion region, const Input &mask, const Intersection &estimate) {
    for (int x = 0; x < region.w; x++) {
        for (int y = 0; y < region.h; y++) {
            auto ray = castRay(camera.getPosition(), camera.getRay(region.x + x, region.y + y), objects, lights, mask);
            
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

// MARK: Main loop
void Renderer::render() {
    start = chrono::high_resolution_clock::now();
    
    display->log("Starting render...");
    
    for (const auto &object : objects) info += object->getInfo();
    if (settings.save_render) result.resize(RenderTypes, vector<vector<Color>>(width, vector<Color>(height, Color::Black)));
    
    // Render at lower resolution
    const auto buffer = preRender();
    
    // Process what to render
    const auto mask = processPreRender(buffer);
    
    // Create job queue
    ConcurrentQueue<RenderRegion> task_queue;
    do {
        task_queue.push(RenderRegion(minX, maxX, minY, maxY));
    } while (next(mask));
    renderInfo();
    
    // Create job lambda
    ConcurrentQueue<RenderRegion> result_queue;
    auto func = [&]() {
        while (true) {
            RenderRegion task;
            if (!task_queue.pop(task)) return;
            int x = task.x / settings.render_region_size, y = task.y / settings.render_region_size;
            result_queue.push(this->renderRegion(task, mask[x][y], buffer[x][y]));
        }
    };
    
    // Startup sibling threads
    vector<thread> threads(settings.rendering_threads);
    for (auto &it : threads) it = thread(func);
    
    // Use main thread to render results
    do {
        RenderRegion region;
        result_queue.pop(region);
        for (int x = 0; x < region.w; x++) for (int y = 0; y < region.h; y++) display->drawPixel(region.x + x, region.y + y, region.buffer[x][y]);
        
        timer += region.timer;
        region_current++;
        renderInfo();
    } while (region_current < region_count);
    
    // Terminate sibling threads
    task_queue.stop();
    for (auto &it : threads) it.join();
    
    for (short i = 0; i < timer.c; i++) display->log("Calculating " + timer.names[i] + " took " + timer.times[i] / 1000.f + " seconds");
    display->log("Total time was " + chrono::duration<float, milli>(end - start).count() / 1000.f + " seconds");
}

void Renderer::redraw() {
    for (int x = 0; x < result[settings.render_mode].size(); x++) {
        for (int y = 0; y < result[settings.render_mode][x].size(); y++) {
            display->drawPixel(x, y, result[settings.render_mode][x][y]);
        }
    }
    
    renderInfo();
}

// MARK: - Region management
void Renderer::generateRange() {
    minX = fmax(x, 0);
    maxX = fmin(x + settings.render_region_size, width);
    minY = fmax(y, 0);
    maxY = fmin(y + settings.render_region_size, height);
}

void Renderer::resetPosition() {
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
    
    region_current = 0;
    generateRange();
}

bool Renderer::next(const vector<vector<Input>> &mask) {
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
