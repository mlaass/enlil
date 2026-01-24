# PRD: LibGodot Window Embedding

**Status:** Proposal
**Target:** Godot 4.x LibGodot API
**Author:** Godot Community
**Date:** January 2026

---

## 1. Problem Statement

### 1.1 The Core Issue

LibGodot's public C API does **not** support passing an external window handle for embedded rendering. Host applications—whether audio plugin hosts, IDE panels, kiosk systems, or hybrid desktop frameworks—create their own OS windows and expect embedded UIs to render directly into them. Godot's current architecture cannot satisfy this requirement.

### 1.2 Why `--wid` Doesn't Work

The `--wid` command-line argument appears to support window embedding but fails for embedded rendering scenarios:

| Platform | `--wid` Behavior | Actual Result |
|----------|------------------|---------------|
| **Linux/X11** | Calls `XSetTransientForHint()` | Creates WM relationship only; renders to separate window |
| **Windows** | Sets `GWLP_HWNDPARENT` | Designed for embedding external processes, not rendering into parent |
| **macOS** | N/A | Uses `DisplayServerEmbedded` with CALayer (most mature) |

**Result:** On Linux and Windows, Godot creates a visually separate window that floats near the parent rather than rendering pixels directly into the parent window's graphics context.

### 1.3 Use Cases Requiring Window Embedding

Many application domains require embedding Godot rendering into a parent window:

**Audio/Video Plugins (VST3, AU, CLAP, LV2)**
- Digital audio workstations provide a window handle; plugin UIs must render into it
- No option for floating windows—embedding is mandatory

**IDE & Editor Integrations**
- Game preview panels within development environments
- Asset browsers with interactive 3D previews
- Visual scripting node editors

**Embedded Applications**
- Kiosk displays with touch interfaces
- Automotive HMI (human-machine interface) systems
- Industrial control panels

**Hybrid Desktop Applications**
- Godot rendering within Electron, Qt, or GTK applications
- Native app shells hosting Godot content
- Multi-engine compositing scenarios

Current workarounds include:

1. **Headless Blit:** Run Godot in headless mode, render to `SubViewport`, copy RGBA pixels to host's graphics context
2. **Separate Window:** Open Godot as a floating window (unacceptable UX for most use cases)
3. **Abandon Godot:** Use traditional UI frameworks (ImGui, Qt, etc.)

The headless blit approach works but introduces:
- 1 frame of latency due to double buffering
- ~0.5ms CPU overhead per frame for image copy
- Platform-specific texture upload code
- No GPU resource sharing between Godot and host

---

## 2. Current Architecture Analysis

### 2.1 LibGodot Public API

The LibGodot C API (introduced in Godot 4.x) provides:

```c
// From godot/platform/libgodot/libgodot.h
typedef void (*GDExtensionInitializationFunction)(
    void* userdata,
    GDExtensionInitializationLevel level
);

godot_error godot_main(int argc, char* argv[]);
```

**Missing:** No mechanism to pass a parent window handle during initialization.

### 2.2 DisplayServer Internals

Godot's `DisplayServer` class handles window creation with a `p_parent_window` parameter that is **not exposed** through the public LibGodot API:

```cpp
// platform/linuxbsd/x11/display_server_x11.cpp
DisplayServerX11::DisplayServerX11(
    const String &p_rendering_driver,
    WindowMode p_mode,
    VSyncMode p_vsync_mode,
    uint32_t p_flags,
    const Vector2i *p_position,
    const Vector2i &p_resolution,
    int p_screen,
    Context p_context,
    Error &r_error,
    int64_t p_parent_window  // <-- Exists internally but not exposed
)
```

The internal `p_parent_window` parameter exists but:
- Is not passed through the LibGodot initialization path
- Has platform-specific interpretations that don't achieve true embedding
- Lacks corresponding rendering context sharing

### 2.3 Platform-Specific Implementations

#### Linux/X11 (`display_server_x11.cpp`)

```cpp
if (p_parent_window != 0) {
    XSetTransientForHint(x11_display, wd.x11_window, (Window)p_parent_window);
}
```

This only sets a window manager hint. The Godot window remains a separate X11 window with its own graphics context.

#### Windows (`display_server_windows.cpp`)

```cpp
if (p_parent_window != 0) {
    SetWindowLongPtr(wd.hWnd, GWLP_HWNDPARENT, (LONG_PTR)p_parent_window);
}
```

Sets the owner window for Z-ordering but doesn't embed the rendering surface.

#### macOS (`DisplayServerEmbedded`)

macOS has the most mature embedded display server implementation using CALayer/CAContext for cross-process rendering. This architectural pattern should be extended to other platforms.

---

## 3. Proposed Solution

### 3.1 Overview

Extend the LibGodot API to accept a parent window handle and implement platform-specific `DisplayServerEmbedded` backends that render directly into the provided surface.

### 3.2 API Extension

