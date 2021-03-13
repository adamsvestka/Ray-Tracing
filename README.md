# Ray Tracing
[![platform](https://img.shields.io/badge/platform-macOS-lightgray)](https://www.apple.com/macos)
[![framework](https://img.shields.io/badge/framework-X11-brightgreen)](https://www.xquartz.org)
[![CodeFactor](https://www.codefactor.io/repository/github/adamsvestka/ray-tracing/badge)](https://www.codefactor.io/repository/github/adamsvestka/ray-tracing)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ce7c6be071ec40078c3f88480471cb47)](https://www.codacy.com/manual/svestka.adam1/Ray-Tracing?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=adamsvestka/Ray-Tracing&amp;utm_campaign=Badge_Grade)
[![GitHub milestones](https://img.shields.io/github/milestones/progress-percent/adamsvestka/Ray-Tracing/2)](https://github.com/adamsvestka/Ray-Tracing/milestone/2)
[![GitHub issues](https://img.shields.io/github/issues/adamsvestka/Ray-Tracing)](https://github.com/adamsvestka/Ray-Tracing/issues)
[![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/adamsvestka/Ray-Tracing)](https://github.com/adamsvestka/Ray-Tracing)
[![GitHub releases](https://img.shields.io/github/v/release/adamsvestka/Ray-Tracing?include_prereleases)](https://github.com/adamsvestka/Ray-Tracing/releases)

My very own ray tracing engine which I built to prove that I could.

Also, I built a WebAssembly version that can run anywhere, on any device and is hosted on my [GitHub Pages](https://adamsvestka.github.io/raytracing).

## Dependencies
- [XQuartz](https://www.xquartz.org)
- [CImg](https://cimg.eu)
- [libpng](http://www.libpng.org)
- [libjpeg](http://libjpeg.sourceforge.net)

## Incorporated libraries
- [nlohman/json](https://github.com/nlohmann/json)

## Installation instructions
1. Download latest release from [GitHub](https://github.com/adamsvestka/Ray-Tracing/releases)
1. Install Homebrew: `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
1. Install dependencies: `brew install libpng libjpeg cimg`
1. Download and install [XQuartz](https://www.xquartz.org)
1. Run the the `Ray Tracing` executable

---

Some data is loaded at runtime from configuration files:

## Settings file

This file includes settings for how the scene is rendered.
- filename: `settings.ini`
- format: `text/ini`

```ini
# comment
[Section]
key=value
```

### List of options

| key                 | description                                                                               | type                                  | default   |
|---------------------|-------------------------------------------------------------------------------------------|---------------------------------------|-----------|
| max_render_distance | Far camera cutoff                                                                         | `int`                                 | `100`     |
| surface_bias        | Collided ray offset to prevent shadow acne                                                | `float`                               | `0.001`   |
| max_light_bounces   | Prevent infinite loops                                                                    | `int`                                 | `5`       |
| render_mode         | What layers to collect from collisions                                                    | `enum (0-7)`                          | `0`       |
| render_pattern      | What pattern to render region in                                                          | `enum (0-2)`                          | `1`       |
| show_debug          | Show tiles over regions specifying what to render; preprocess must be true to take effect | `bool`                                | `true`    |
| preprocess          | Only render what is necessary; !! may result in render issues                             | `bool`                                | `false`   |
| save_render         | Save result to buffer to allow for layer switching afterwards                             | `bool`                                | `true`    |
| resolution_decrease | Divide resolution by                                                                      | `int`                                 | `1`       |
| render_region_size  | Render region size                                                                        | `int`                                 | `10`      |
| rendering_threads   | Amount of threads for rendering                                                           | `int`                                 | `25`      |
| background_color    | Background color to fill empty space                                                      | `Color`<sup>[1](#footnoteColor)</sup> | `x000000` |

## Scene file

This file defines objects and lighting in the scene.
- filename: `scence.json`
- format: `text/json`

```js
{
  "camera": {
    "position": Vector3,
    "rotation": Vector3,
    "fov": int
  },
  "shaders": {
    "name": {
      "type": string,
      ...
    }
  },
  "objects": [
    {
      "type": string,
      ...
      "object-specific-params": number,
      "for-example-position": Vector3,
      ...
      "rotation": Vector3,
      "material": Material
    }
  ],
  "lights": [
    "type": string,
    ...
    "light-specific-params": Vector3,
    ...
    "color": Color,
    "intensity": number
  ]
}
```

### Vector3 format

```js
Vector3: {
  "x": float,
  "y": float,
  "z": float
}
```

### Color format<sup>[1](#footnoteColor)</sup>

```js
Color: string
```

### Material format

```js
Material: {
  "shader": {
    "type": string,
    ...
  },
  "n": float,
  "Ks": float,
  "ior": float,
  "transparent": bool
}
```

### List of object types

| type   | params                                                                                                            |
|--------|-------------------------------------------------------------------------------------------------------------------|
| sphere | position: `Vector3`, diameter: `float`, rotation: `Vector3`, material: `Material`                                 |
| cube   | position: `Vector3`, size: `float`, rotation: `Vector3`, material: `Material`                                     |
| cube-2 | position: `Vector3`, size_x: `float`, size_y: `float`, size_z: `float`, rotation: `Vector3`, material: `Material` |
| cube-3 | corner_min: `Vector3`, corner_max: `Vector3`, rotation: `Vector3`, material: `Material`                           |
| plane  | position: `Vector3`, size_x: `float`, size_y: `float`, rotation: `Vector3`, material: `Material`                  |
| object | name: `string`, position: `Vector3`, scale: `float`, rotation: `Vector3`, material: `Material`                    |

### List of light types

| type        | params                                                 |
|-------------|--------------------------------------------------------|
| point       | position: `Vector3`, color: `Color`, intensity: `int`  |
| linear      | position: `Vector3`, color: `Color`, intensity: `int`  |
| global      | color: `Color`, intensity: `int`                       |
| directional | direction: `Vector3`, color: `Color`, intensity: `int` |

### List of shader types

| type         | params                                                                                                              | info                                                                                                                                                                                        |
|--------------|---------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| color        | value: `Color`                                                                                                      | single color texture                                                                                                                                                                        |
| image        | value: `string`                                                                                                     | use image as texture                                                                                                                                                                        |
| checkerboard | scale: `int`, primary: `Color`, secondary: `Color`                                                                  | generate checkerboard pattern sized `scale`x`scale` of two colors                                                                                                                           |
| bricks       | scale: `int`, ratio: `float`, mortar: `float`, primary: `Color`, secondary: `Color`, tertiary: `Color`, seed: `int` | generate brick pattern `scale`  high, `ratio`=width/height, `mortar`=\[0-1\], bricks interpolate between `primary` and `secondary` colors seeded by `seed`, `tertiary` defines mortar color |
| noise        | scale: `int`, seed: `int`, primary: `Color`                                                                         | generate _Perlin_ noise of `primary` color                                                                                                                                                  |
|              |                                                                                                                     |                                                                                                                                                                                             |
| named        | value: `string`                                                                                                     | References a shader with name `value` defined under `shaders` section                                                                                                                       |
|              |                                                                                                                     |                                                                                                                                                                                             |
| grayscale    | value: `Shader`                                                                                                     | grayscale shader `value`                                                                                                                                                                    |
| negate       | value: `Shader`                                                                                                     | negate shader `value`                                                                                                                                                                       |
| add          | values: `[ Shader ]`                                                                                                | add all shaders in `values`                                                                                                                                                                 |
| multiply     | values: `[ Shader ]`                                                                                                | multiply all shaders in `values`                                                                                                                                                            |
| mix          | values: `[ Shader ]`, weights: `[ float ]`                                                                          | add all shaders in `values` first multiplied by respective numbers in `weights`                                                                                                             |

---

<a name="footnoteColor">1</a>: Accepted color formats:
- `x47B3DB` 6-digit hex code prepended with 'x'
- `gray` lowercase color name

Supported colors:
        ![white](https://via.placeholder.com/10/ffffff/000000?text=+) `white`,
        ![lightGray](https://via.placeholder.com/10/ffffff/000000?text=+) `lightGray`,
        ![gray](https://via.placeholder.com/10/8e8e93/000000?text=+) `gray`,
        ![darkGray](https://via.placeholder.com/10/2f2f31/000000?text=+) `darkGray`,
        ![black](https://via.placeholder.com/10/000000/000000?text=+) `black`,
        ![red](https://via.placeholder.com/10/ff3b30/000000?text=+) `red`,
        ![orange](https://via.placeholder.com/10/ff9500/000000?text=+) `orange`,
        ![yellow](https://via.placeholder.com/10/ffcc00/000000?text=+) `yellow`,
        ![pink](https://via.placeholder.com/10/ff2d55/000000?text=+) `pink`,
        ![brown](https://via.placeholder.com/10/a2845e/000000?text=+) `brown`,
        ![mocha](https://via.placeholder.com/10/945200/000000?text=+) `mocha`,
        ![asparagus](https://via.placeholder.com/10/929000/000000?text=+) `asparagus`,
        ![lime](https://via.placeholder.com/10/8efa00/000000?text=+) `lime`,
        ![green](https://via.placeholder.com/10/28cd41/000000?text=+) `green`,
        ![moss](https://via.placeholder.com/10/009051/000000?text=+) `moss`,
        ![fern](https://via.placeholder.com/10/4f8f00/000000?text=+) `fern`,
        ![blue](https://via.placeholder.com/10/007aff/000000?text=+) `blue`,
        ![cyan](https://via.placeholder.com/10/00fdff/000000?text=+) `cyan`,
        ![magenta](https://via.placeholder.com/10/ff40ff/000000?text=+) `magenta`,
        ![teal](https://via.placeholder.com/10/5ac8fa/000000?text=+) `teal`,
        ![indigo](https://via.placeholder.com/10/5856d6/000000?text=+) `indigo`,
        ![purple](https://via.placeholder.com/10/af52de/000000?text=+) `purple`
