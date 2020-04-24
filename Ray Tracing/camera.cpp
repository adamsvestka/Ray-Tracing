//
//  camera.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 4/14/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "camera.hpp"

Camera::Camera(Vector3 position) {
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
    fovFactor = 1 / tan(settings.fov / 2);
    region_count = 0;
    region_current = 0;
    
    switch (settings.render_pattern) {
        case 1:
            x = round(this->width / settings.region_size / 2) * settings.region_size;
            y = round(this->height / settings.region_size / 2) * settings.region_size;
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
        case RENDER_SHADED: XSetForeground(display, gc, data.shaded); break;
        case RENDER_COLOR: XSetForeground(display, gc, data.color); break;
        case RENDER_LIGHT: XSetForeground(display, gc, data.hit ? data.light : Black); break;
        case RENDER_SHADOWS: XSetForeground(display, gc, data.hit ? (data.shadow ? Red : data.light) : Black); break;
        case RENDER_NORMALS: XSetForeground(display, gc, data.hit ? data.normal.toColor() : Black); break;
        case RENDER_DEPTH: XSetForeground(display, gc, White * (1 - data.distance / settings.render_distance)); break;
        default: XSetForeground(display, gc, data.shaded); break;
    }
    
    XFillRectangle(display, window, gc, x * settings.resolution * s, y * settings.resolution * s, settings.resolution * s, settings.resolution * s);
}

// MARK: - Preprocessing
vector<vector<float>> Camera::preRender(vector<Shape*> objects, vector<Light> lights) {
    const int regions_x = ceil((float)width / settings.region_size);
    const int regions_y = ceil((float)height / settings.region_size);
    
    const auto temp = settings.step_size;
    settings.step_size = settings.probe_step_size;
    
    vector<vector<float>> buffer(regions_x);
    for (int x = 0; x < regions_x; x++) {
        buffer[x].resize(regions_y);
        for (int y = 0; y < regions_y; y++) {
            auto ray = castRay(position, getCameraRay((x + 0.5) * settings.region_size, (y + 0.5) * settings.region_size), objects, lights);
            
            buffer[x][y] = 1 - ray.distance / settings.render_distance;
            
            drawPixel(x, y, settings.region_size, ray);
        }
    }
    settings.step_size = temp;
    
    return buffer;
}

vector<vector<bool>> Camera::processPreRender(vector<vector<float>> buffer) {
    const int regions_x = (int)buffer.size();
    const int regions_y = (int)buffer[0].size();
    vector<vector<bool>> processed(regions_x);
    
    vector<vector<float>> nodes = {{0, 1, 0}, {1, 1, 1}, {0, 1, 0}};
    NeuralNetwork nn(nodes);
    
    for (int x = 0; x < regions_x; x++) {
        processed[x].resize(regions_y);
        for (int y = 0; y < regions_y; y++) {
            vector<vector<float>> matrixDepth(3);
            for (int i = 0; i < 3; i++) {
                matrixDepth[i].resize(3);
                for (int j = 0; j < 3; j++) matrixDepth[i][j] = buffer[min(max(x + i - 1, 0), regions_x - 1)][min(max(y + j - 1, 0), regions_y - 1)];
            }
            
            const float d = abs(nn.eval(matrixDepth));
            if (d == 0) processed[x][y] = false;
            else {
                processed[x][y] = true;
                region_count++;

                XSetForeground(display, gc, Gray);
                const auto size = 0.3;
                
                XPoint points[4][3] = {{
                    {(short)((x * settings.resolution + size) * settings.region_size), (short)(y * settings.resolution * settings.region_size)},
                    {(short)(x * settings.resolution * settings.region_size), (short)(y * settings.resolution * settings.region_size)},
                    {(short)(x * settings.resolution * settings.region_size), (short)((y * settings.resolution + size) * settings.region_size)}
                }, {
                    {(short)(((x + 1) * settings.resolution - size) * (settings.region_size - 1.f/x)), (short)(y * settings.resolution * settings.region_size)},
                    {(short)((x + 1) * settings.resolution * (settings.region_size - 1.f/x)), (short)(y * settings.resolution * settings.region_size)},
                    {(short)((x + 1) * settings.resolution * (settings.region_size - 1.f/x)), (short)((y * settings.resolution + size) * settings.region_size)}
                }, {
                    {(short)((x * settings.resolution + size) * settings.region_size), (short)((y + 1) * settings.resolution * (settings.region_size - 1.f/y))},
                    {(short)(x * settings.resolution * settings.region_size), (short)((y + 1) * settings.resolution * (settings.region_size - 1.f/y))},
                    {(short)(x * settings.resolution * settings.region_size), (short)(((y + 1) * settings.resolution - size) * (settings.region_size - 1.f/y))}
                }, {
                    {(short)(((x + 1) * settings.resolution - size) * (settings.region_size - 1.f/x)), (short)((y + 1) * settings.resolution * (settings.region_size - 1.f/y))},
                    {(short)((x + 1) * settings.resolution * (settings.region_size - 1.f/x)), (short)((y + 1) * settings.resolution * (settings.region_size - 1.f/y))},
                    {(short)((x + 1) * settings.resolution * (settings.region_size - 1.f/x)), (short)(((y + 1) * settings.resolution - size) * (settings.region_size - 1.f/y))}
                }};
                
                int npoints = sizeof(points[0]) / sizeof(XPoint);
                for (int i = 0; i < 4; i++) XDrawLines(display, window, gc, points[i], npoints, CoordModeOrigin);
            }
        }
    }
    
    return processed;
}

