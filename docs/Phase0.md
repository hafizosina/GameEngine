# Phase 0 — Project Foundation — Full Plan

---

## Goal
```
By end of Phase 0:
  ✅ CLion fully configured
  ✅ CMake + CPM pulling all dependencies
  ✅ All vendor libraries compile
  ✅ Window opens with raylib
  ✅ Logger prints to console
  ✅ Application loop runs (Init/Run/Shutdown)
  ✅ Timer gives delta time + FPS
  ✅ ServiceLocator works
  ✅ EngineConfig struct exists
  ✅ Clean folder skeleton for all future phases
```

---

## Part 1 — CLion Setup

### Install Requirements First
```
REQUIRED TOOLS (install before opening CLion):

Windows:
  □ CLion (JetBrains)
  □ CMake 3.25+       → cmake.org
  □ Visual Studio 2022 Build Tools
      → only need "Desktop development with C++"
      → NOT full Visual Studio IDE
  □ Git               → git-scm.com
  □ Ninja             → github.com/ninja-build/ninja
      → add to PATH

Mac:
  □ CLion
  □ Xcode Command Line Tools  → xcode-select --install
  □ CMake via homebrew        → brew install cmake
  □ Git                       → already in xcode tools
  □ Ninja                     → brew install ninja

Linux:
  □ CLion
  □ build-essential   → sudo apt install build-essential
  □ CMake             → sudo apt install cmake
  □ Git               → sudo apt install git
  □ Ninja             → sudo apt install ninja-build
  □ libGL dev         → sudo apt install libgl1-mesa-dev
  □ libX11 dev        → sudo apt install libx11-dev
```

---

### CLion Project Settings
```
File → Settings → Build, Execution, Deployment → CMake

Add TWO profiles:

PROFILE 1 — Debug
  Name:           Debug
  Build type:     Debug
  Generator:      Ninja
  Build dir:      build/debug
  CMake options:  -DCMAKE_BUILD_TYPE=Debug
                  -DENGINE_DEBUG=ON

PROFILE 2 — Release
  Name:           Release
  Build type:     Release
  Generator:      Ninja
  Build dir:      build/release
  CMake options:  -DCMAKE_BUILD_TYPE=Release
                  -DENGINE_DEBUG=OFF


File → Settings → Build, Execution, Deployment → Toolchains

Windows:   set to "Visual Studio" → point to MSVC
Mac/Linux: set to "System" → uses clang/gcc from PATH
```

---

### CLion Code Style Settings
```
File → Settings → Editor → Code Style → C/C++

Indent:             4 spaces (not tabs)
Brace style:        Allman (opening brace on new line)
Namespace indent:   OFF
Max line length:    120

File → Settings → Editor → File and Code Templates
  C++ Header File:
    #pragma once       ← add this as first line in header template

File → Settings → Editor → General → Auto Import
  ✅ Add unambiguous imports on the fly
```

---

### CLion Plugins to Install
```
File → Settings → Plugins → Marketplace

  □ .gitignore         ← gitignore file support
  □ CMake Simple        ← better CMake highlighting
  □ Rainbow Brackets   ← color-matched brackets (very helpful in C++)
  □ GitHub Copilot     ← optional AI assist
```

---

### .clang-format (Auto Format on Save)
```
// create this file at project root: .clang-format

BasedOnStyle:              Google
IndentWidth:               4
ColumnLimit:               120
BraceWrapping:
  AfterFunction:           true
  AfterClass:              true
  AfterControlStatement:   false
BreakBeforeBraces:         Custom
PointerAlignment:          Left
SpaceAfterTemplateKeyword: false
SortIncludes:              false
```

Enable in CLion:
```
File → Settings → Editor → Code Style
  ✅ Enable ClangFormat
  ✅ Format on save
```

---

