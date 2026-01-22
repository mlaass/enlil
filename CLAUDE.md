# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Enlil** is the GodotVST Framework - a template for building audio plugins (VST3/CLAP/LV2) that use the Godot game engine for rich, animated UIs. The pilot plugin "FatSat" is a one-knob saturation/dynamics effect.

See `docs/PRD_ GodotVST Framework & FatSat Pilot.md` for the full specification.

## Architecture

| Component | Technology | Role |
|-----------|------------|------|
| Host Shell | DPF (DISTRHO Plugin Framework) | VST3/CLAP/LV2 entry points, OS window creation, audio processing |
| Guest UI | LibGodot (Godot 4.x as shared library) | Renders UI into DPF window, animations, 2D graphics |
| Bridge | GDExtension (C++) | Thread-safe communication between DSP and UI |
| UI Logic | GDScript | Knob interactions, animations, visual feedback |

## Threading Model (Critical)

**Audio Thread (Real-time):**
- Runs DSP, writes visualization data to lock-free ring buffer
- Reads atomic parameter values
- **NEVER locks, NEVER allocates memory**

**UI Thread (Main):**
- Godot 60fps frame loop
- Polls ring buffer for visualization data (RMS levels, etc.)
- Writes parameter changes as atomic values

## Build System

Uses SCons to compile:
- Custom LibGodot with disabled modules (3D, physics, navigation, XR, multiplayer)
- DPF framework integration
- GDExtension bridge code

Target binary size: <30MB (stripped Godot)

**Important:** Always use `make` targets for building - never run `scons` directly. Run `make help` to see available targets.

## Key Constraints

- **Single binary distribution:** Godot .pck assets embedded as static byte array in C++
- **Window parenting:** DPF creates OS window (HWND/Cocoa/X11), LibGodot renders into it
- **Cross-platform:** Windows, macOS, Linux support required

## FatSat DSP Algorithm

```
Input → Gain Stage → tanh(input * drive) → Ceiling Limiter (-0.1dB) → Output Gain (~50% compensation)
```

Parameters: FATNESS (0.0-1.0), OUTPUT (0.0-1.0)
