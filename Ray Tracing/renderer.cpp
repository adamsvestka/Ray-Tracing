//
//  renderer.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "renderer.hpp"

// MARK: Select render_mode
// Funkce, která převede data nasbíraná paprskem na barvu podle vykreslovacího režimu
inline Color getPixel(RayIntersection data, int mode) {
    switch (mode) {
        case RENDER_COLOR: return data.texture;
        case RENDER_REFLECTION: return data.reflection;
        case RENDER_TRANSMISSION: return data.transmission;
        case RENDER_LIGHT: return data.hit ? data.light : Color::Black;
        case RENDER_SHADOWS: return data.hit ? (any_of(data.shadows.begin(), data.shadows.end(), [](bool b) { return b; }) ? Color(255, 0, 0) : data.light) : Color::Black;
        case RENDER_NORMALS: return data.hit ? data.normal.asColor() : Color::Black;
        case RENDER_INORMALS: return data.hit ? -data.normal.asColor() : Color::Black;
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

Renderer::Renderer(NativeInterface &display, Camera &camera, vector<Object *> &objects, vector<Light *> &lights) : display(display), camera(camera), objects(objects), lights(lights) {
    width = height = 0;
}

// MARK: - Preprocessing
// Vyšle jeden paprsek pro každý region, podle toho pak zjistí, kde je zbytečné vykreslovat
vector<vector<RayIntersection>> Renderer::preRender() {
    const int regions_x = ceil((float)width / settings.render_region_size);
    const int regions_y = ceil((float)height / settings.render_region_size);
    
    vector<vector<RayIntersection>> buffer(regions_x, vector<RayIntersection>(regions_y));
    if (!settings.preprocess) return buffer;
    
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            buffer[x][y] = castRay(camera.getPosition(), camera.getRay((x + 0.5) * settings.render_region_size, (y + 0.5) * settings.render_region_size), objects, lights, {true, 0, true, true, true, true, vector<bool>(objects.size(), true)});
            
            for (int dx = 0; dx < settings.render_region_size; dx++) {
                if (settings.save_render) for (auto mode = 0; mode < RenderTypes; mode++) fill(&result[mode][x * settings.render_region_size + dx][y * settings.render_region_size], &result[mode][x * settings.render_region_size + dx][(y + 1) * settings.render_region_size], getPixel(buffer[x][y], mode));
                for (int dy = 0; dy < settings.render_region_size; dy++) display.drawPixel(x * settings.render_region_size + dx, y * settings.render_region_size + dy, getPixel(buffer[x][y], settings.render_mode));
            }
        }
    }
    
    display.refresh();
    return buffer;
}

// Rozhoddne, co vykreslovat podle výsledku předchozí funkce
vector<vector<RayInput>> Renderer::processPreRender(const vector<vector<RayIntersection>> &buffer) {
    const int regions_x = (int)buffer.size();
    const int regions_y = (int)buffer[0].size();
    
    vector<vector<RayInput>> processed(buffer.size(), vector<RayInput>(buffer[0].size(), RayInput{true, 0, true, true, true, true, vector<bool>(lights.size(), true)}));
    if (!settings.preprocess) {
        region_count = regions_x * regions_y;
        return processed;
    } else region_count = 0;
    
    vector<vector<vector<float>>> depth_nodes = {{{0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}, {0.1}}};
    vector<vector<vector<float>>> edge_nodes = {{{-0.5}, {-1.5}, {-2.5}, {-3.5}, {0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5}, {-4.5}, {-5.5}, {-6.5}, {-7.5}}, {{0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}, {0.5}}};
    NeuralNetwork depth_filter(depth_nodes);
    NeuralNetwork edge_filter(edge_nodes);
    
    for (int x = 0; x < regions_x; x++) {
        for (int y = 0; y < regions_y; y++) {
            vector<float> depth_matrix(9);      // Filtr na hloubku
            vector<float> reflect_matrix(9);    // Filtr na odrazy
            vector<float> transmit_matrix(9);   // Filtr na průhlednost
            vector<vector<float>> shadow_matrix(lights.size(), vector<float>(9));   // Filtr na stíny
            
            // Načte nasbíraná data do filtrů
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    auto item = buffer[clamp(x + i - 1, 0, regions_x - 1)][clamp(y + j - 1, 0, regions_y - 1)];
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
            if (depth_filter.eval(depth_matrix)[0] == 0) processed[x][y].render = false;    // Pokud se v tomto regionu ani v okolních regionech nic nenachází, nevykreslovat
            else {
                processed[x][y] = {
                    true, 0, true, true,
                    edge_filter.eval(reflect_matrix)[0] != 0,   // To samé pro odrazy (pokud není v blízkosti reflexní objekt)
                    edge_filter.eval(transmit_matrix)[0] != 0,  // To samé pro průhlednost
                    vector<bool>(lights.size(), true)
                };
                
                for (int i = 0; i < processed[x][y].shadows.size(); i++) processed[x][y].shadows[i] = edge_filter.eval(shadow_matrix[i])[0];    // Nepočítat stíny, pokud není třeba
                
                // Při některých vykreslovacích režimech není třeba sbírat všechny data
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
                    if (settings.show_debug) display.drawDebugBox(x, y, processed[x][y]);
                }
            }
        }
    }
    
    display.refresh();
    return processed;
}

