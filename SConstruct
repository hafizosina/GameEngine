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
    '.',
    'vendor/raylib/src',
    'vendor/box2d/build/bin'
])

# Libraries to link
env.Append(LIBS=['raylib', 'box2d'])

# Platform-specific links (Linux/macOS)
if env['PLATFORM'] == 'posix' or env['PLATFORM'] == 'darwin':
    env.Append(LIBS=['GL', 'm', 'pthread', 'dl', 'rt', 'X11'])

# Gather source files
engine_src = Glob('engine/core/*.cpp') + Glob('engine/utils/*.cpp')
game_src = Glob('src/*.cpp')

# Build the engine as a static library
engine_lib = env.StaticLibrary('zhenzhu-engine', engine_src)

# Build the main game executable
# Note: Since the engine depends on vendor libs, we link them into the final executable as well
# SCons handles LIBS order properly
env.Prepend(LIBS=['zhenzhu-engine'])
game = env.Program('MyGame', game_src)

# Default target
Default(game)