### .gitignore
```
# Build output
build/
cmake-build-debug/
cmake-build-release/
cmake-build-*/

# CLion
.idea/

# Vendor (pulled by CPM at build time)
vendor/

# OS
.DS_Store
Thumbs.db

# Compiled output
*.exe
*.out
*.app
*.pdb

# Generated placeholders (Phase 2+)
assets/placeholder/
```

---

## Part 2 — Folder Skeleton

> Create ALL folders and empty files now.
> Future phases just fill them in.

```
zhenzhu-engine/
│
├── .clang-format
├── .gitignore
├── CMakeLists.txt               ← root cmake
├── cmake/
│   └── CPM.cmake                ← copy from github
│
├── config/
│   ├── settings.json            ← stub (Phase 1 fills)
│   ├── keybinds.json            ← stub
│   ├── ui_theme.json            ← stub
│   ├── assets.json              ← stub
│   ├── game_config.json         ← stub
│   └── scenes.json              ← stub
│
├── assets/
│   ├── textures/                ← empty, real art goes here
│   ├── sounds/                  ← empty
│   ├── fonts/                   ← empty
│   └── placeholder/             ← empty, generated in Phase 6
│
├── engine/
│   ├── core/
│   │   ├── Application.hpp
│   │   ├── Application.cpp
│   │   ├── Window.hpp
│   │   ├── Window.cpp
│   │   ├── Timer.hpp
│   │   ├── Timer.cpp
│   │   ├── ServiceLocator.hpp
│   │   └── EngineConfig.hpp
│   │
│   ├── data/                    ← empty stubs, Phase 1
│   │   ├── DataManager.hpp
│   │   ├── SettingsDB.hpp
│   │   ├── KeybindDB.hpp
│   │   ├── ThemeDB.hpp
│   │   ├── GameConfigDB.hpp
│   │   ├── AssetDB.hpp
│   │   └── SceneDB.hpp
│   │
│   ├── assets/                  ← empty stubs, Phase 2
│   │   ├── AssetTracker.hpp
│   │   └── AssetEntry.hpp
│   │
│   ├── resources/               ← empty stubs, Phase 2
│   │   ├── ResourceManager.hpp
│   │   ├── TextureLoader.hpp
│   │   ├── FontLoader.hpp
│   │   ├── SoundLoader.hpp
│   │   ├── MusicLoader.hpp
│   │   └── DataLoader.hpp
│   │
│   ├── ecs/                     ← empty stubs, Phase 4
│   │   ├── Registry.hpp
│   │   ├── Entity.hpp
│   │   ├── components/
│   │   │   ├── Transform2D.hpp
│   │   │   ├── Velocity2D.hpp
│   │   │   ├── Health.hpp
│   │   │   ├── Sprite.hpp
│   │   │   ├── Animator.hpp
│   │   │   ├── Collider2D.hpp
│   │   │   ├── RigidBody2D.hpp
│   │   │   ├── AudioSource.hpp
│   │   │   ├── Script.hpp
│   │   │   └── Tags.hpp
│   │   └── systems/
│   │       ├── MovementSystem2D.hpp
│   │       ├── CollisionSystem2D.hpp
│   │       ├── PhysicsSystem2D.hpp
│   │       ├── AnimationSystem.hpp
│   │       ├── RenderSystem2D.hpp
│   │       ├── HealthSystem.hpp
│   │       ├── AISystem.hpp
│   │       └── ScriptSystem.hpp
│   │
│   ├── ui/                      ← empty stubs, Phase 6
│   │   ├── UISystem.hpp
│   │   ├── UIContext.hpp
│   │   ├── core/
│   │   │   ├── UINode.hpp
│   │   │   └── UICanvas.hpp
│   │   ├── layout/
│   │   │   ├── LayoutEngine.hpp
│   │   │   ├── Anchor.hpp
│   │   │   └── FlexLayout.hpp
│   │   ├── widgets/
│   │   │   ├── UIPanel.hpp
│   │   │   ├── UILabel.hpp
│   │   │   ├── UIButton.hpp
│   │   │   ├── UIImage.hpp
│   │   │   ├── UISlider.hpp
│   │   │   ├── UITextInput.hpp
│   │   │   └── UIScrollView.hpp
│   │   ├── animation/
│   │   │   ├── UIAnimator.hpp
│   │   │   └── UITransition.hpp
│   │   └── style/
│   │       ├── UITheme.hpp
│   │       └── UIStyleSheet.hpp
│   │
│   ├── renderer/                ← empty stubs, Phase 3
│   │   ├── Renderer2D.hpp
│   │   ├── Camera2D.hpp
│   │   ├── RenderLayer.hpp
│   │   ├── SpriteBatch.hpp
│   │   └── DebugDraw2D.hpp
│   │
│   ├── input/                   ← empty stubs, Phase 3
│   │   ├── InputManager.hpp
│   │   ├── Keyboard.hpp
│   │   ├── Mouse.hpp
│   │   ├── Gamepad.hpp
│   │   └── InputAction.hpp
│   │
│   ├── audio/                   ← empty stubs, Phase 5
│   │   ├── AudioManager.hpp
│   │   ├── SoundPlayer.hpp
│   │   ├── MusicPlayer.hpp
│   │   └── AudioBus.hpp
│   │
│   ├── scene/                   ← empty stubs, Phase 5
│   │   ├── SceneManager.hpp
│   │   ├── Scene.hpp
│   │   └── transitions/
│   │       ├── SceneTransition.hpp
│   │       ├── FadeTransition.hpp
│   │       ├── SlideTransition.hpp
│   │       └── ZoomTransition.hpp
│   │
│   ├── physics/                 ← empty stubs, Phase 4
│   │   ├── PhysicsWorld2D.hpp
│   │   ├── RigidBody2D.hpp
│   │   ├── Collider2D.hpp
│   │   └── PhysicsSystem2D.hpp
│   │
│   ├── async/                   ← empty stubs, Phase 2
│   │   ├── AsyncManager.hpp
│   │   ├── ThreadPool.hpp
│   │   ├── AsyncJob.hpp
│   │   ├── AsyncHandle.hpp
│   │   └── MainThreadDispatcher.hpp
│   │
│   ├── pool/                    ← empty stubs, Phase 4
│   │   ├── ObjectPool.hpp
│   │   ├── PoolManager.hpp
│   │   └── Poolable.hpp
│   │
│   └── utils/
│       ├── Logger.hpp           ← Phase 0 (implement now)
│       ├── Logger.cpp
│       ├── EventBus.hpp         ← Phase 1
│       ├── Math2D.hpp           ← Phase 1
│       ├── UUID.hpp             ← Phase 1
│       └── Serializer.hpp       ← Phase 1
│
└── src/                         ← game project (separate from engine)
    └── main.cpp
```

