/*
 * FatSat UI - Godot-based user interface using headless rendering
 * Part of the Enlil/GodotVST Framework
 *
 * Godot runs headless, renders to SubViewport, pixels are blitted to DPF's OpenGL context.
 * Input events from DPF are forwarded to Godot via FrameBridge.
 */

// X11/GLX headers must come BEFORE DPF headers to avoid Window naming conflict
#if !defined(__APPLE__)
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

#include "FatSatUI.hpp"
#include "DistrhoPluginInfo.h"
#include "../shared/frame_bridge.hpp"
#include "../bridge/fatsat_bridge.hpp"
#include "../bridge/frame_bridge_gd.hpp"

#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>

// OpenGL headers
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

START_NAMESPACE_DISTRHO

// GDExtension initialization callbacks (using godot-cpp pattern)
extern "C" {

static void fatsat_initialize_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    // Register GDExtension classes so GDScript can use them
    godot::ClassDB::register_class<godot::FatSatBridge>();
    godot::ClassDB::register_class<godot::FrameBridgeGD>();

    fprintf(stdout, "[FatSat] GDExtension module initialized - registered FatSatBridge and FrameBridgeGD\n");
}

static void fatsat_uninitialize_module(godot::ModuleInitializationLevel p_level) {
    if (p_level != godot::MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    fprintf(stdout, "[FatSat] GDExtension module uninitialized\n");
}

GDExtensionBool GDE_EXPORT fatsat_gdextension_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization* r_initialization)
{
    fprintf(stdout, "[FatSat] GDExtension init called\n");

    godot::GDExtensionBinding::InitObject init_object(p_get_proc_address, p_library, r_initialization);

    init_object.register_initializer(fatsat_initialize_module);
    init_object.register_terminator(fatsat_uninitialize_module);
    init_object.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_object.init();
}

} // extern "C"

FatSatUI::FatSatUI()
    : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT),
      fLibGodotHandle(nullptr),
      fCreateInstance(nullptr),
      fDestroyInstance(nullptr),
      fGodotInstance(nullptr),
      fGodotStarted(false),
      fParentWindowId(0),
      fFrameTexture(0),
      fOpenGLInitialized(false),
      fTextureWidth(0),
      fTextureHeight(0),
      fCurrentFatness(0.0f),
      fCurrentOutput(1.0f),
      fLastMouseX(0.0f),
      fLastMouseY(0.0f),
      fFrameSkipCount(0)
#if !defined(__APPLE__)
      , fGodotDisplay(nullptr)
      , fGodotDrawable(0)
      , fGodotContext(nullptr)
#endif
{
    fParentWindowId = getWindow().getNativeWindowHandle();

    // Initialize the frame bridge with our window size
    enlil::FrameBridge::instance().setRequestedSize(
        DISTRHO_UI_DEFAULT_WIDTH,
        DISTRHO_UI_DEFAULT_HEIGHT
    );

    if (loadLibGodot()) {
        initGodot();
    }
}

FatSatUI::~FatSatUI()
{
    shutdownGodot();
    unloadLibGodot();
    cleanupOpenGL();
}

bool FatSatUI::loadLibGodot()
{
    // Try to find libgodot.so in various locations
    const char* paths[] = {
        // Relative to plugin location
        "./libgodot.linuxbsd.template_release.x86_64.so",
        "../lib/libgodot.linuxbsd.template_release.x86_64.so",
        // From build output
        "godot/bin/libgodot.linuxbsd.template_release.x86_64.so",
        // Symlink name
        "./libgodot.so",
        nullptr
    };

    for (int i = 0; paths[i] != nullptr; ++i) {
        fLibGodotHandle = dlopen(paths[i], RTLD_NOW | RTLD_LOCAL);
        if (fLibGodotHandle) {
            fprintf(stdout, "[FatSat] Loaded LibGodot from: %s\n", paths[i]);
            break;
        }
    }

    if (!fLibGodotHandle) {
        // Try with just the library name (system search)
        fLibGodotHandle = dlopen("libgodot.linuxbsd.template_release.x86_64.so", RTLD_NOW | RTLD_LOCAL);
    }

    if (!fLibGodotHandle) {
        fprintf(stderr, "[FatSat] Failed to load LibGodot: %s\n", dlerror());
        return false;
    }

    // Get function pointers
    *(void**)(&fCreateInstance) = dlsym(fLibGodotHandle, "libgodot_create_godot_instance");
    *(void**)(&fDestroyInstance) = dlsym(fLibGodotHandle, "libgodot_destroy_godot_instance");

    if (!fCreateInstance || !fDestroyInstance) {
        fprintf(stderr, "[FatSat] Failed to get LibGodot symbols: create=%p destroy=%p\n",
                (void*)fCreateInstance, (void*)fDestroyInstance);
        dlclose(fLibGodotHandle);
        fLibGodotHandle = nullptr;
        return false;
    }

    fprintf(stdout, "[FatSat] LibGodot symbols loaded successfully\n");
    return true;
}

