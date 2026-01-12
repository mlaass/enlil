/*
 * FatSat UI - Godot-based user interface
 * Part of the Enlil/GodotVST Framework
 */

#ifndef FATSAT_UI_HPP
#define FATSAT_UI_HPP

#include "DistrhoUI.hpp"

// Forward declarations for Godot
typedef struct GodotContext GodotContext;

START_NAMESPACE_DISTRHO

class FatSatUI : public UI {
public:
    FatSatUI();
    ~FatSatUI() override;

protected:
    void parameterChanged(uint32_t index, float value) override;
    void stateChanged(const char* key, const char* value) override;
    void uiIdle() override;
    void onDisplay() override;
    void uiReshape(uint width, uint height) override;

private:
    void initGodot();
    void shutdownGodot();
    void godotIteration();

    uintptr_t fParentWindowId;
    GodotContext* fGodotContext;
    bool fGodotInitialized;

    float fCurrentFatness;
    float fCurrentOutput;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FatSatUI)
};

END_NAMESPACE_DISTRHO

#endif // FATSAT_UI_HPP
