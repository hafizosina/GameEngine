#pragma once

namespace Zhenzhu {
namespace Assets {

    // ── UI ──────────────────────────────────────────
    constexpr const char* TEX_UI_BUTTON_NORMAL    = "tex.ui.button.normal";
    constexpr const char* TEX_UI_BUTTON_HOVER     = "tex.ui.button.hover";
    constexpr const char* TEX_UI_BUTTON_PRESSED   = "tex.ui.button.pressed";
    constexpr const char* TEX_UI_PANEL_PARCHMENT  = "tex.ui.panel.parchment";
    
    // ── Gameplay ────────────────────────────────────
    constexpr const char* TEX_PLAYER             = "tex.player";
    constexpr const char* TEX_ENEMY              = "tex.enemy";
    constexpr const char* TEX_BULLET             = "tex.bullet";
    constexpr const char* TEX_WALL               = "tex.wall";

    // ── Tiles ───────────────────────────────────────
    // Dual-grid autotile sheets: 128×128, 4×4 grid of 32px variants.
    // Bitmask bits: TL=1, TR=2, BL=4, BR=8
    constexpr const char* TEX_TILE_DIRT          = "tex.tile.dirt";
    constexpr const char* TEX_TILE_GRASS         = "tex.tile.grass";
    constexpr const char* TEX_TILE_SAND          = "tex.tile.sand";
    constexpr const char* TEX_TILE_WATER         = "tex.tile.water";
    constexpr const char* TEX_TILE_STONE         = "tex.tile.stone";

    // ── SFX ─────────────────────────────────────────
    constexpr const char* SFX_UI_HOVER            = "sfx.ui.hover";

    // ── Common ──────────────────────────────────────
    constexpr const char* TEX_MISSING             = "tex.missing";
    constexpr const char* FONT_MAIN               = "font.main";

} // namespace Assets
} // namespace Zhenzhu
