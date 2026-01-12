/*
 * FatSat UI - Godot-based user interface
 * Part of the Enlil/GodotVST Framework
 */

#include "FatSatUI.hpp"
#include "DistrhoPluginInfo.h"

// TODO: Include LibGodot headers when available
// #include <godot_cpp/godot.hpp>

START_NAMESPACE_DISTRHO

FatSatUI::FatSatUI()
    : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT),
      fParentWindowId(0),
      fGodotContext(nullptr),
      fGodotInitialized(false),
      fCurrentFatness(0.0f),
      fCurrentOutput(1.0f)
{
    // Get native window handle from DPF
    fParentWindowId = getParentWindowHandle();

    initGodot();
}

FatSatUI::~FatSatUI()
{
    shutdownGodot();
}

void FatSatUI::initGodot()
{
    if (fGodotInitialized) {
        return;
    }

    // TODO: Initialize LibGodot with embedded window
    // 1. Load embedded PCK data
    // 2. Initialize Godot with parent window handle
    // 3. Load main scene

    // Placeholder - actual implementation requires LibGodot integration
    // GodotContext* ctx = libgodot_create();
    // libgodot_set_native_handle(ctx, (void*)fParentWindowId);
    // libgodot_initialize(ctx, embedded_pck_data, embedded_pck_size);
    // fGodotContext = ctx;

    fGodotInitialized = true;
}

void FatSatUI::shutdownGodot()
{
    if (!fGodotInitialized) {
        return;
    }

    // TODO: Cleanup LibGodot
    // if (fGodotContext) {
    //     libgodot_shutdown(fGodotContext);
    //     libgodot_destroy(fGodotContext);
    //     fGodotContext = nullptr;
    // }

    fGodotInitialized = false;
}

void FatSatUI::godotIteration()
{
    if (!fGodotInitialized || !fGodotContext) {
        return;
    }

    // TODO: Run one Godot frame iteration
    // libgodot_iteration(fGodotContext);
}

void FatSatUI::parameterChanged(uint32_t index, float value)
{
    switch (index) {
    case 0: // kParamFatness
        fCurrentFatness = value;
        // TODO: Update Godot UI via bridge
        // bridge_set_param(fGodotContext, "fatness", value);
        break;
    case 1: // kParamOutput
        fCurrentOutput = value;
        // TODO: Update Godot UI via bridge
        // bridge_set_param(fGodotContext, "output", value);
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
    // Run Godot frame loop iteration
    godotIteration();

    // Request repaint if Godot rendered
    if (fGodotInitialized) {
        repaint();
    }
}

void FatSatUI::onDisplay()
{
    // TODO: For now, just clear to a dark color
    // Once LibGodot is integrated, Godot renders directly to the window

    // Placeholder rendering - dark background
    // This will be replaced by Godot rendering
}

void FatSatUI::uiReshape(uint width, uint height)
{
    (void)width;
    (void)height;

    // TODO: Notify Godot of resize
    // if (fGodotContext) {
    //     libgodot_resize(fGodotContext, width, height);
    // }
}

UI* createUI()
{
    return new FatSatUI();
}

END_NAMESPACE_DISTRHO