void FatSatUI::unloadLibGodot()
{
    if (fLibGodotHandle) {
        dlclose(fLibGodotHandle);
        fLibGodotHandle = nullptr;
    }
    fCreateInstance = nullptr;
    fDestroyInstance = nullptr;
}

bool FatSatUI::initGodot()
{
    if (!fCreateInstance) {
        return false;
    }

    // Build command line arguments for Godot
    // Godot creates its own window for rendering. We extract frames via FrameBridge.
    const char* args[] = {
        "fatsat",
        "--path", "src/godot",
        "--rendering-method", "gl_compatibility",
        "--rendering-driver", "opengl3",
        nullptr
    };

    int argc = 0;
    while (args[argc] != nullptr) {
        ++argc;
    }

    fprintf(stdout, "[FatSat] Creating Godot instance with offscreen window, %d args\n", argc);

    // Create the Godot instance using 3-arg signature
    GDExtensionObjectPtr instancePtr = fCreateInstance(argc, (char**)args, fatsat_gdextension_init);

    if (!instancePtr) {
        fprintf(stderr, "[FatSat] Failed to create Godot instance\n");
        return false;
    }

    // Get the godot-cpp wrapper for the instance
    fGodotInstance = reinterpret_cast<godot::GodotInstance*>(
        godot::internal::get_object_instance_binding(instancePtr)
    );

    if (!fGodotInstance) {
        fprintf(stderr, "[FatSat] Failed to get GodotInstance binding\n");
        return false;
    }

    fprintf(stdout, "[FatSat] Godot instance created, starting engine...\n");

    // Start the engine using godot-cpp method
    fGodotInstance->start();
    fGodotStarted = true;

#if !defined(__APPLE__)
    // Capture Godot's GLX context info right after start() while it's still current.
    // We'll need this to restore the context before each iteration().
    fGodotDisplay = glXGetCurrentDisplay();
    fGodotDrawable = glXGetCurrentDrawable();
    fGodotContext = glXGetCurrentContext();
#endif

    return true;
}

void FatSatUI::shutdownGodot()
{
    if (fGodotInstance && fDestroyInstance) {
        // Get the raw object pointer for destruction
        GDExtensionObjectPtr obj = godot::internal::gdextension_interface_object_get_instance_from_id(
            fGodotInstance->get_instance_id()
        );
        fDestroyInstance(obj);
        fGodotInstance = nullptr;
        fGodotStarted = false;
        fprintf(stdout, "[FatSat] Godot instance destroyed\n");
    }
}

void FatSatUI::initOpenGL()
{
    if (fOpenGLInitialized) {
        return;
    }

    // Generate texture for frame display
    glGenTextures(1, &fFrameTexture);
    glBindTexture(GL_TEXTURE_2D, fFrameTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    fOpenGLInitialized = true;
}

void FatSatUI::cleanupOpenGL()
{
    if (fFrameTexture != 0) {
        glDeleteTextures(1, &fFrameTexture);
        fFrameTexture = 0;
    }
    fOpenGLInitialized = false;
}

void FatSatUI::uploadFrameTexture()
{
    auto& bridge = enlil::FrameBridge::instance();

    // Check if we have a new frame
    if (!bridge.hasNewFrame()) {
        return;
    }

    const uint8_t* data = bridge.getFrameData();
    int width = bridge.getFrameWidth();
    int height = bridge.getFrameHeight();

    if (!data || width <= 0 || height <= 0) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, fFrameTexture);

    // Upload texture data
    if (width != fTextureWidth || height != fTextureHeight) {
        // Texture size changed, reallocate
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data);
        fTextureWidth = width;
        fTextureHeight = height;
    } else {
        // Same size, just update
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                        GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

void FatSatUI::drawFullscreenQuad()
{
    if (fTextureWidth <= 0 || fTextureHeight <= 0) {
        return;
    }

    // Save OpenGL state
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT);

    // Setup for 2D rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, getWidth(), getHeight(), 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Bind texture and draw quad
    glBindTexture(GL_TEXTURE_2D, fFrameTexture);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    // Top-left
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    // Top-right
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f((float)getWidth(), 0.0f);
    // Bottom-right
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f((float)getWidth(), (float)getHeight());
    // Bottom-left
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, (float)getHeight());
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);

    // Restore matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Restore OpenGL state
    glPopAttrib();

    // Check for GL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        fprintf(stderr, "[FatSat] GL error in drawFullscreenQuad: 0x%x\n", err);
    }
}

