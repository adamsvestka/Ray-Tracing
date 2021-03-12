//
//  interfaces.cpp
//  Ray Tracing
//
//  Created by Adam Svestka on 8/31/20.
//  Copyright © 2020 Adam Svestka. All rights reserved.
//

#include "interfaces.hpp"

void InterfaceTemplate::getDimensions(int &w, int &h) {
    w = width / settings.resolution_decrease;
    h = height / settings.resolution_decrease;
}


// Pomocná funkce na formátování času jako hh:mm:ss.mls
string formatTime(int milliseconds) {
    time_t seconds = milliseconds / 1000;
    milliseconds -= seconds * 1000;

    struct tm result;
    gmtime_r(&seconds, &result);

    stringstream ss;
    ss << put_time(&result, "%T") << '.' << setfill('0') << setw(3) << milliseconds;
    return ss.str();
}

#ifndef __EMSCRIPTEN__

// MARK: - X11Interface
X11Interface::X11Interface() {
    // Inicializace X11
    display = XOpenDisplay(nullptr);
    scr = DefaultScreen(display);
    gc = DefaultGC(display, scr);
    screen = ScreenOfDisplay(display, scr);
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, screen->width, screen->height, 1, BlackPixel(display, scr), WhitePixel(display, scr));

    // Nastavení X11 okna
    XSetStandardProperties(display, window, "Ray Tracing", "XTerm", None, nullptr, 0, nullptr);
    XSelectInput(display, window, ExposureMask | ButtonPressMask | KeyPressMask);

    XClearWindow(display, window);
    XMapRaised(display, window);

    width = screen->width;
    height = screen->height;
    
    // Čekání na spuštění X11 okna
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == Expose && event.xexpose.count == 0) break;
    }
}

