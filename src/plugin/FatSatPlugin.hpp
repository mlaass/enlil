/*
 * FatSat - One-knob saturation and dynamics plugin
 * Part of the Enlil/GodotVST Framework
 */

#ifndef FATSAT_PLUGIN_HPP
#define FATSAT_PLUGIN_HPP

#include "DistrhoPlugin.hpp"

START_NAMESPACE_DISTRHO

enum Parameters {
    kParamFatness = 0,
    kParamOutput,
    kParamCount
};

class FatSatPlugin : public Plugin {
public:
    FatSatPlugin();

protected:
    const char* getLabel() const override { return "FatSat"; }
    const char* getDescription() const override {
        return "One-knob saturation and dynamics";
    }
    const char* getMaker() const override { return "Enlil"; }
    const char* getHomePage() const override {
        return "https://github.com/mlaass/enlil";
    }
    const char* getLicense() const override { return "MIT"; }
    uint32_t getVersion() const override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const override { return d_cconst('e', 'F', 'a', 't'); }

    void initParameter(uint32_t index, Parameter& parameter) override;
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    void initState(uint32_t index, State& state) override;
    void setState(const char* key, const char* value) override;

    void run(const float** inputs, float** outputs, uint32_t frames) override;

private:
    float fFatness;
    float fOutput;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FatSatPlugin)
};

END_NAMESPACE_DISTRHO

#endif // FATSAT_PLUGIN_HPP