---

## Part 3 — CMakeLists.txt

### Root CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.25)
project(ISTEngine VERSION 0.1.0 LANGUAGES CXX)

# ── C++ Standard ────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Build Type Guard ─────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ── Engine Debug Flag ────────────────────────────────
option(ENGINE_DEBUG "Enable engine debug tools" ON)
if(ENGINE_DEBUG)
    add_compile_definitions(ENGINE_DEBUG)
endif()

# ── CPM Package Manager ──────────────────────────────
include(cmake/CPM.cmake)

# ── Dependencies via CPM ─────────────────────────────
CPMAddPackage(
    NAME raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 5.0
    OPTIONS "BUILD_EXAMPLES OFF"
)

CPMAddPackage(
    NAME EnTT
    GITHUB_REPOSITORY skypjack/entt
    GIT_TAG v3.13.0
)

CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    GIT_TAG v1.13.0
)

CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    GIT_TAG v3.11.3
    OPTIONS "JSON_BuildTests OFF"
)

CPMAddPackage(
    NAME box2d
    GITHUB_REPOSITORY erincatto/box2d
    GIT_TAG v2.4.1
    OPTIONS "BOX2D_BUILD_UNIT_TESTS OFF"
            "BOX2D_BUILD_TESTBED OFF"
)

# ── Engine Source Files ───────────────────────────────
set(ENGINE_SOURCES
    # core
    engine/core/Application.cpp
    engine/core/Window.cpp
    engine/core/Timer.cpp
    # utils (Phase 0 only — expand in Phase 1)
    engine/utils/Logger.cpp
)