X11Interface::~X11Interface() {
    // Zavření X11 okna
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

void X11Interface::drawPixel(int x, int y, Color c) {
    // Vykreslení pixelu dané barvy na souřadnice x, y
    XSetForeground(display, gc, c);
    XFillRectangle(display, window, gc, x * settings.resolution_decrease, y * settings.resolution_decrease, settings.resolution_decrease, settings.resolution_decrease);
}

void X11Interface::drawDebugBox(int x, int y, RayInput mask) {
    const float size = 0.25;
    static const short region_size = settings.resolution_decrease * settings.render_region_size;
    
    // Body pro vykreslení terče
    XPoint points[4][3] = {{
        {(short)((x + size) * region_size), (short)(y * region_size)},
        {(short)(x * region_size), (short)(y * region_size)},
        {(short)(x * region_size), (short)((y + size) * region_size)}
    }, {
        {(short)((x + 1 - size) * region_size - 2), (short)(y * region_size)},
        {(short)((x + 1) * region_size - 2), (short)(y * region_size)},
        {(short)((x + 1) * region_size - 2), (short)((y + size) * region_size)}
    }, {
        {(short)((x + size) * region_size), (short)((y + 1) * region_size - 2)},
        {(short)(x * region_size), (short)((y + 1) * region_size - 2)},
        {(short)(x * region_size), (short)((y + 1 - size) * region_size - 2)}
    }, {
        {(short)((x + 1 - size) * region_size - 2), (short)((y + 1) * region_size - 2)},
        {(short)((x + 1) * region_size - 2), (short)((y + 1) * region_size - 2)},
        {(short)((x + 1) * region_size - 2), (short)((y + 1 - size) * region_size - 2)}
    }};
    
    const short npoints = sizeof(points[0]) / sizeof(XPoint);
    
    // Různé kombinace barev terče pro různé vykreslovací situace
    if (mask.bounce_count < 0) {
        XSetForeground(display, gc, Color::Orange);
        for (auto &point : points) XDrawLines(display, window, gc, point, npoints, CoordModeOrigin);
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

// Pomocná funkce na vypisování unformací
inline void X11Interface::drawInfoString(int x, int y, stringstream &ss, Color color) {
    XSetForeground(display, gc, color);
    XDrawString(display, window, gc, x * 6, y * 15, ss.str().c_str(), (int)ss.str().length());
    ss.str("");
}

// Funkce na vypisování informací do levého horního rohu
void X11Interface::renderInfo(DebugInfo stats) {
    XSetForeground(display, gc, Color::Black);
    XFillRectangle(display, window, gc, 2, 2, 1 + 6 * 28, 4 + 15 * 6);
    
    stringstream ss;
    
    // Zabraný čas
    ss << "Celkovy cas: ";
    drawInfoString(1, 1, ss, Color::Green);
    ss << formatTime(stats.render_time);
    drawInfoString(14, 1, ss, Color::Yellow);
    
    // Počet vykreslených regionů
    ss << "Region: " << stats.region_current << "/" << stats.region_count << " @ " << settings.render_region_size << " px";
    drawInfoString(1, 2, ss, Color::Green);
    
    // Ukazatel průběhu
    ss << "Prub:";
    drawInfoString(1, 3, ss, Color::Green);
    XSetForeground(display, gc, Color::Gray.dark());
    XFillRectangle(display, window, gc, 42, 35, 6 * 15, 12);
    const auto progress = 6 * 15.f * stats.region_current / stats.region_count;
    const auto total = accumulate(stats.timer.times.begin(), stats.timer.times.end(), 0.f);
    float current = 0;
    for (short i = 0; i < stats.timer.c; i++) {
        const auto temp = round(progress * stats.timer.times[i] / total);
        XSetForeground(display, gc, stats.timer.colors[i]);
        XFillRectangle(display, window, gc, current + 42, 35, temp, 12);
        current += temp;
    }
    if (stats.region_current >= stats.region_count) ss << "100%";
    else ss << fixed << setprecision(1) << min(100.f * stats.region_current / stats.region_count, 99.9f) << defaultfloat << "%";
    drawInfoString(23, 3, ss, Color::Orange);
    
    // Rozlišení obrazovky
    ss << "Kvalita: " << width / settings.resolution_decrease << "x" << height / settings.resolution_decrease << " (0," << settings.max_render_distance << "]";
    drawInfoString(1, 4, ss, Color::Green);
    
    // Složitost scény
    ss << "Slozitost: " << stats.object_count << " x " << settings.max_light_bounces;
    drawInfoString(1, 5, ss, Color::Green);
    
    // Vykreslovací režim
    ss << "Mod: " << settings.render_mode << "/" << RenderTypes - 1 << " ";
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
    ofstream ofile(filename, ios::out | ios::app);
    
    if (ofile.is_open()) {
        ofile << buffer.rdbuf();
        ofile.close();
        return true;
    }
    
    return false;
}

bool X11Interface::loadImage(string filename, Buffer &buffer) {
    CImg<unsigned char> image;
    bool success = true;
    
    try {
        image.load(filename.c_str());
    } catch(...) {
        // Pokud žádaný obrázek neexistuje, tak se vygeneruje šablona
        image = CImg<unsigned char>(64, 64, 1, 3);
        
        cimg_forXYC(image, x, y, c) { image(x, y, c) = (x / 8 % 2) != (y / 8 % 2) ? Color::Black[c] : Color::Magenta[c]; }
        image.draw_text(16, 8, "Image", Color::White.cimg().data(), 0, 1, 13);
        image.draw_text(24, 24, "not", Color::White.cimg().data(), 0, 1, 13);
        image.draw_text(16, 40, "found", Color::White.cimg().data(), 0, 1, 13);
        
        success = false;
    }
    
    buffer.resize(image.width(), vector<Color>(image.height(), Color::Black));
    cimg_forXY(image, x, y) { buffer[x][y] = { image(x, y, 0), image(x, y, 1), image(x, y, 2) }; }
    
    return success;
}

bool X11Interface::saveImage(string filename, const Buffer &buffer) {
    CImg<unsigned char> image((int)buffer.size(), (int)buffer[0].size(), 1, 3);
    
    try {
        cimg_forXYC(image, x, y, c) { image(x, y, c) = buffer[x][y][c]; }
        
        image.save(filename.c_str());
        
        return true;
    } catch(...) {}
    
    return false;
}

void X11Interface::log(const string &message) {
    cout << message << endl;
}

#else

// MARK: - WASMInterface
// Pomocné funkce pro snímání stisků kláves z HTML
EM_BOOL WASMInterface::key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
    WASMInterface *interface = (WASMInterface *)userData;
    interface->push_key(e->keyCode);
    
    return 1;
}

void WASMInterface::push_key(int code) {
    keys.call<void>("push", code);
}

WASMInterface::WASMInterface() {
    // Inicializace HTML
    window = val::global("window");
    document = val::global("document");
    
    float ratio = window["devicePixelRatio"].as<float>();
    width = window["innerWidth"].as<int>() * ratio;
    height = window["innerHeight"].as<int>() * ratio;
    
    canvas = document.call<val>("createElement", string("canvas"));
    canvas.set("width", width);
    canvas.set("height", height);
    canvas["style"].set("width", to_string(width / ratio) + "px");
    canvas["style"].set("height", to_string(height / ratio) + "px");
    document["body"].call<void>("append", canvas);
    
    context = canvas.call<val>("getContext", string("2d"));
    context.set("font", string("10px monospace"));
    
    // Slouží na zobrazení nejnovější zprávy z logu
    hud = document.call<val>("createElement", string("span"));
    hud.set("id", string("console_display"));
    document["body"].call<void>("append", hud);
    
    // Snímání kláves
    keys = val::array();
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WASMInterface::key_callback);
}

WASMInterface::~WASMInterface() {
//    document.call<void>("removeChild", canvas);
}

void WASMInterface::drawPixel(int x, int y, Color c) {
    // Vykreslení pixelu dané barvy na souřadnice x, y
    context.set("fillStyle", c.css());
    context.call<void>("fillRect", x * settings.resolution_decrease, y * settings.resolution_decrease, settings.resolution_decrease, settings.resolution_decrease);
}

// Pomocná funkce na kreslení rohu terče
inline void WASMInterface::drawBoxCorner(vector<array<short, 2>> points) {
    context.call<void>("moveTo", points[0][0], points[0][1]);
    for (int i = 1; i < points.size(); i++) context.call<void>("lineTo", points[i][0], points[i][1]);
    context.call<void>("stroke");
}

void WASMInterface::drawDebugBox(int x, int y, RayInput mask) {
    const float size = 0.25;
    static const short region_size = settings.resolution_decrease * settings.render_region_size;
    
    // Body pro vykreslení terče
    vector<vector<array<short, 2>>> points = {{
        {(short)((x + size) * region_size + 1), (short)(y * region_size + 1)},
        {(short)(x * region_size + 1), (short)(y * region_size + 1)},
        {(short)(x * region_size + 1), (short)((y + size) * region_size + 1)}
    }, {
        {(short)((x + 1 - size) * region_size - 2), (short)(y * region_size + 1)},
        {(short)((x + 1) * region_size - 2), (short)(y * region_size + 1)},
        {(short)((x + 1) * region_size - 2), (short)((y + size) * region_size + 1)}
    }, {
        {(short)((x + size) * region_size + 1), (short)((y + 1) * region_size - 2)},
        {(short)(x * region_size + 1), (short)((y + 1) * region_size - 2)},
        {(short)(x * region_size + 1), (short)((y + 1 - size) * region_size - 2)}
    }, {
        {(short)((x + 1 - size) * region_size - 2), (short)((y + 1) * region_size - 2)},
        {(short)((x + 1) * region_size - 2), (short)((y + 1) * region_size - 2)},
        {(short)((x + 1) * region_size - 2), (short)((y + 1 - size) * region_size - 2)}
    }};

    context.call<void>("beginPath");
    context.set("lineWidth", 1);
    
    // Různé kombinace barev terče pro různé vykreslovací situace
    if (mask.bounce_count < 0) {
        context.set("strokeStyle", Color::Orange.css());
        for (const auto &point : points) drawBoxCorner(point);
        return;
    }
    
    if (mask.transmission) context.set("strokeStyle", Color::Green.css());
    else context.set("strokeStyle", Color::Gray.dark().css());
    drawBoxCorner(points[0]);
    
    if (mask.reflections) context.set("strokeStyle", Color::Blue.css());
    else if (any_of(mask.shadows.begin(), mask.shadows.end(), [](bool b) { return b; })) context.set("strokeStyle", Color::Yellow.css());
    else context.set("strokeStyle", Color::Gray.dark().css());
    drawBoxCorner(points[1]);
    
    if (any_of(mask.shadows.begin(), mask.shadows.end(), [](bool b) { return b; })) context.set("strokeStyle", Color::Yellow.css());
    else if (mask.reflections) context.set("strokeStyle", Color::Blue.css());
    else context.set("strokeStyle", Color::Gray.dark().css());
    drawBoxCorner(points[2]);
    
    if (mask.transmission) context.set("strokeStyle", Color::Green.css());
    else context.set("strokeStyle", Color::Gray.dark().css());
    drawBoxCorner(points[3]);
}

// Pomocná funkce na vypisování unformací
inline void WASMInterface::drawInfoString(int x, int y, stringstream &ss, Color color) {
    context.set("fillStyle", color.css());
    context.call<void>("fillText", ss.str(), x * 6, y * 15);
    ss.str("");
}

// Funkce na vypisování informací do levého horního rohu
void WASMInterface::renderInfo(DebugInfo stats) {
    context.set("fillStyle", Color::Black.css());
    context.call<void>("fillRect", 2, 2, 1 + 6 * 28, 4 + 15 * 6);
    
    stringstream ss;
    
    // Zabraný čas
    ss << "Celkový čas: ";
    drawInfoString(1, 1, ss, Color::Green);
    ss << formatTime(stats.render_time);
    drawInfoString(14, 1, ss, Color::Yellow);
    
    // Počet vykreslených regionů
    ss << "Region: " << stats.region_current << "/" << stats.region_count << " @ " << settings.render_region_size << " px";
    drawInfoString(1, 2, ss, Color::Green);
    
    // Ukazatel průběhu
    ss << "Průb:";
    drawInfoString(1, 3, ss, Color::Green);
    context.set("fillStyle", Color::Gray.dark().css());
    context.call<void>("fillRect", 42, 35, 6 * 15, 12);
    const auto progress = 6 * 15.f * stats.region_current / stats.region_count;
    const auto total = accumulate(stats.timer.times.begin(), stats.timer.times.end(), 0.f);
    float current = 0;
    for (short i = 0; i < stats.timer.c; i++) {
        const auto temp = round(progress * stats.timer.times[i] / total);
        context.set("fillStyle", stats.timer.colors[i].css());
        context.call<void>("fillRect", current + 42, 35, temp, 12);
        current += temp;
    }
    if (stats.region_current >= stats.region_count) ss << "100%";
    else ss << fixed << setprecision(1) << min(100.f * stats.region_current / stats.region_count, 99.9f) << defaultfloat << "%";
    drawInfoString(23, 3, ss, Color::Orange);
    
    // Rozlišení obrazovky
    ss << "Kvalita: " << width / settings.resolution_decrease << "x" << height / settings.resolution_decrease << " (0," << settings.max_render_distance << "]";
    drawInfoString(1, 4, ss, Color::Green);
    
    // Složitost scény
    ss << "Složitost: " << stats.object_count << " x " << settings.max_light_bounces;
    drawInfoString(1, 5, ss, Color::Green);
    
    // Vykreslovací režim
    ss << "Mód: " << settings.render_mode << "/" << RenderTypes - 1 << " ";
    drawInfoString(1, 6, ss, Color::Green);
    ss << " (" << stats.render_mode_name << ")";
    drawInfoString(10, 6, ss, Color::Red);
    
    refresh();
}

// WebAssembly vzdá kontrolu JavaScriptu na jeden cyklus
void WASMInterface::refresh() {
    emscripten_sleep(0);
}

char WASMInterface::getChar() {
    val ch = val::undefined();
    do {
        refresh();
        ch = keys.call<val>("shift");
    } while (!ch.as<bool>());   // Čeká na to až se v JS poli 'keys' objeví hodnota
    
    return ch.as<int>();
}

bool WASMInterface::loadFile(string filename, stringstream &buffer) {
    val temp = window.call<val>("fetch", filename).await();
    string result = temp.call<val>("text").await().as<string>();
    
    buffer.str(result);
    
    return buffer.str()[0] != '!';
}

// WebAssembly verze nepodporuje ukládání souborů
bool WASMInterface::saveFile(string, const stringstream &) {
    return false;
}

// Pomocné funkce na načítání obrázků z HTML
val WASMInterface::image = val::undefined();
void WASMInterface::init_image(val resolve, val reject) {
    WASMInterface::image.set("onload", resolve);
}

EMSCRIPTEN_BINDINGS(events) {
    emscripten::function("init_image", &WASMInterface::init_image);
}

bool WASMInterface::loadImage(string filename, Buffer &buffer) {
    // Na <img> elementu se nastaví src atribut
    image = val::global("Image").new_();
    image.set("src", filename);
    val::global("Promise").new_(val::module_property("init_image")).await();
    int width = image["width"].as<int>();
    int height = image["height"].as<int>();
    
    // Vytvoří se <canvas> a na jeho context se vykreslí obrázek z <img>
    val canvas = document.call<val>("createElement", string("canvas"));
    canvas.set("width", width);
    canvas.set("height", height);
    
    val context = canvas.call<val>("getContext", string("2d"));
    context.call<void>("drawImage", image, 0, 0);
    
    // Z contextu se dostane 1D bitmapa
    val imageData = context.call<val>("getImageData", 0, 0, width, height);
    vector<int> arrayBuffer = vecFromJSArray<int>(imageData["data"]);
    
    buffer.resize(width, vector<Color>(height));
    
    // 1D bitmapa se překopíruje do očekávaného 2D formátu
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            int i = (y * height + x) * 4;
            buffer[x][y] = Color(arrayBuffer[i], arrayBuffer[i + 1], arrayBuffer[i + 2]);
        }
    }
    
    return true;
}

// WebAssembly verze nepodporuje ukládání obrázků
bool WASMInterface::saveImage(string, const Buffer &) {
    return false;
}

void WASMInterface::log(const string &message) {
    emscripten_console_log(message.c_str());
    hud.set("innerText", message);
    refresh();
}

#endif
