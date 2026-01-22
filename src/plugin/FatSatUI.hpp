/*
 * FatSat UI - Godot-based user interface with headless rendering
 * Part of the Enlil/GodotVST Framework
 */

#ifndef FATSAT_UI_HPP
#define FATSAT_UI_HPP

// GLX types for context management (must come before DPF headers)
#if !defined(__APPLE__)
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

#include "DistrhoUI.hpp"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/godot_instance.hpp>

START_NAMESPACE_DISTRHO

// LibGodot API function pointer types (simplified 3-arg signature)
typedef GDExtensionObjectPtr (*LibGodotCreateInstanceFunc)(
    int p_argc, char* p_argv[],
    GDExtensionInitializationFunction p_init_func
);
typedef void (*LibGodotDestroyInstanceFunc)(GDExtensionObjectPtr p_godot_instance);

class FatSatUI : public UI {
public:
    FatSatUI();
    ~FatSatUI() override;

protected:
    // DPF UI callbacks
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char* key, const char* value) override;
    void uiIdle() override;
    void onDisplay() override;
    void uiReshape(uint width, uint height) override;

    // Input event handlers
    bool onMouse(const MouseEvent& ev) override;
    bool onMotion(const MotionEvent& ev) override;
    bool onScroll(const ScrollEvent& ev) override;
    bool onKeyboard(const KeyboardEvent& ev) override;

private:
    // LibGodot management
    bool loadLibGodot();
    void unloadLibGodot();
    bool initGodot();
    void shutdownGodot();

    // OpenGL helpers
    void initOpenGL();
    void cleanupOpenGL();
    void uploadFrameTexture();
    void drawFullscreenQuad();

    // LibGodot library handle and function pointers
    void* fLibGodotHandle;
    LibGodotCreateInstanceFunc fCreateInstance;
    LibGodotDestroyInstanceFunc fDestroyInstance;

    // Godot instance (using godot-cpp wrapper)
    godot::GodotInstance* fGodotInstance;
    bool fGodotStarted;

    // DPF window info (no longer used for embedding, kept for reference)
    uintptr_t fParentWindowId;

    // OpenGL texture for frame display
    unsigned int fFrameTexture;
    bool fOpenGLInitialized;
    int fTextureWidth;
    int fTextureHeight;

    // Cached parameter values
    float fCurrentFatness;
    float fCurrentOutput;

    // Mouse state tracking
    float fLastMouseX;
    float fLastMouseY;

    // Frame skip counter for Godot initialization
    int fFrameSkipCount;

#if !defined(__APPLE__)
    // Godot's GLX context info - captured after start() so we can restore it
    Display* fGodotDisplay;
    GLXDrawable fGodotDrawable;
    GLXContext fGodotContext;
#endif

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FatSatUI)
};

END_NAMESPACE_DISTRHO

#endif // FATSAT_UI_HPP