// MARK: - Rendering
// Vypíše dodatečné údaje o vykreslování do levého horního rohu
void Renderer::renderInfo() {
    // Názvy vykreslovacích režimů
    static const vector<string> render_type_names = {"Stinovano", "Textury", "Odrazy", "Pruhlednost", "Svetlo", "Stiny", "Normaly", "Opacne Normaly", "Hloubka", "Objekty"};
    
    // Časování
    if (region_current < region_count) end = chrono::high_resolution_clock::now();
    display.renderInfo({region_current, region_count, (int)chrono::duration<float, milli>(end - start).count(), info.objects, timer, render_type_names[settings.render_mode]});
    
    display.refresh();
}

// Metoda na vykreslení jednoho regionu
RenderRegion Renderer::renderRegion(RenderRegion region, const RayInput &mask, const RayIntersection &estimate) {
    for (int x = 0; x < region.w; x++) {
        for (int y = 0; y < region.h; y++) {
            auto ray = castRay(camera.getPosition(), camera.getRay(region.x + x, region.y + y), objects, lights, mask);
            
            // Pokud není třeba počítat některé údaje, tak je odhadne
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
    
    // Vynulování
    display.getDimensions(width, height);
    camera.getDimensions(width, height);
    
    resetPosition();
    
    for (const auto &object : objects) info += object->getInfo();
    if (settings.save_render) result = vector<Buffer>(RenderTypes, Buffer(width, vector<Color>(height, Color::Black)));
    
    display.log("Spouštění vykreslování...");
    
    // Vykreslení při nižším rozlišení
    const auto buffer = preRender();
    
    // Zpracuje, co se má vykreslit
    const auto mask = processPreRender(buffer);
    
#ifndef __EMSCRIPTEN__
    
    // Vytvoří seznam úloh
    ConcurrentQueue<RenderRegion> task_queue;
    do {
        task_queue.push(RenderRegion(minX, maxX, minY, maxY));
    } while (next(mask));
    renderInfo();
    
    // Vytvoří lambda funkci práce
    ConcurrentQueue<RenderRegion> result_queue;
    auto func = [&]() {
        while (true) {
            RenderRegion task;
            if (!task_queue.pop(task)) return;
            int x = task.x / settings.render_region_size, y = task.y / settings.render_region_size;
            result_queue.push(this->renderRegion(task, mask[x][y], buffer[x][y]));
        }
    };
    
    // Spustí vlákna
    vector<thread> threads(settings.rendering_threads);
    for (auto &it : threads) it = thread(func);
    
    // K vykreslení výsledků se použije hlavní vlákno
    do {
        RenderRegion region;
        result_queue.pop(region);
        for (int x = 0; x < region.w; x++) for (int y = 0; y < region.h; y++) display.drawPixel(region.x + x, region.y + y, region.buffer[x][y]);
        
        timer += region.timer;
        region_current++;
        renderInfo();
    } while (region_current < region_count);
    
    // Ukončí se ostatní vlákna
    task_queue.stop();
    for (auto &it : threads) it.join();
    
#else
    
    // To samé, ale bez multithreadingu (pro WebAssembly)
    auto refresh = chrono::high_resolution_clock::now();
    
    do {
        RenderRegion task(minX, maxX, minY, maxY);
        int x = task.x / settings.render_region_size, y = task.y / settings.render_region_size;
        
        RenderRegion region = renderRegion(task, mask[x][y], buffer[x][y]);
        for (int x = 0; x < region.w; x++) for (int y = 0; y < region.h; y++) display.drawPixel(region.x + x, region.y + y, region.buffer[x][y]);
        
        timer += region.timer;
        region_current++;
        if ((int)chrono::duration<float, milli>(chrono::high_resolution_clock::now() - refresh).count() > 1000) {
            renderInfo();
            refresh = chrono::high_resolution_clock::now();
        }
    } while (next(mask));
    renderInfo();
    
#endif
    
    // Statistika do konzole
    for (short i = 0; i < timer.c; i++) display.log("Výpočet " + timer.names[i] + " trval " + to_string(timer.times[i] / 1000.f) + " sekund");
    display.log("Celkový čas byl " + to_string(chrono::duration<float, milli>(end - start).count() / 1000.f) + " sekund");
}

Buffer Renderer::getResult(short layer) {
    return result[layer];
}

// MARK: - Region management
void Renderer::generateRange() {
    minX = fmax(x, 0);
    maxX = fmin(x + settings.render_region_size, width);
    minY = fmax(y, 0);
    maxY = fmin(y + settings.render_region_size, height);
}

// Nastaví souřadnice prvního regionu podle vykreslovacího vzoru
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
    
    timer = Timer();
    info = ObjectInfo();
    region_current = 0;
    
    generateRange();
}

// Vygeneruje souřadnice následujícího regionu podle vykreslovacího vzoru
bool Renderer::next(const vector<vector<RayInput>> &mask) {
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
