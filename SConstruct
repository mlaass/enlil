#!/usr/bin/env python
"""
Enlil/GodotVST Framework - Main Build Configuration

This SCons build system compiles:
1. LibGodot (stripped Godot as shared library)
2. godot-cpp bindings
3. FatSat GDExtension bridge
4. FatSat DPF plugin (VST3/CLAP/LV2)
"""

import os
import sys
import subprocess

# Ensure we're using Python 3
if sys.version_info < (3, 6):
    print("Python 3.6+ is required")
    Exit(1)

# Project paths
PROJECT_ROOT = Dir('.').abspath
DPF_PATH = os.path.join(PROJECT_ROOT, 'dpf')
GODOT_PATH = os.path.join(PROJECT_ROOT, 'godot')
GODOT_CPP_PATH = os.path.join(PROJECT_ROOT, 'godot-cpp')
SRC_PATH = os.path.join(PROJECT_ROOT, 'src')
BUILD_PATH = os.path.join(PROJECT_ROOT, 'build')

# Build environment
env = Environment(ENV=os.environ)

# Build options
opts = Variables('custom.py')
opts.Add(EnumVariable('target', 'Build target', 'debug',
    allowed_values=('debug', 'release')))
opts.Add(EnumVariable('platform', 'Target platform', '',
    allowed_values=('', 'linux', 'macos', 'windows')))
opts.Add(BoolVariable('verbose', 'Enable verbose output', False))
opts.Add(BoolVariable('build_godot', 'Build LibGodot from source', True))
opts.Add(PathVariable('godot_bin', 'Path to pre-built LibGodot', '',
    PathVariable.PathAccept))
opts.Update(env)

# Auto-detect platform
if env['platform'] == '':
    if sys.platform.startswith('linux'):
        env['platform'] = 'linux'
    elif sys.platform == 'darwin':
        env['platform'] = 'macos'
    elif sys.platform == 'win32':
        env['platform'] = 'windows'
    else:
        print(f"Unsupported platform: {sys.platform}")
        Exit(1)

print(f"Building for {env['platform']} ({env['target']})")

# Create build directories
for subdir in ['godot', 'godot-cpp', 'bridge', 'plugin']:
    os.makedirs(os.path.join(BUILD_PATH, subdir), exist_ok=True)

# ============================================================================
# Phase 1: Build LibGodot (stripped)
# ============================================================================

if env['build_godot']:
    godot_target = 'template_debug' if env['target'] == 'debug' else 'template_release'

    godot_scons_opts = [
        f'platform={env["platform"]}',
        f'target={godot_target}',
        'library_type=shared_library',
        'optimize=size',
        'disable_3d=yes',
        'module_bullet_enabled=no',
        'module_navigation_enabled=no',
        'module_openxr_enabled=no',
        'module_multiplayer_enabled=no',
        'module_text_server_adv_enabled=no',
        'deprecated=no',
    ]

    if env['target'] == 'release':
        godot_scons_opts.append('production=yes')
        godot_scons_opts.append('debug_symbols=no')

    godot_build_cmd = f'cd {GODOT_PATH} && scons {" ".join(godot_scons_opts)}'

    if env['verbose']:
        print(f"Godot build command: {godot_build_cmd}")

    godot_lib = env.Command(
        target=os.path.join(BUILD_PATH, 'godot', 'libgodot.so'),
        source=os.path.join(GODOT_PATH, 'SConstruct'),
        action=godot_build_cmd
    )
    env.Alias('godot', godot_lib)

# ============================================================================
# Phase 2: Build godot-cpp
# ============================================================================

godot_cpp_target = 'template_debug' if env['target'] == 'debug' else 'template_release'

godot_cpp_scons_opts = [
    f'platform={env["platform"]}',
    f'target={godot_cpp_target}',
]

godot_cpp_build_cmd = f'cd {GODOT_CPP_PATH} && scons {" ".join(godot_cpp_scons_opts)}'

if env['verbose']:
    print(f"godot-cpp build command: {godot_cpp_build_cmd}")

godot_cpp_lib = env.Command(
    target=os.path.join(BUILD_PATH, 'godot-cpp', 'libgodot-cpp.a'),
    source=os.path.join(GODOT_CPP_PATH, 'SConstruct'),
    action=godot_cpp_build_cmd
)
env.Alias('godot-cpp', godot_cpp_lib)

# ============================================================================
# Phase 3: Build FatSat Bridge (GDExtension)
# ============================================================================

bridge_env = env.Clone()

# Include paths
bridge_env.Append(CPPPATH=[
    os.path.join(GODOT_CPP_PATH, 'include'),
    os.path.join(GODOT_CPP_PATH, 'gen', 'include'),
    os.path.join(GODOT_CPP_PATH, 'gdextension'),
    os.path.join(SRC_PATH, 'bridge'),
])

# Library paths and full library name
godot_cpp_lib_name = f'godot-cpp.{env["platform"]}.{godot_cpp_target}.x86_64'
bridge_env.Append(LIBPATH=[
    os.path.join(GODOT_CPP_PATH, 'bin'),
])

# Compiler flags
bridge_env.Append(CXXFLAGS=['-std=c++17', '-fPIC'])
if env['target'] == 'debug':
    bridge_env.Append(CXXFLAGS=['-g', '-O0'])
else:
    bridge_env.Append(CXXFLAGS=['-O2'])

# Source files
bridge_sources = [
    os.path.join(SRC_PATH, 'bridge', 'register_types.cpp'),
    os.path.join(SRC_PATH, 'bridge', 'fatsat_bridge.cpp'),
]

# Build bridge library
bridge_lib_name = f'libfatsat_bridge.{env["platform"]}.{godot_cpp_target}.x86_64.so'
bridge_lib = bridge_env.SharedLibrary(
    target=os.path.join(BUILD_PATH, 'bridge', bridge_lib_name),
    source=bridge_sources,
    LIBS=[godot_cpp_lib_name]
)
bridge_env.Depends(bridge_lib, godot_cpp_lib)
env.Alias('bridge', bridge_lib)

# ============================================================================
# Phase 4: Build FatSat Plugin (DPF)
# ============================================================================

# DPF uses its own Makefile system - we invoke it
plugin_build_cmd = f'cd {SRC_PATH}/plugin && make DPF_PATH={DPF_PATH}'

plugin_vst3 = env.Command(
    target=os.path.join(BUILD_PATH, 'plugin', 'FatSat.vst3'),
    source=[
        os.path.join(SRC_PATH, 'plugin', 'FatSatPlugin.cpp'),
        os.path.join(SRC_PATH, 'plugin', 'FatSatUI.cpp'),
    ],
    action=plugin_build_cmd
)
env.Alias('plugin', plugin_vst3)

# ============================================================================
# Default targets
# ============================================================================

Default('bridge')

# Help message
Help("""
Enlil/GodotVST Framework Build System

Targets:
    godot       - Build LibGodot (stripped)
    godot-cpp   - Build godot-cpp bindings
    bridge      - Build FatSat GDExtension bridge (default)
    plugin      - Build FatSat DPF plugin

Options:
    target=debug|release    - Build type (default: debug)
    platform=linux|macos|windows - Target platform (auto-detected)
    verbose=yes|no          - Show build commands (default: no)
    build_godot=yes|no      - Build LibGodot from source (default: yes)

Examples:
    scons                   - Build bridge (debug)
    scons target=release    - Build bridge (release)
    scons godot             - Build LibGodot
    scons -j8               - Parallel build with 8 jobs
""")