# ── Engine Library ────────────────────────────────────
add_library(zhenzhu-engine STATIC ${ENGINE_SOURCES})

target_include_directories(zhenzhu-engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(zhenzhu-engine PUBLIC
    raylib
    EnTT::EnTT
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    box2d
)

# ── Platform Specific Links ───────────────────────────
if(WIN32)
    target_link_libraries(zhenzhu-engine PUBLIC winmm)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(zhenzhu-engine PUBLIC
        GL m pthread dl rt X11
    )
endif()

# ── Game Executable ───────────────────────────────────
add_executable(MyGame src/main.cpp)
target_link_libraries(MyGame PRIVATE zhenzhu-engine)

# ── Copy config folder to build dir ──────────────────
add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:MyGame>/config
    COMMENT "Copying config files to build directory"
)

# ── Copy assets folder to build dir ──────────────────
add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:MyGame>/assets
    COMMENT "Copying assets to build directory"
)

# ── Compiler Warnings ─────────────────────────────────
if(MSVC)
    target_compile_options(zhenzhu-engine PRIVATE /W4)
    target_compile_options(MyGame PRIVATE /W4)
else()
    target_compile_options(zhenzhu-engine PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(MyGame PRIVATE -Wall -Wextra -Wpedantic)
endif()
```

---

### cmake/CPM.cmake
```
DO NOT write this file manually.

Download it from:
https://github.com/cpm-cmake/CPM.cmake/releases/latest

Download: CPM.cmake
Place at:  cmake/CPM.cmake

CPM auto-downloads all dependencies on first CMake configure.
They are cached in ~/.cpm or build/_deps
```

---

## Part 4 — Phase 0 Code

### engine/utils/Logger.hpp
```cpp
#pragma once
#include <string>

// Thin wrapper over spdlog
// All engine code uses these macros, never spdlog directly

namespace Zhenzhu {

class Logger {
public:
    static void Init(const std::string& logFile = "");
    static void Shutdown();

    static void Info (const std::string& msg);
    static void Warn (const std::string& msg);
    static void Error(const std::string& msg);
    static void Debug(const std::string& msg); // stripped in release
};

} // namespace Zhenzhu

// Convenience macros — use these everywhere in engine code
#define LOG_INFO(msg)  Zhenzhu::Logger::Info(msg)
#define LOG_WARN(msg)  Zhenzhu::Logger::Warn(msg)
#define LOG_ERROR(msg) Zhenzhu::Logger::Error(msg)

#ifdef ENGINE_DEBUG
    #define LOG_DEBUG(msg) Zhenzhu::Logger::Debug(msg)
#else
    #define LOG_DEBUG(msg) // stripped in release build
#endif
```

---

### engine/utils/Logger.cpp
```cpp
#include "Logger.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Zhenzhu {

void Logger::Init(const std::string& logFile) {
    // Console sink — colored output
    auto consoleSink = std::make_shared
        <spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%H:%M:%S] [%^%l%$] %v");

    std::vector<spdlog::sink_ptr> sinks { consoleSink };

    // Optional file sink
    if (!logFile.empty()) {
        auto fileSink = std::make_shared
            <spdlog::sinks::basic_file_sink_mt>(logFile, true);
        sinks.push_back(fileSink);
    }

    auto logger = std::make_shared<spdlog::logger>
        ("Zhenzhu", sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
}

void Logger::Shutdown() { spdlog::shutdown(); }
void Logger::Info (const std::string& m) { spdlog::info(m);  }
void Logger::Warn (const std::string& m) { spdlog::warn(m);  }
void Logger::Error(const std::string& m) { spdlog::error(m); }
void Logger::Debug(const std::string& m) { spdlog::debug(m); }

} // namespace Zhenzhu
```

---

### engine/core/EngineConfig.hpp
```cpp
#pragma once
#include <string>

