# Ray Tracing
[![platform](https://img.shields.io/badge/platform-macOS-lightgray)](https://www.apple.com/macos)
[![dependencies](https://img.shields.io/badge/dependencies-X11-brightgreen)](https://www.xquartz.org)
[![CodeFactor](https://www.codefactor.io/repository/github/adamsvestka/ray-tracing/badge)](https://www.codefactor.io/repository/github/adamsvestka/ray-tracing)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ce7c6be071ec40078c3f88480471cb47)](https://www.codacy.com/manual/svestka.adam1/Ray-Tracing?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=adamsvestka/Ray-Tracing&amp;utm_campaign=Badge_Grade)
[![GitHub issues](https://img.shields.io/github/issues/adamsvestka/Ray-Tracing)](https://github.com/adamsvestka/Ray-Tracing/issues)
[![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/adamsvestka/Ray-Tracing)](https://github.com/adamsvestka/Ray-Tracing)
[![GitHub releases](https://img.shields.io/github/v/release/adamsvestka/Ray-Tracing?include_prereleases)](https://github.com/adamsvestka/Ray-Tracing/releases)

My very own ray tracing engine which I built to prove that I could.

## Dependencies
- [XQuartz](https://www.xquartz.org)

#

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
| render_mode         | What layers to collect from collisions                                                    | `enum (0-7)`.                         | `0`       |
| render_pattern      | What pattern to render region in                                                          | `enum (0-2)`                          | `1`       |
| show_debug          | Show tiles over regions specifying what to render; preprocess must be true to take effect | `bool`                                | `true`    |
| preprocess          | Only render what is necessary; !! may result in render issues                             | `bool`                                | `false`   |
| save_render         | Save result to buffer to allow for layer switching afterwards                             | `bool`                                | `true`    |
| field_of_view       | Camera FOV                                                                                | `int`                                 | `120`     |
| resolution_decrease | Divide resolution by                                                                      | `int`                                 | `1`       |
| render_region_size  | Render region size                                                                        | `int`                                 | `10`      |
| rendering_threads   | Amount of threads for rendering                                                           | `int`                                 | `25`      |
| ambient_lighting    | Ambient light added on top of all objects                                                 | `Color`<sup>[1](#footnoteColor)</sup> | `x0d0d0d` |
| background_color    | Background color to fill empty space                                                      | `Color`<sup>[1](#footnoteColor)</sup> | `x1a1a1a` |

## Scene file

This file defines objects and lighting in the scene.
- filename: `scence.json`
- format: `text/json`

```js
{
  "objects": [
    {
      "type": String,
      ...
      "object-specific-params": Int,
      "for-example-position": Vector3,
      ...
      "rotation": Vector3,
      "material": Material
    }
  ],
  "lights": [
    "type": String,
    ...
    "light-specific-params": Vector3,
    ...
    "color": Color,
    "intensity": Number
  ]
}
```

### Vector3 format

```js
Vector3: {
  "x": Float,
  "y": Float,
  "z": Float
}
```

### Color format<sup>[1](#footnoteColor)</sup>

```js
Color: String
```

### Material format

```js
Material: {
  "shader": {
    "type": String,
    ...
  },
  "n": Float,
  "Ks": Float,
  "ior": Float,
  "transparent": Bool
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
| color        | color: `Color`                                                                                                      | single color texture                                                                                                                                                                        |
| image        | name: `string`                                                                                                      | use image as texture                                                                                                                                                                        |
| checkerboard | scale: `int`, primary: `Color`, secondary: `Color`                                                                  | generate checkerboard pattern sized `scale`x`scale` of two colors                                                                                                                           |
| bricks       | scale: `int`, ratio: `float`, mortar: `float`, primary: `Color`, secondary: `Color`, tertiary: `Color`, seed: `int` | generate brick pattern `scale`  high, `ratio`=width/height, `mortar`=\[0-1\], bricks interpolate between `primary` and `secondary` colors seeded by `seed`, `tertiary` defines mortar color |
| noise        | scale: `int`, seed: `int`, primary: `Color`                                                                         | generate _Perlin_ noise of `primary` color                                                                                                                                                  |
|              |                                            |                                                                                  |
| negate       | value: `Shader`                            | negate shader `value`                                                            |
| add          | values: `[ Shader ]`                       | add all shaders in `values`                                                      |
| multiply     | values: `[ Shader ]`                       | multiply all shaders in `values`                                                 |
| mix          | values: `[ Shader ]`, weights: `[ float ]` | add all shaders in `values` first multiplied by respectable numbers in `weights` |

#
<a name="footnoteColor">1</a>: Accepted color formats:
- `x47B3DB` 6-digit hex code prepended with 'x'
- `azure` lowercase color name

Supported colors: `white`, `gray`, `black`,
        `red`, `orange`, `yellow`, `lime`,
        `green`, `turquoise`, `cyan`,
        `blue`, `azure`, `purple`, `magenta`, `pink`