```c
// New LibGodot API addition
typedef struct {
    int64_t parent_window_handle;  // HWND, X11 Window, NSView*, etc.
    int width;
    int height;
    bool transparent;
    GDExtensionInitializationFunction init_callback;
    void* userdata;
} GodotEmbeddedConfig;

godot_error godot_embedded_init(const GodotEmbeddedConfig* config);
godot_error godot_embedded_iterate();  // Call each frame from host
godot_error godot_embedded_resize(int width, int height);
godot_error godot_embedded_shutdown();
```

### 3.3 DisplayServerEmbedded Architecture

Following the macOS pattern, implement `DisplayServerEmbedded` for each platform:

```
┌─────────────────────────────────────────────────────────────┐
│                    LibGodot Embedded Mode                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   godot_embedded_init(config)                               │
│         │                                                   │
│         ▼                                                   │
│   ┌─────────────────────────────────────────────────────┐  │
│   │           DisplayServerEmbedded                      │  │
│   │  ┌─────────────────────────────────────────────┐    │  │
│   │  │  Platform-Specific Renderer                 │    │  │
│   │  │                                             │    │  │
│   │  │  Linux: Shared GLX Context / Vulkan FD     │    │  │
│   │  │  Windows: HWND Child + WGL/D3D Interop     │    │  │
│   │  │  macOS: CALayer (existing)                 │    │  │
│   │  │                                             │    │  │
│   │  └─────────────────────────────────────────────┘    │  │
│   └─────────────────────────────────────────────────────┘  │
│                                                             │
│   Host calls godot_embedded_iterate() in render loop       │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 3.4 Platform Implementation Details

#### 3.4.1 Linux/X11 Implementation

**Option A: Shared GLX Context**
- Host provides GLX context and drawable
- Godot renders using shared context
- No pixel copy overhead

**Option B: Reparented Window**
- Create Godot X11 window with override-redirect
- Reparent into host window using `XReparentWindow()`
- Handle expose/configure events

**Option C: Off-screen with DMA-BUF**
- Render to GPU buffer
- Share via DMA-BUF/EGLImage
- Host composites with zero-copy

#### 3.4.2 Windows Implementation

**Child Window Approach:**
```cpp
HWND childHwnd = CreateWindowEx(
    WS_EX_LAYERED,
    "GodotEmbedded",
    nullptr,
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
    0, 0, width, height,
    parentHwnd,  // Host-provided handle
    nullptr, nullptr, nullptr
);
```

**Graphics Interop:**
- WGL: `wglShareLists()` for OpenGL resource sharing
- D3D11: `IDXGIKeyedMutex` for cross-adapter sharing
- Vulkan: External memory extension

#### 3.4.3 macOS Implementation

Existing `DisplayServerEmbedded` using `CAContextID` and `CALayer`:
- `CAContext` creates a shareable rendering surface
- Host creates `CALayerHost` with Godot's context ID
- GPU-accelerated compositing with no copy

This implementation is already functional and serves as the reference for other platforms.

---

## 4. Implementation Roadmap

### Phase 1: API Design & Prototyping (4-6 weeks)

1. **Define Public API**
   - RFC for `GodotEmbeddedConfig` structure
   - Event forwarding interface design
   - Lifecycle management (init/iterate/shutdown)

2. **Prototype Linux Reparenting**
   - `XReparentWindow()` based embedding
   - Input event forwarding via `XSendEvent()`
   - Validate with integration test harness

3. **Document macOS Reference**
   - Extract `DisplayServerEmbedded` patterns
   - Create architecture documentation

### Phase 2: Core Implementation (8-12 weeks)

1. **DisplayServerEmbedded Base Class**
   - Common interface for all platforms
   - Event translation layer
   - Resize handling

2. **Windows Implementation**
   - Child HWND approach
   - WGL context sharing
   - Input forwarding

3. **Linux X11 Implementation**
   - Reparented window approach
   - GLX context handling
   - XEmbed protocol support (optional)

### Phase 3: Advanced Features (4-6 weeks)

1. **Vulkan Support**
   - External memory sharing
   - Cross-platform rendering path

2. **Wayland Support**
   - Subsurface approach
   - wl_subsurface protocol

3. **High-DPI Handling**
   - Scale factor negotiation
   - Retina/HiDPI awareness

### Phase 4: Testing & Documentation (2-4 weeks)

1. **Integration Tests**
   - Host application compatibility testing
   - Cross-platform embedding validation
   - Standalone integration test harness

2. **Documentation**
   - API reference
   - Platform-specific notes
   - Migration guide from headless blit

---

## 5. GitHub Issues Reference

### 5.1 Core LibGodot PRs

| Issue | Title | Relevance |
|-------|-------|-----------|
| [#72883](https://github.com/godotengine/godot/pull/72883) | LibGodot with GDExtension | Main LibGodot implementation; defines current C API |
| [#90510](https://github.com/godotengine/godot/pull/90510) | Migeran LibGodot Feature | Extended LibGodot capabilities; potential API extensions |

### 5.2 Window Embedding

| Issue | Title | Relevance |
|-------|-------|-----------|
| [#99010](https://github.com/godotengine/godot/pull/99010) | Embed game process in editor (Windows/X11) | Shows embedding approach for editor; similar requirements |
| [#105884](https://github.com/godotengine/godot/pull/105884) | macOS: Embedded window support (CALayer) | Reference implementation for DisplayServerEmbedded |
| [#56785](https://github.com/godotengine/godot/issues/56785) | Native display/window/view handles API | Request for handle access; foundational for embedding |

### 5.3 Known Issues

| Issue | Title | Relevance |
|-------|-------|-----------|
| [#103171](https://github.com/godotengine/godot/issues/103171) | Embedded window only supports Windowed mode | Fullscreen limitation in embedded context |
| [#102236](https://github.com/godotengine/godot/issues/102236) | Embed Game unavailable when multi-window disabled | Single-window mode conflict |
| [#105349](https://github.com/godotengine/godot/issues/105349) | Viewport doesn't update on embedded window resize | Resize handling bug in embedded mode |
| [#68525](https://github.com/godotengine/godot/issues/68525) | `window_get_native_handle` crashes | Native handle access stability |

### 5.4 Proposals & Feature Requests

| Issue | Title | Relevance |
|-------|-------|-----------|
| [#7242](https://github.com/godotengine/godot-proposals/issues/7242) | Direct access to embedded windows | API for embedded window management |
| [#4741](https://github.com/godotengine/godot-proposals/issues/4741) | Per-window embedding configuration | Flexible embedding options |
| [#7213](https://github.com/godotengine/godot-proposals/issues/7213) | Embedding game window in editor | Editor integration pattern |
| [#5790](https://github.com/godotengine/godot-proposals/issues/5790) | Off-screen rendering support | Headless rendering proposal |

---

## 6. Alternatives Considered

### 6.1 Headless Blit (Current Implementation)

**Description:** Run Godot in headless mode, render to SubViewport, copy RGBA pixels to host's graphics context.

**Pros:**
- Works today with no Godot modifications
- Portable across all platforms
- No window manager dependencies

**Cons:**
- 1 frame latency from double buffering
- CPU overhead for pixel copy (~0.5ms/frame at 600x400)
- No GPU resource sharing
- Manual input forwarding required

**Status:** Viable interim solution for projects that cannot wait for native embedding support.

### 6.2 Shared Memory / IPC

**Description:** Run Godot as separate process, share frame buffer via shared memory.

**Pros:**
- Process isolation
- Crash isolation

**Cons:**
- Complex IPC for input/resize events
- Synchronization overhead
- Two processes for one embedded component (resource waste)

**Status:** Rejected for most use cases - unnecessary complexity for tightly integrated embedding.

### 6.3 Vulkan/Metal Interop

**Description:** Share GPU textures between Godot and host using cross-process GPU APIs.

**Pros:**
- Zero-copy rendering
- GPU-accelerated

**Cons:**
- Requires both Godot and host to use compatible APIs
- Complex external memory handling
- Not all hosts support Vulkan

**Status:** Future enhancement after basic embedding works.

### 6.4 Native UI Frameworks

**Description:** Abandon Godot for UI, use ImGui/Qt/native widgets.

**Pros:**
- Mature embedding support
- Known working solutions

**Cons:**
- Loses Godot's animation system
- Loses Godot Editor workflow
- Loses game-engine rendering capabilities

**Status:** Viable alternative when Godot's unique features are not required.

---

## 7. Success Criteria

1. **API Stability:** Defined public C API that application developers can rely on
2. **Platform Coverage:** Working implementation on Linux, Windows, macOS
3. **Performance:** No frame copy overhead; GPU resource sharing where possible
4. **Compatibility:** Works with major host application types (audio hosts, IDEs, desktop frameworks)
5. **Input Fidelity:** Mouse, keyboard, scroll, drag-drop events properly forwarded

---

## 8. Open Questions

1. **Upstream Acceptance:** Will Godot maintainers accept DisplayServerEmbedded for all platforms?
2. **Vulkan Priority:** Should Vulkan be primary renderer for embedding?
3. **Wayland Support:** X11 reparenting doesn't apply; requires different approach
4. **Multi-Instance:** How to handle multiple embedded Godot instances (multi-plugin hosts)?

---

## 9. References

- [LibGodot PR #72883](https://github.com/godotengine/godot/pull/72883) - Core LibGodot implementation
- [DisplayServerEmbedded (macOS)](https://github.com/godotengine/godot/tree/master/platform/macos) - Reference implementation
- [Godot Proposals #7242](https://github.com/godotengine/godot-proposals/issues/7242) - Direct access to embedded windows
- [XEmbed Protocol Specification](https://specifications.freedesktop.org/xembed-spec/xembed-spec-latest.html) - X11 embedding standard
