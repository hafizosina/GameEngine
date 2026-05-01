import os

env = Environment()

# C++ Standard and Compiler flags
env.Append(CXXFLAGS=['-std=c++20', '-Wall', '-Wextra', '-Wpedantic'])

# Debug flag
env.Append(CPPDEFINES=['ENGINE_DEBUG'])

# Include directories
env.Append(CPPPATH=[
    'engine',
    '.',
    'vendor/nlohmann_json/single_include',
    'vendor/entt/src',
    'vendor/spdlog/include',
    'vendor/raylib/src',
    'vendor/box2d/include'
])

# Library directories
env.Append(LIBPATH=[
    'build',
    'vendor/raylib/src',
    'vendor/box2d/build/bin'
])

# Libraries to link
env.Append(LIBS=['raylib', 'box2d'])

# Platform-specific links (Linux/macOS)
if env['PLATFORM'] == 'posix' or env['PLATFORM'] == 'darwin':
    env.Append(LIBS=['GL', 'm', 'pthread', 'dl', 'rt', 'X11'])

# ── Build Setup ──────────────────────────────────
# Redirect all compiled objects to build/ directory
env.VariantDir('build/engine', 'engine', duplicate=0)
env.VariantDir('build/src',    'src',    duplicate=0)

engine_src = (
    Glob('build/engine/core/*.cpp') +
    Glob('build/engine/utils/*.cpp') +
    Glob('build/engine/async/*.cpp') +
    Glob('build/engine/resources/*.cpp') +
    Glob('build/engine/assets/*.cpp') +
    Glob('build/engine/renderer/*.cpp') +
    Glob('build/engine/input/*.cpp') +
    Glob('build/engine/physics/*.cpp')
)
game_src   = Glob('build/src/*.cpp')

# Build the engine as a static library in build/
engine_lib = env.StaticLibrary('build/zhenzhu-engine', engine_src)

# Build the main game executable in build/
env.Prepend(LIBS=['zhenzhu-engine'])
game = env.Program('build/MyGame', game_src)

# Create a symlink in root for convenience (optional)
# env.Command('MyGame', game, 'ln -sf build/MyGame MyGame')

# Default target
Default(game)
