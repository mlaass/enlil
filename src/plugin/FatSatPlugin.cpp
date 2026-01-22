/*
 * FatSat - One-knob saturation and dynamics plugin
 * Part of the Enlil/GodotVST Framework
 */

#include "FatSatPlugin.hpp"
#include <cmath>
#include <algorithm>

// Shared DSP-UI bridge for visualization data
#include "../shared/dsp_bridge.hpp"

START_NAMESPACE_DISTRHO

FatSatPlugin::FatSatPlugin()
    : Plugin(kParamCount, 0, 1), // params, programs, states
      fFatness(0.0f),
      fOutput(1.0f)
{
}

void FatSatPlugin::initParameter(uint32_t index, Parameter& parameter)
{
    switch (index) {
    case kParamFatness:
        parameter.hints = kParameterIsAutomatable;
        parameter.name = "Fatness";
        parameter.symbol = "fatness";
        parameter.unit = "%";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;

    case kParamOutput:
        parameter.hints = kParameterIsAutomatable;
        parameter.name = "Output";
        parameter.symbol = "output";
        parameter.unit = "%";
        parameter.ranges.def = 1.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;
    }
}

float FatSatPlugin::getParameterValue(uint32_t index) const
{
    switch (index) {
    case kParamFatness:
        return fFatness;
    case kParamOutput:
        return fOutput;
    default:
        return 0.0f;
    }
}

void FatSatPlugin::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case kParamFatness:
        fFatness = value;
        break;
    case kParamOutput:
        fOutput = value;
        break;
    }
}

void FatSatPlugin::initState(uint32_t index, State& state)
{
    if (index == 0) {
        state.key = "bridge_state";
        state.defaultValue = "";
    }
}

void FatSatPlugin::setState(const char* key, const char* value)
{
    (void)key;
    (void)value;
}

void FatSatPlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    const float* inL = inputs[0];
    const float* inR = inputs[1];
    float* outL = outputs[0];
    float* outR = outputs[1];

    // Drive amount from fatness parameter (1.0 to 10.0)
    const float drive = 1.0f + fFatness * 9.0f;
    // Output gain with auto-compensation
    const float gain = fOutput * (1.0f / (1.0f + fFatness * 0.5f));
    // Ceiling limit at -0.1dB
    const float ceiling = 0.989f;

    for (uint32_t i = 0; i < frames; ++i) {
        // Tanh soft clipper: output = tanh(input * drive)
        float sampleL = std::tanh(inL[i] * drive) * gain;
        float sampleR = std::tanh(inR[i] * drive) * gain;

        // Hard ceiling limiter
        sampleL = std::fmax(-ceiling, std::fmin(ceiling, sampleL));
        sampleR = std::fmax(-ceiling, std::fmin(ceiling, sampleR));

        outL[i] = sampleL;
        outR[i] = sampleR;
    }

    // Calculate RMS and peak for visualization
    float sumL = 0.0f, sumR = 0.0f;
    float peakL = 0.0f, peakR = 0.0f;

    for (uint32_t i = 0; i < frames; ++i) {
        sumL += outL[i] * outL[i];
        sumR += outR[i] * outR[i];
        peakL = std::max(peakL, std::fabs(outL[i]));
        peakR = std::max(peakR, std::fabs(outR[i]));
    }

    float rmsL = std::sqrt(sumL / frames);
    float rmsR = std::sqrt(sumR / frames);

    // Push to shared bridge for UI
    enlil::DSPBridge::instance().pushVisualization(rmsL, rmsR, peakL, peakR);
}

Plugin* createPlugin()
{
    return new FatSatPlugin();
}

END_NAMESPACE_DISTRHO
