#include <cmath>
#include "dev/TextureBaker.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>
#include <filesystem>

namespace Zhenzhu {

bool TextureBaker::Bake(const std::string& assetId, const std::string& outputPath)
{
    if (assetId.find("tex.ui.button") != std::string::npos) {
        return BakeWoodenButton(assetId, outputPath);
    }
    if (assetId.find("tex.ui.panel.parchment") != std::string::npos) {
        return BakeParchmentPanel(assetId, outputPath);
    }
    if (assetId == "tex.player") return BakePlayer(assetId, outputPath);
    if (assetId == "tex.enemy")  return BakeEnemy(assetId, outputPath);
    if (assetId == "tex.bullet") return BakeBullet(assetId, outputPath);
    if (assetId == "tex.wall")       return BakeWoodenWall(assetId, outputPath);
    if (assetId == "tex.tile.grass") return BakeGrassTile(assetId, outputPath);

    return BakePlaceholder(assetId, outputPath);
}

bool TextureBaker::BakePlaceholder(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking texture placeholder for: " + assetId);

    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    size_t hash = std::hash<std::string>{}(assetId);
    unsigned char r = (hash & 0xFF0000) >> 16;
    unsigned char g = (hash & 0x00FF00) >> 8;
    unsigned char b = (hash & 0x0000FF);
    Color color = {r, g, b, 255};

    Image img = GenImageChecked(64, 64, 8, 8, color, DARKGRAY);
    ImageDrawRectangleLines(&img, {0, 0, 64, 64}, 2, WHITE);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)
        LOG_INFO("Baked: " + outputPath);
    else
        LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

bool TextureBaker::BakeWoodenButton(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking wooden button for: " + assetId);

    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    // Define colors for wooden style
    Color baseColor = {160, 120, 80, 255};  // Light brown (Normal)
    Color highlight = {190, 150, 110, 255}; // Lighter highlight
    Color shadow = {100, 70, 40, 255};      // Darker brown
    Color grain = {130, 90, 50, 255};       // Grain color
    int borderThick = 3;

    // Adjust based on state
    if (assetId.find("hover") != std::string::npos) {
        baseColor = {130, 90, 50, 255};     // Darker than normal
        highlight = {160, 120, 80, 255};
        shadow = {60, 40, 20, 255};
        borderThick = 6;                    // Stronger border
    } else if (assetId.find("pressed") != std::string::npos) {
        baseColor = {100, 70, 40, 255};     // Darkest
        highlight = baseColor;
        shadow = {50, 30, 15, 255};
    }

    // Larger texture for buttons
    int width = 256;
    int height = 64;
    Image img = GenImageColor(width, height, baseColor);

    // Grain: random horizontal lines or rectangles
    for (int i = 0; i < 20; ++i) {
        int y = (assetId.length() * i * 7) % height;
        int h = 2 + (i % 6);
        ImageDrawRectangle(&img, 0, y, width, h, grain);
    }

    // Border (4 sides)
    ImageDrawRectangle(&img, 0, 0, width, borderThick, shadow);                         // Top
    ImageDrawRectangle(&img, 0, height - borderThick, width, borderThick, shadow);     // Bottom
    ImageDrawRectangle(&img, 0, 0, borderThick, height, shadow);                         // Left
    ImageDrawRectangle(&img, width - borderThick, 0, borderThick, height, shadow);       // Right

    // Highlight (inside the border)
    if (assetId.find("pressed") == std::string::npos) {
        ImageDrawRectangle(&img, borderThick, borderThick, width - 2 * borderThick, 2, highlight);
        ImageDrawRectangle(&img, borderThick, borderThick, 2, height - 2 * borderThick, highlight);
    }

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)
        LOG_INFO("Baked: " + outputPath);
    else
        LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

bool TextureBaker::BakeParchmentPanel(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking parchment panel for: " + assetId);

    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    // Parchment colors
    Color baseColor = {245, 230, 190, 255};   // Light yellowish cream
    Color spotsColor = {230, 215, 175, 255};  // Slightly darker spots
    Color borderColor = {139, 115, 85, 255};  // Aged brown border

    int width = 512;
    int height = 512;
    Image img = GenImageColor(width, height, baseColor);

    // Add some random "aging" spots
    for (int i = 0; i < 150; ++i) {
        int x = (i * 12345) % width;
        int y = (i * 54321) % height;
        int radius = 1 + (i % 4);
        ImageDrawCircle(&img, x, y, radius, spotsColor);
    }

    // Soft border to simulate worn edges
    ImageDrawRectangleLines(&img, {0, 0, (float)width, (float)height}, 4, borderColor);
    ImageDrawRectangleLines(&img, {4, 4, (float)width - 8, (float)height - 8}, 1, spotsColor);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)
        LOG_INFO("Baked: " + outputPath);
    else
        LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

bool TextureBaker::BakePlayer(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Baking player for: " + assetId);
    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());
    