namespace Zhenzhu {

// Populated in Phase 0 with hardcoded defaults.
// Phase 1 replaces hardcoded values with SettingsDB reads.

struct EngineConfig {
    // Display
    int         windowWidth  = 1280;
    int         windowHeight = 720;
    std::string title        = "Zhenzhu Engine";
    int         targetFPS    = 60;
    bool        fullscreen   = false;
    bool        vsync        = true;

    // Audio (used by AudioManager in Phase 5)
    float masterVolume = 1.0f;
    float sfxVolume    = 0.8f;
    float musicVolume  = 0.6f;
};

} // namespace Zhenzhu
```

---

### engine/core/Window.hpp
```cpp
#pragma once
#include "EngineConfig.hpp"
#include <string>

namespace Zhenzhu {

class Window {
public:
    void Create(const EngineConfig& config);
    void Close();

    bool ShouldClose() const;
    void SetTitle(const std::string& title);
    void SetFullscreen(bool fullscreen);

    int  GetWidth()  const;
    int  GetHeight() const;
    bool IsFullscreen() const;
};

} // namespace Zhenzhu
```

---

### engine/core/Window.cpp
```cpp
#include "Window.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Window::Create(const EngineConfig& cfg) {
    SetTargetFPS(cfg.targetFPS);

    if (cfg.vsync)
        SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(cfg.windowWidth, cfg.windowHeight, cfg.title.c_str());

    LOG_INFO("Window created: "
        + std::to_string(cfg.windowWidth) + "x"
        + std::to_string(cfg.windowHeight)
        + " | " + cfg.title);
}

void Window::Close()             { CloseWindow(); }
bool Window::ShouldClose() const { return WindowShouldClose(); }
int  Window::GetWidth()    const { return GetScreenWidth(); }
int  Window::GetHeight()   const { return GetScreenHeight(); }

void Window::SetTitle(const std::string& t) {
    SetWindowTitle(t.c_str());
}

void Window::SetFullscreen(bool fs) {
    if (fs != IsWindowFullscreen())
        ToggleFullscreen();
}

} // namespace Zhenzhu
```

---

### engine/core/Timer.hpp
```cpp
#pragma once

namespace Zhenzhu {

class Timer {
public:
    void   Tick();              // call once per frame

    float  GetDeltaTime() const;   // seconds since last frame
    float  GetFixedStep() const;   // fixed physics step (1/60s)
    float  GetElapsed()   const;   // total time since start
    int    GetFPS()        const;

    // Fixed timestep accumulator
    bool   ShouldFixedUpdate();    // call in loop until false

private:
    float  m_DeltaTime    = 0.0f;
    float  m_Elapsed      = 0.0f;
    float  m_FixedStep    = 1.0f / 60.0f;
    float  m_Accumulator  = 0.0f;
};

} // namespace Zhenzhu
```

---

### engine/core/Timer.cpp
```cpp
#include "Timer.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Timer::Tick() {
    m_DeltaTime  = GetFrameTime();
    m_Elapsed   += m_DeltaTime;
    m_Accumulator += m_DeltaTime;
}

float Timer::GetDeltaTime()  const { return m_DeltaTime; }
float Timer::GetFixedStep()  const { return m_FixedStep; }
float Timer::GetElapsed()    const { return m_Elapsed; }
int   Timer::GetFPS()        const { return GetFPS(); }