void FatSatUI::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case 0: // kParamFatness
        fCurrentFatness = value;
        // TODO: Send to Godot UI via DSP bridge
        break;
    case 1: // kParamOutput
        fCurrentOutput = value;
        // TODO: Send to Godot UI via DSP bridge
        break;
    }

    repaint();
}

void FatSatUI::stateChanged(const char* key, const char* value)
{
    (void)key;
    (void)value;
}

void FatSatUI::uiIdle()
{
    // Run Godot frame iteration here (NOT in onDisplay)
    // This separates Godot's context management from DPF's OpenGL context
    if (fGodotStarted && fGodotInstance) {
#if !defined(__APPLE__)
        // Switch to Godot's OpenGL context before iteration
        if (fGodotDisplay && fGodotContext) {
            glXMakeCurrent(fGodotDisplay, fGodotDrawable, fGodotContext);
        }
#endif

        // Run Godot's frame with its context now active
        fGodotInstance->iteration();

#if !defined(__APPLE__)
        // Ensure Godot's rendering is complete
        glFinish();

        // Unbind context - DPF will bind its own in onDisplay()
        if (fGodotDisplay) {
            glXMakeCurrent(fGodotDisplay, None, nullptr);
        }
#endif

        // Skip first few frames to let Godot fully initialize
        if (fFrameSkipCount < 5) {
            fFrameSkipCount++;
        }

        // Note: DPF will re-bind its context when onDisplay() is called
    }

    // Request repaint to display captured frame
    repaint();
}

void FatSatUI::onDisplay()
{
    // DPF's OpenGL context is active here - only do DPF OpenGL operations
    // Godot iteration happens in uiIdle() to avoid context conflicts

    // Initialize our OpenGL resources (in DPF's context)
    if (!fOpenGLInitialized) {
        initOpenGL();
    }

    // Clear to dark gray background
    glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Upload frame from Godot if available (CPU data, no context conflict)
    uploadFrameTexture();

    // Draw the frame
    drawFullscreenQuad();
}

void FatSatUI::uiReshape(uint width, uint height)
{
    // Notify Godot of resize via FrameBridge
    enlil::FrameBridge::instance().setRequestedSize(width, height);

    // Setup viewport
    glViewport(0, 0, width, height);
}

bool FatSatUI::onMouse(const MouseEvent& ev)
{
    // Forward mouse button events to Godot
    enlil::FrameBridge::instance().pushMouseButton(
        ev.pos.getX(),
        ev.pos.getY(),
        ev.button,
        ev.press
    );

    fLastMouseX = ev.pos.getX();
    fLastMouseY = ev.pos.getY();

    return true;
}

bool FatSatUI::onMotion(const MotionEvent& ev)
{
    // Forward mouse motion events to Godot
    enlil::FrameBridge::instance().pushMouseMotion(
        ev.pos.getX(),
        ev.pos.getY()
    );

    fLastMouseX = ev.pos.getX();
    fLastMouseY = ev.pos.getY();

    return true;
}

bool FatSatUI::onScroll(const ScrollEvent& ev)
{
    // Forward scroll events to Godot
    enlil::FrameBridge::instance().pushScroll(
        ev.pos.getX(),
        ev.pos.getY(),
        ev.delta.getX(),
        ev.delta.getY()
    );

    return true;
}

bool FatSatUI::onKeyboard(const KeyboardEvent& ev)
{
    // Forward keyboard events to Godot
    enlil::FrameBridge::instance().pushKey(
        ev.key,
        ev.press
    );

    return true;
}

UI* createUI()
{
    return new FatSatUI();
}

END_NAMESPACE_DISTRHO