    Image img = GenImageColor(64, 64, BLANK);
    // Body: Blue knight-ish shield
    ImageDrawRectangle(&img, 16, 8, 32, 48, SKYBLUE);
    ImageDrawRectangleLines(&img, {16, 8, 32, 48}, 3, BLUE);
    // Visor
    ImageDrawRectangle(&img, 20, 16, 24, 8, DARKGRAY);
    // Shoulder pads
    ImageDrawCircle(&img, 16, 24, 8, DARKBLUE);
    ImageDrawCircle(&img, 48, 24, 8, DARKBLUE);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);
    return ok;
}

bool TextureBaker::BakeEnemy(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Baking enemy for: " + assetId);
    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());
    
    Image img = GenImageColor(64, 64, BLANK);
    // Body: Red angry block
    ImageDrawRectangle(&img, 12, 12, 40, 40, RED);
    ImageDrawRectangleLines(&img, {12, 12, 40, 40}, 3, MAROON);
    // Eyes
    ImageDrawRectangle(&img, 20, 24, 8, 8, YELLOW);
    ImageDrawRectangle(&img, 36, 24, 8, 8, YELLOW);
    // Simple blocky horns
    ImageDrawRectangle(&img, 12, 4, 8, 8, MAROON);
    ImageDrawRectangle(&img, 44, 4, 8, 8, MAROON);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);
    return ok;
}

bool TextureBaker::BakeBullet(const std::string& assetId, const std::string& outputPath) {
    LOG_INFO("Baking bullet for: " + assetId);
    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());
    
    Image img = GenImageColor(32, 32, BLANK);
    // Simple glow using nested circles since ImageDrawCircleGradient doesn't exist
    ImageDrawCircle(&img, 16, 16, 12, {255, 200, 0, 100});
    ImageDrawCircle(&img, 16, 16, 8, YELLOW);
    ImageDrawCircle(&img, 16, 16, 3, WHITE);

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);
    return ok;
}