// MARK: - Rendering
void Camera::renderInfo() {
    XSetForeground(display, gc, Black);
    XFillRectangle(display, window, gc, 2, 2, 158, 48);
    
    XSetForeground(display, gc, Green);
    
    const float elapsed = (float)(clock() - time) / CLOCKS_PER_SEC;
    string time = "Render time: " + formatTime(elapsed);
    XDrawString(display, window, gc, 5, 15, time.c_str(), (int)time.length());

    string str = "Regions: " + to_string(region_current) + "/" + to_string(region_count) + " @ " + to_string(settings.region_size) + " px";
    XDrawString(display, window, gc, 5, 30, str.c_str(), (int)str.length());
    
    string time2 = "ETC: " + formatTime(elapsed / region_current * region_count - elapsed);
    XDrawString(display, window, gc, 5, 45, time2.c_str(), (int)time2.length());
}

void Camera::renderRegions(vector<Shape *> objects, vector<Light> lights, vector<vector<bool>> mask) {
    do {
        if (!mask[minX()/settings.region_size][minY()/settings.region_size]) continue;
        region_current++;
        
        for (int x = minX(); x < maxX(); x++) {
            for (int y = minY(); y < maxY(); y++) {
                auto ray = castRay(position, getCameraRay(x, y), objects, lights);
                
                drawPixel(x, y, 1, ray);
            }
        }
        
        renderInfo();
    } while (next());
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
            renderRegions(objects, lights, mask);
            
            // Render stats
            renderInfo();
            
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

bool Camera::next() {
    switch (settings.render_pattern) {
        case PATTERN_SPIRAL: // MARK: Spiral
            do {
                switch (r) {
                    case 0: x += settings.region_size; break;
                    case 1: y -= settings.region_size; break;
                    case 2: x -= settings.region_size; break;
                    case 3: y += settings.region_size; break;
                }
                if (++i >= l) {
                    if (++r % 2 == 0) l++;
                    if (r > 3) r = 0;
                    i = 0;
                }
                
                if (x >= width && y >= height) return false;
            } while (x >= width || x + settings.region_size <= 0 || y >= height || y + settings.region_size <= 0);
            
            return true;
            
        default: // MARK: Default
            x += settings.region_size;
            if (x >= width) {
                x = 0;
                y += settings.region_size;
            }
            
            return y < height;
    }
}

int Camera::minX() { return fmax(x, 0); };
int Camera::maxX() { return fmin(x + settings.region_size, width); };
int Camera::minY() { return fmax(y, 0); };
int Camera::maxY() { return fmin(y + settings.region_size, height); };
