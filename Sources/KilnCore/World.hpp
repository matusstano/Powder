#pragma once

#include <cstdint>
#include <vector>

namespace kiln {

enum Element : uint8_t {
    Empty = 0,
    Wall,
    Sand,
    Water,
    Wood,
    Fire,
    Smoke,
    Oil,
    Steam,
    Ember,
    Steel,
    Methane,
    Ash,
    ElementCount
};

enum View { ViewNormal = 0, ViewHeat = 1, ViewOxygen = 2 };
enum Theme { ThemeDark = 0, ThemeWhite = 1 };

// xorshift64*: fast, and seeded, so a run replays identically.
struct Rng {
    uint64_t state;
    explicit Rng(uint64_t seed) : state(seed ? seed : 0x9E3779B97F4A7C15ull) {}
    uint32_t next() {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return uint32_t((state * 0x2545F4914F6CDD1Dull) >> 32);
    }
    float unit() { return float(next() >> 8) * (1.0f / 16777216.0f); }
    bool chance(float p) { return unit() < p; }
    uint32_t below(uint32_t n) { return next() % n; }
};

struct Stats {
    uint64_t tick;
    int particles;
    int burning;
    float peakTemperatureC;
};

// The world is a grid of cells. Each cell holds one element plus a small amount of state,
// and two continuous fields (heat and oxygen) are diffused across the grid every tick.
class World {
public:
    World(int width, int height, uint64_t seed);

    int width() const { return W; }
    int height() const { return H; }
    uint64_t ticks() const { return tick; }

    void clear();
    void loadScene(int scene);
    void step();
    void setGravity(float dx, float dy);

    void paint(int cx, int cy, int radius, int element);
    void paintLine(int x0, int y0, int x1, int y1, int radius, int element);
    void setCell(int x, int y, int element);

    void render(int view, int theme, uint32_t *base, uint32_t *emission) const;

    bool inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < W && y < H; }
    Element at(int x, int y) const { return Element(type[index(x, y)]); }
    bool isBurning(int x, int y) const;
    float temperatureC(int x, int y) const { return 20.0f + heat[index(x, y)] * 1180.0f; }
    float oxygenFraction(int x, int y) const { return oxygen[index(x, y)]; }
    Stats stats() const;

private:
    int index(int x, int y) const { return y * W + x; }
    int neighbor(int x, int y, int dir) const;
    int randomNeighbor(int x, int y);
    int up() const { return (down + 4) & 7; }

    void swapCells(int a, int b);
    void updateCell(int x, int y);
    void updatePowder(int i, int x, int y, uint8_t c);
    void updateLiquid(int i, int x, int y, uint8_t c);
    void updateEmber(int i, int x, int y);
    void updateGas(int i, int x, int y, uint8_t c);
    bool burn(int i, int x, int y, int maxLife);
    void updateFields();
    void fillRect(int x0, int y0, int x1, int y1, int element, float density = 1.0f);

    int W;
    int H;
    Rng rng;
    uint64_t tick = 0;
    uint8_t stamp = 1;

    // Gravity: `down` indexes the 8-neighbour direction table, `gravity` is the strength in g.
    int down = 2;
    float gravity = 1.0f;

    std::vector<uint8_t> type;      // Element
    std::vector<uint16_t> life;     // lifetime for gases and embers, burn progress for fuels
    std::vector<uint8_t> variant;   // per-particle texture noise; travels with the particle
    std::vector<uint8_t> moved;     // stamp of the tick a cell last moved, so nothing moves twice
    std::vector<float> heat;        // 0 = ambient, 1 = flame temperature
    std::vector<float> heatNext;
    std::vector<float> oxygen;      // 1 = fresh air
    std::vector<float> oxygenNext;
    std::vector<uint8_t> permeable; // gas can flow through this cell
    std::vector<uint8_t> sky;       // open path to the edge the hot gas rises toward
};

}