bool TextureBaker::BakeWoodenWall(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking wooden wall for: " + assetId);
    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    const int W = 64, H = 64;

    // Plank colours
    Color base      = {110, 70, 35, 255};   // mid brown
    Color light     = {135, 90, 50, 255};   // lighter grain
    Color dark      = {75,  45, 18, 255};   // dark grain
    Color gap       = {45,  25,  8, 255};   // plank divider
    Color border    = {35,  18,  5, 255};   // outer frame
    Color knotOuter = {60,  35, 12, 255};
    Color knotInner = {40,  20,  5, 255};

    Image img = GenImageColor(W, H, base);

    // ── Two horizontal planks separated by a dark gap ──────────────
    // Plank A: rows 2..29   Plank B: rows 34..61
    // Gap: rows 30..33
    int plankDefs[2][2] = {{2, 29}, {34, 61}};

    for (int p = 0; p < 2; ++p) {
        int y0 = plankDefs[p][0];
        int y1 = plankDefs[p][1];

        // Horizontal grain lines within this plank
        for (int i = 0; i < 6; ++i) {
            // Deterministic spread across plank height
            int gy = y0 + 2 + (i * (y1 - y0)) / 7;
            Color gc = (i % 2 == 0) ? light : dark;
            ImageDrawRectangle(&img, 2, gy, W - 4, 1, gc);
        }

        // Subtle vertical colour variation (simulates wood cross-grain)
        for (int x = 4; x < W - 4; x += 8) {
            Color vc = (x % 16 == 4) ? dark : light;
            ImageDrawRectangle(&img, x, y0, 1, y1 - y0, {vc.r, vc.g, vc.b, 40});
        }

        // Knot — one per plank, offset to avoid repetition
        int kx = (p == 0) ? 18 : 44;
        int ky = y0 + (y1 - y0) / 2;
        ImageDrawCircle(&img, kx, ky, 5, knotOuter);
        ImageDrawCircle(&img, kx, ky, 2, knotInner);
        // Grain curves around knot
        ImageDrawRectangle(&img, kx - 8, ky - 1, 5, 1, dark);
        ImageDrawRectangle(&img, kx + 4, ky - 1, 5, 1, dark);
    }

    // Nail-head hints at plank corners
    auto nail = [&](int x, int y) {
        ImageDrawRectangle(&img, x, y, 3, 3, border);
        ImageDrawRectangle(&img, x + 1, y + 1, 1, 1, {180, 150, 120, 200});
    };
    nail(4,  4);  nail(W - 7, 4);
    nail(4,  H - 7); nail(W - 7, H - 7);
    nail(4,  31); nail(W - 7, 31);   // mid-row nails

    // Plank gap
    ImageDrawRectangle(&img, 2, 30, W - 4, 4, gap);

    // Outer border
    ImageDrawRectangle(&img, 0, 0,     W, 2,     border); // top
    ImageDrawRectangle(&img, 0, H - 2, W, 2,     border); // bottom
    ImageDrawRectangle(&img, 0, 0,     2, H,     border); // left
    ImageDrawRectangle(&img, W - 2, 0, 2, H,     border); // right

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)  LOG_INFO("Baked: " + outputPath);
    else     LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}

