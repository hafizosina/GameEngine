#pragma once
#include "utils/Serializer.hpp"
#include "utils/Logger.hpp"
#include <array>
#include <cstdint>

namespace Zhenzhu {

// Raylib-compatible color struct alias
// We avoid including raylib.h in data layer
struct ThemeColor {
    uint8_t r, g, b, a;
};

inline ThemeColor MakeColor(std::array<uint8_t, 4> arr) { return {arr[0], arr[1], arr[2], arr[3]}; }

class ThemeDB
{
   public:
    void Init(const Json& j)
    {
        // colors
        colors.primary = MakeColor(Serializer::GetColor(j, "colors.primary", {70, 130, 180, 255}));
        colors.primaryHover = MakeColor(Serializer::GetColor(j, "colors.primaryHover", {106, 170, 212, 255}));
        colors.primaryPress = MakeColor(Serializer::GetColor(j, "colors.primaryPress", {42, 90, 138, 255}));
        colors.background = MakeColor(Serializer::GetColor(j, "colors.background", {30, 30, 35, 255}));
        colors.surface = MakeColor(Serializer::GetColor(j, "colors.surface", {45, 45, 52, 255}));
        colors.textPrimary = MakeColor(Serializer::GetColor(j, "colors.textPrimary", {240, 240, 240, 255}));
        colors.textSecondary = MakeColor(Serializer::GetColor(j, "colors.textSecondary", {160, 160, 160, 255}));
        colors.danger = MakeColor(Serializer::GetColor(j, "colors.danger", {200, 60, 60, 255}));
        colors.success = MakeColor(Serializer::GetColor(j, "colors.success", {60, 186, 95, 255}));
        colors.warning = MakeColor(Serializer::GetColor(j, "colors.warning", {212, 160, 23, 255}));
        // typography
        typography.fontId = Serializer::GetString(j, "typography.fontId", "font.main");
        typography.sizeSmall = Serializer::GetInt(j, "typography.sizeSmall", 12);
        typography.sizeNormal = Serializer::GetInt(j, "typography.sizeNormal", 16);
        typography.sizeLarge = Serializer::GetInt(j, "typography.sizeLarge", 24);
        typography.sizeTitle = Serializer::GetInt(j, "typography.sizeTitle", 48);
        // shape
        shape.cornerRadius = Serializer::GetFloat(j, "shape.cornerRadius", 6.0f);
        shape.buttonPadX = Serializer::GetFloat(j, "shape.buttonPadX", 16.0f);
        shape.buttonPadY = Serializer::GetFloat(j, "shape.buttonPadY", 8.0f);
        shape.panelPad = Serializer::GetFloat(j, "shape.panelPad", 20.0f);
        // animation
        anim.buttonHoverScale = Serializer::GetFloat(j, "animation.buttonHoverScale", 1.05f);
        anim.transitionDuration = Serializer::GetFloat(j, "animation.transitionDuration", 0.3f);

        LOG_INFO("ThemeDB loaded");
    }

    struct {
        ThemeColor primary, primaryHover, primaryPress;
        ThemeColor background, surface;
        ThemeColor textPrimary, textSecondary;
        ThemeColor danger, success, warning;
    } colors;

    struct {
        std::string fontId = "font.main";
        int sizeSmall = 12;
        int sizeNormal = 16;
        int sizeLarge = 24;
        int sizeTitle = 48;
    } typography;

    struct {
        float cornerRadius = 6.0f;
        float buttonPadX = 16.0f;
        float buttonPadY = 8.0f;
        float panelPad = 20.0f;
    } shape;

    struct {
        float buttonHoverScale = 1.05f;
        float transitionDuration = 0.3f;
    } anim;
};

}  // namespace Zhenzhu