bool Timer::ShouldFixedUpdate() {
    if (m_Accumulator >= m_FixedStep) {
        m_Accumulator -= m_FixedStep;
        return true;
    }
    return false;
}

} // namespace Zhenzhu
```

---

### engine/core/ServiceLocator.hpp
```cpp
#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include "utils/Logger.hpp"

namespace Zhenzhu {

// Global access to subsystems without singletons.
// Register once at startup. Get anywhere.

class ServiceLocator {
public:
    template<typename T>
    static void Register(T* service) {
        auto id = std::type_index(typeid(T));
        s_Services[id] = service;
        LOG_DEBUG("ServiceLocator: registered " + std::string(typeid(T).name()));
    }

    template<typename T>
    static T* Get() {
        auto id = std::type_index(typeid(T));
        auto it = s_Services.find(id);
        if (it == s_Services.end()) {
            LOG_ERROR("ServiceLocator: service not found: "
                + std::string(typeid(T).name()));
            return nullptr;
        }
        return static_cast<T*>(it->second);
    }

    static void Clear() { s_Services.clear(); }

private:
    static inline std::unordered_map<std::type_index, void*> s_Services;
};

} // namespace Zhenzhu
```

---

### engine/core/Application.hpp
```cpp
#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include "EngineConfig.hpp"

namespace Zhenzhu {

class Application {
public:
    void Init();
    void Run();
    void Shutdown();

    static void Quit();          // call from anywhere to exit loop

private:
    void ProcessInput();
    void Update(float dt);
    void FixedUpdate();
    void Render();

    Window        m_Window;
    Timer         m_Timer;
    EngineConfig  m_Config;

    static inline bool s_Running = false;
};

} // namespace Zhenzhu
```

---

### engine/core/Application.cpp
```cpp
#include "Application.hpp"
#include "ServiceLocator.hpp"
#include "utils/Logger.hpp"
#include <raylib.h>

namespace Zhenzhu {

void Application::Init() {
    Logger::Init("engine.log");
    LOG_INFO("=== Zhenzhu Engine v0.1.0 ===");
    LOG_INFO("Phase 0 — Project Foundation");

    // Phase 1 will replace this with DataManager reads
    m_Config.windowWidth  = 1280;
    m_Config.windowHeight = 720;
    m_Config.title        = "Zhenzhu Engine";
    m_Config.targetFPS    = 60;

    m_Window.Create(m_Config);

    // Register core services
    ServiceLocator::Register(&m_Window);
    ServiceLocator::Register(&m_Timer);

    s_Running = true;
    LOG_INFO("Application initialized");
}

void Application::Run() {
    LOG_INFO("Entering main loop...");

    while (s_Running && !m_Window.ShouldClose()) {
        m_Timer.Tick();
        float dt = m_Timer.GetDeltaTime();

        ProcessInput();

        // Fixed timestep loop (for physics in Phase 4)
        while (m_Timer.ShouldFixedUpdate()) {
            FixedUpdate();
        }

        Update(dt);
        Render();
    }

    LOG_INFO("Main loop exited");
}

void Application::Shutdown() {
    LOG_INFO("Shutting down...");
    ServiceLocator::Clear();
    m_Window.Close();
    Logger::Shutdown();
}

void Application::Quit() {
    s_Running = false;
}

void Application::ProcessInput() {
    // Phase 3 — InputManager.Update() goes here
}

void Application::Update(float dt) {
    // Phase 5 — SceneManager.Update(dt) goes here
    (void)dt;
}

void Application::FixedUpdate() {
    // Phase 4 — PhysicsWorld2D.Step() goes here
}

void Application::Render() {
    BeginDrawing();
        ClearBackground({ 20, 20, 25, 255 });

        // Phase 5 — SceneManager.Render() goes here

        // Phase 0 — temp debug info
#ifdef ENGINE_DEBUG
        DrawFPS(10, 10);
        DrawText("Zhenzhu Engine — Phase 0", 10, 40, 20, GRAY);
#endif

    EndDrawing();
}

} // namespace Zhenzhu
```

---

### src/main.cpp
```cpp
#include "core/Application.hpp"