bool TextureBaker::BakeGrassTile(const std::string& assetId, const std::string& outputPath)
{
    LOG_INFO("Baking grass autotile sheet for: " + assetId);
    std::filesystem::create_directories(std::filesystem::path(outputPath).parent_path());

    // Dual-grid autotile sheet: 4 columns × 4 rows = 16 variants.
    // Variant index = 4-bit bitmask: bit0=TL, bit1=TR, bit2=BL, bit3=BR.
    // Each bit = 1 means that data-grid corner is "grass"; 0 means "dirt/other".
    // Each quadrant of the tile (8×8) is drawn as grass or dirt accordingly.
    constexpr int TILE = 32;         // Increased to 32 for a clearer reference
    constexpr int HALF = TILE / 2;   // 16 — one quadrant
    constexpr int W    = TILE * 4;   // 128
    constexpr int H    = TILE * 4;   // 128

    const Color gBase  = {68,  155, 45, 255};  // Grass
    const Color gLight = {92,  198, 65, 255};
    const Color gDark  = {48,  118, 32, 255};
    const Color dBase  = {148, 112, 68, 255};  // Dirt
    const Color dLight = {172, 134, 88, 255};
    const Color dDark  = {115,  85, 52, 255};

    Image img = GenImageColor(W, H, BLANK);

    // Deterministic Hash for noise
    auto Hash = [](int x, int y) -> float {
        uint32_t h = (uint32_t)x * 374761393U + (uint32_t)y * 668265263U;
        h = (h ^ (h >> 13)) * 1274126177U;
        return (float)(h & 0x7FFFFFFF) / (float)0x7FFFFFFF;
    };

    // Smoothing Helpers
    auto Lerp = [](float a, float b, float t) { return a + t * (b - a); };
    auto SmoothStep = [](float t) { return t * t * (3.0f - 2.0f * t); };

    auto Hash2D = [](int x, int y) -> float {
        uint32_t h = (uint32_t)x * 374761393U + (uint32_t)y * 668265263U;
        h = (h ^ (h >> 13)) * 1274126177U;
        return (float)(h & 0x7FFFFFFF) / (float)0x7FFFFFFF;
    };

    auto ValueNoise = [&](float x, float y) -> float {
        int ix = (int)std::floor(x);
        int iy = (int)std::floor(y);
        float fx = x - ix;
        float fy = y - iy;

        float a = Hash2D(ix, iy);
        float b = Hash2D(ix + 1, iy);
        float c = Hash2D(ix, iy + 1);
        float d = Hash2D(ix + 1, iy + 1);

        float ux = SmoothStep(fx);
        float uy = SmoothStep(fy);

        return Lerp(Lerp(a, b, ux), Lerp(c, d, ux), uy);
    };

    const int layout[16] = {
        4,  10, 13, 12, 
        9,  14, 15, 7,  
        2,  3,  11, 5,  
        0,  8,  6,  1   
    };

    for (int i = 0; i < 16; i++) {
        int mask = layout[i];
        int ox = (i % 4) * TILE;
        int oy = (i / 4) * TILE;

        bool corners[4] = {
            (mask & 1) != 0, (mask & 2) != 0, 
            (mask & 4) != 0, (mask & 8) != 0
        };

        // ── Render Tile Pixels ───────────────────────────────────────
        for (int ly = 0; ly < TILE; ly++) {
            for (int lx = 0; lx < TILE; lx++) {
                int px = ox + lx;
                int py = oy + ly;

                // Base smooth noise
                float n = ValueNoise((float)px * 0.15f, (float)py * 0.15f);
                // Extra crunch for the edges
                float edgeCrunch = Hash2D(px, py);

                // Determine base quadrant
                int q = (lx < HALF ? 0 : 1) + (ly < HALF ? 0 : 2);
                bool isGrass = corners[q];

                // ── Jagged Boundary Transition ───────────────────────
                float distToEdgeX = std::abs((float)lx - HALF + 0.5f);
                float distToEdgeY = std::abs((float)ly - HALF + 0.5f);

                auto JaggedEdge = [&](float dist, int otherQ) {
                    if (dist < 3.5f) {
                        if (corners[otherQ] != isGrass) {
                            // Hard threshold flip for "crunchy" jagged look
                            if (edgeCrunch > (dist / 4.0f) + 0.2f) isGrass = corners[otherQ];
                        }
                    }
                };
                
                JaggedEdge(distToEdgeX, (lx < HALF) ? q + 1 : q - 1);
                JaggedEdge(distToEdgeY, (ly < HALF) ? q + 2 : q - 2);

                // ── Color Selection (Post-Transition) ─────────────────
                Color c = isGrass ? gBase : dBase;
                
                // Subtle organic shading (Surface only)
                if (n < 0.25f) {
                    float t = (0.25f - n) / 0.25f;
                    c.r = (unsigned char)Lerp(c.r, isGrass ? gDark.r : dDark.r, t);
                    c.g = (unsigned char)Lerp(c.g, isGrass ? gDark.g : dDark.g, t);
                    c.b = (unsigned char)Lerp(c.b, isGrass ? gDark.b : dDark.b, t);
                } else if (n > 0.75f) {
                    float t = (n - 0.75f) / 0.25f;
                    c.r = (unsigned char)Lerp(c.r, isGrass ? gLight.r : dLight.r, t);
                    c.g = (unsigned char)Lerp(c.g, isGrass ? gLight.g : dLight.g, t);
                    c.b = (unsigned char)Lerp(c.b, isGrass ? gLight.b : dLight.b, t);
                }

                // Add very subtle tufts
                if (isGrass && edgeCrunch > 0.98f) {
                    ImageDrawPixel(&img, px, py, gLight);
                }

                ImageDrawPixel(&img, px, py, c);
            }
        }
    }

    bool ok = ExportImage(img, outputPath.c_str());
    UnloadImage(img);

    if (ok)  LOG_INFO("Baked: " + outputPath);
    else     LOG_ERROR("Failed to bake: " + outputPath);
    return ok;
}


}  // namespace Zhenzhu
