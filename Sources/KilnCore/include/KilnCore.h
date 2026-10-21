// Kiln simulation core: a plain C API over the C++ world so Swift can call it directly.
#ifndef KILN_CORE_H
#define KILN_CORE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Element ids. Mirrored by `Element` on the Swift side; keep the values in sync.
enum {
    KILN_EMPTY = 0,
    KILN_WALL = 1,      // concrete
    KILN_SAND = 2,
    KILN_WATER = 3,
    KILN_WOOD = 4,
    KILN_FIRE = 5,
    KILN_SMOKE = 6,
    KILN_OIL = 7,
    KILN_STEAM = 8,
    KILN_EMBER = 9,
    KILN_STEEL = 10,
    KILN_METHANE = 11,
    KILN_ASH = 12,
    KILN_ELEMENT_COUNT = 13
};

enum { KILN_VIEW_NORMAL = 0, KILN_VIEW_HEAT = 1, KILN_VIEW_OXYGEN = 2 };
enum { KILN_THEME_DARK = 0, KILN_THEME_WHITE = 1 };
enum { KILN_SCENE_EMPTY_LAB = 0, KILN_SCENE_DEMO = 1 };

typedef struct KilnWorld KilnWorld;

typedef struct {
    bool valid;
    int32_t element;
    bool burning;
    float temperatureC;
    float oxygenPercent;
} KilnProbe;

typedef struct {
    uint64_t tick;
    int32_t particles;
    int32_t burning;
    float peakTemperatureC;
} KilnStats;

KilnWorld *kiln_world_create(int32_t width, int32_t height, uint64_t seed);
void kiln_world_destroy(KilnWorld *world);

int32_t kiln_world_width(const KilnWorld *world);
int32_t kiln_world_height(const KilnWorld *world);

void kiln_world_clear(KilnWorld *world);
void kiln_world_load_scene(KilnWorld *world, int32_t scene);

// Advances the simulation by one fixed tick (1/60 s of game time).
void kiln_world_step(KilnWorld *world);

// Gravity as a vector in screen space (+y is down), in g. (0, 1) is normal; (0, 0) is zero-g.
void kiln_world_set_gravity(KilnWorld *world, float dx, float dy);

void kiln_world_paint(KilnWorld *world, int32_t x, int32_t y, int32_t radius, int32_t element);
void kiln_world_paint_line(KilnWorld *world, int32_t x0, int32_t y0, int32_t x1, int32_t y1,
                           int32_t radius, int32_t element);
// Sets exactly one cell, with no brush randomness. Meant for scripts and tests.
void kiln_world_set_cell(KilnWorld *world, int32_t x, int32_t y, int32_t element);
int32_t kiln_world_get_cell(const KilnWorld *world, int32_t x, int32_t y);

// Writes width*height RGBA8 pixels into each buffer.
// `base` is surface colour (albedo); `emission` is emitted light, encoded as value / 4.
// The heat and oxygen views write a ready-to-show false-colour image into `base` and no emission.
void kiln_world_render(const KilnWorld *world, int32_t view, int32_t theme, uint32_t *base,
                       uint32_t *emission);

KilnProbe kiln_world_probe(const KilnWorld *world, int32_t x, int32_t y);
KilnStats kiln_world_stats(const KilnWorld *world);

#ifdef __cplusplus
}
#endif

#endif
