# Ray Tracing
[![platform](https://img.shields.io/badge/platform-macOS-lightgray)](https://www.apple.com/macos)
[![framework](https://img.shields.io/badge/framework-X11-brightgreen)](https://www.xquartz.org)
[![CodeFactor](https://www.codefactor.io/repository/github/adamsvestka/ray-tracing/badge)](https://www.codefactor.io/repository/github/adamsvestka/ray-tracing)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/ce7c6be071ec40078c3f88480471cb47)](https://www.codacy.com/manual/svestka.adam1/Ray-Tracing?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=adamsvestka/Ray-Tracing&amp;utm_campaign=Badge_Grade)
[![GitHub milestones](https://img.shields.io/github/milestones/progress-percent/adamsvestka/Ray-Tracing/2)](https://github.com/adamsvestka/Ray-Tracing/milestone/2)
[![GitHub issues](https://img.shields.io/github/issues/adamsvestka/Ray-Tracing)](https://github.com/adamsvestka/Ray-Tracing/issues)
[![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/adamsvestka/Ray-Tracing)](https://github.com/adamsvestka/Ray-Tracing)
[![GitHub releases](https://img.shields.io/github/v/release/adamsvestka/Ray-Tracing?include_prereleases)](https://github.com/adamsvestka/Ray-Tracing/releases)

Můj vlastní program pro sledování paprsků, který jsem vytvořil, abych dokázal, že můžu.

Také jsem vytvořil WebAssembly verzi, která může běžet kdekoli, na jakémkoli zařízení a je hostována na mých [GitHub stránkách](https://adamsvestka.github.io/raytracing).

## Závislosti
- [XQuartz](https://www.xquartz.org)
- [CImg](https://cimg.eu)
- [libpng](http://www.libpng.org)
- [libjpeg](http://libjpeg.sourceforge.net)

## Začleněné knihovny
- [nlohman/json](https://github.com/nlohmann/json)

## Instrukce k instalaci
1. Stáhněte si nejnovější verzi z [GitHubu](https://github.com/adamsvestka/Ray-Tracing/releases)
1. Nainstalujte si Homebrew: `/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"`
1. Nainstalujte závislosti: `brew install libpng libjpeg cimg`
1. Stáhněte a nainstalujte [XQuartz](https://www.xquartz.org)
1. Spusťte program `Ray Tracing`

---

Některá data se načítají za běhu z konfiguračních souborů:

## Soubor nastavení

Tento soubor obsahuje nastavení způsobu vykreslení scény.
- název souboru: `settings.ini`
- formát: `text/ini`

```ini
# komentář
[Sekce]
klíč=hodnota
```

### Seznam možností

| klíč | popis | datový typ | výchozí hodnota |
|---------------------|-------------------------------------------------------------------------------------------|---------------------------------------|-----------|
| max_render_distance | Zadní oříznutí scény | `int` | `100` |
| surface_bias | Posunutí sraženého paprseku, aby se zabránilo stínovému akné `float` | `0.001` |
| max_light_bounces | Zabránění nekonečných smyček | `int` | `5` |
| render_mode | Jaké vrstvy se mají sbírat při srážkách | `enum (0-7)` | `0` |
| render_pattern | Podle jakého vzoru vykreslovat regiony | `enum (0-2)` | `1` |
| show_debug | Zobrazit terče nad oblastmi určující, co se má vykreslit; preprocess musí být true, aby se toto projevilo | `bool` | `true` |
| preprocess | Vykreslit pouze to, co je nezbytné; !! může mít za následek problémy s vykreslením | `bool` | `false` |
| save_render | Uložte výsledek do paměti, aby se následně mohli přepínat vrstvy | `bool` | `true` |
| resolution_decrease | Vydělit rozlišení číslem | `int` | `1` |
| render_region_size | Velikost vykreslovacích regionů | `int` | `10` |
| rendering_threads | Počet vláken pro vykreslování | `int` | `25` |
| background_color | Barva pozadí pro vyplnění prázdného místa `Color`<sup>[1](#footnoteColor)</sup> | `x000000` |

## Soubor scény

Tento soubor definuje objekty a osvětlení ve scéně.
- název souboru: `scence.json`
- formát: `text/json`

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

### Formát Vector3

```js
Vector3: {
  "x": float,
  "y": float,
  "z": float
}
```

### Formát barvy <sup>[1](#footnoteColor)</sup>

```js
Color: string
```

### Formát materiálu

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

### Seznam typů objektů

| typ    | parametry                                                                                                         |
|--------|-------------------------------------------------------------------------------------------------------------------|
| sphere | position: `Vector3`, diameter: `float`, rotation: `Vector3`, material: `Material`                                 |
| cube   | position: `Vector3`, size: `float`, rotation: `Vector3`, material: `Material`                                     |
| cube-2 | position: `Vector3`, size_x: `float`, size_y: `float`, size_z: `float`, rotation: `Vector3`, material: `Material` |
| cube-3 | corner_min: `Vector3`, corner_max: `Vector3`, rotation: `Vector3`, material: `Material`                           |
| plane  | position: `Vector3`, size_x: `float`, size_y: `float`, rotation: `Vector3`, material: `Material`                  |
| object | name: `string`, position: `Vector3`, scale: `float`, rotation: `Vector3`, material: `Material`                    |

### Seznam typů osvětlení

| typ         | parametry                                              |
|-------------|--------------------------------------------------------|
| point       | position: `Vector3`, color: `Color`, intensity: `int`  |
| linear      | position: `Vector3`, color: `Color`, intensity: `int`  |
| global      | color: `Color`, intensity: `int`                       |
| directional | direction: `Vector3`, color: `Color`, intensity: `int` |

### Seznam typů shaderů

| typ          | parametry                                                                                                           | info                                                                                                                                                                                        |
|--------------|---------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| color        | value: `Color`                                                                                                      | jednobarevná textura                                                                                                                                                                        |
| image        | value: `string`                                                                                                     | použít obrázek jako texturu                                                                                                                                                                        |
| checkerboard | scale: `int`, primary: `Color`, secondary: `Color`                                                                  | vygeneruje šachovnicový vzor o velikosti `scale`x`scale` dvou barev                                                                                                                           |
| bricks       | scale: `int`, ratio: `float`, mortar: `float`, primary: `Color`, secondary: `Color`, tertiary: `Color`, seed: `int` | vygeneruje cihlový vzor `scale` vysoký, `ratio`=šířka/výška, `mortar`=\[0-1\], barva cihel interpoluje mezi `primary` a `secondary` barvou náhodně pomocí `seed`, `tertiary` definuje barvu malty |
| noise        | scale: `int`, seed: `int`, primary: `Color`                                                                         | vygeneruje _Perlinův_ šum `primary` barvy                                                                                                                                                  |
|              |                                                                                                                     |                                                                                                                                                                                             |
| named        | value: `string`                                                                                                     | Odkáže na shader s názvem `value` definovaný v sekci `shaders`                                                                                                                                                  |
|              |                                                                                                                     |                                                                                                                                                                                             |
| grayscale    | value: `Shader`                                                                                                     | hodnota `value` shaderu ve stupních šedi                                                                                                                                                                    |
| negate       | value: `Shader`                                                                                                     | negace shaderu `value`                                                                                                                                                                       |
| add          | values: `[ Shader ]`                                                                                                | součet všech shaderů ve `values`                                                                                                                                                                 |
| multiply     | values: `[ Shader ]`                                                                                                | součin všech shaderů ve `values`                                                                                                                                                            |
| mix          | values: `[ Shader ]`, weights: `[ float ]`                                                                          | součet všech shaderů ve `values` nejprve vynásobeno příslušnými čísly ve `weights`                                                                                                            |

---

<a name="footnoteColor">1</a>: Přijímané barevné formáty:
- `x47B3DB` šestimístný hexadecimální kód doplněný znakem 'x'
- `gray` malý název barvy

Podporované barvy:
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