int main() {
    Zhenzhu::Application app;
    app.Init();
    app.Run();
    app.Shutdown();
    return 0;
}
```

---

## Part 5 — Stub Headers (Empty But Valid)

> All future-phase headers must exist NOW so CMake doesn't break.
> Each one is just a pragma once + empty class.

```cpp
// Example stub — same pattern for ALL empty headers

// engine/data/DataManager.hpp
#pragma once
namespace Zhenzhu {
class DataManager {
    // Phase 1
};
} // namespace Zhenzhu


// engine/renderer/Renderer2D.hpp
#pragma once
namespace Zhenzhu {
class Renderer2D {
    // Phase 3
};
} // namespace Zhenzhu


// engine/ecs/Registry.hpp
#pragma once
namespace Zhenzhu {
class Registry {
    // Phase 4
};
} // namespace Zhenzhu

// ... same for all other empty headers
```

---

## Part 6 — Build & Run in CLion

```
FIRST TIME SETUP:

1. Open CLion
2. File → Open → select zhenzhu-engine/ folder
3. CLion detects CMakeLists.txt automatically
4. CMake tool window appears at bottom
   Click: "Reload CMake Project"
   CPM downloads all vendors (takes 2–5 mins first time)

5. Top right dropdown → select "Debug" profile
6. Top right dropdown → select "MyGame" target
7. Click green ▶ Run button

EXPECTED OUTPUT:
  Console:
    [HH:MM:SS] [info] === Zhenzhu Engine v0.1.0 ===
    [HH:MM:SS] [info] Phase 0 — Project Foundation
    [HH:MM:SS] [info] Window created: 1280x720 | Zhenzhu Engine
    [HH:MM:SS] [info] Application initialized
    [HH:MM:SS] [info] Entering main loop...

  Window:
    Dark background opens
    FPS counter top-left
    "Zhenzhu Engine — Phase 0" text
```

---

## Part 7 — Phase 0 Checklist

```
CLION:
  □ CMake Debug + Release profiles configured
  □ Ninja generator set
  □ .clang-format added + format on save enabled
  □ Plugins installed
  □ Correct toolchain (MSVC on Windows / clang on Mac/Linux)

PROJECT:
  □ CMakeLists.txt written
  □ cmake/CPM.cmake downloaded and placed
  □ .gitignore written
  □ .clang-format written
  □ All folders created

DEPENDENCIES (CPM pulls these — verify in build/_deps):
  □ raylib      5.0
  □ EnTT        v3.13.0
  □ spdlog      v1.13.0
  □ nlohmann_json v3.11.3
  □ box2d       v2.4.1

CODE:
  □ Logger.hpp + Logger.cpp
  □ EngineConfig.hpp
  □ Window.hpp + Window.cpp
  □ Timer.hpp + Timer.cpp
  □ ServiceLocator.hpp
  □ Application.hpp + Application.cpp
  □ main.cpp
  □ All stub headers (empty but valid)

VERIFIED:
  □ CMake configures without errors
  □ Project compiles without errors
  □ Window opens at 1280x720
  □ FPS shows in top left
  □ Logger prints to console
  □ Close button exits cleanly
  □ No memory leaks (run with ASAN in debug)
```

---

## What Phase 1 Gets For Free

```
Phase 1 immediately has:
  ✅ Working build system (just add files to CMake)
  ✅ Logger ready to use everywhere
  ✅ ServiceLocator to register DataManager
  ✅ EngineConfig to populate from JSON
  ✅ Window ready to receive new size/title from SettingsDB
  ✅ Application.Init() has clear slot to call DataManager.Init()
  ✅ All stub headers exist — no missing file errors
```

Want me to write the full Phase 1 plan next?