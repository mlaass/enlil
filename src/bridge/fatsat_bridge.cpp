/*
 * FatSat Bridge - Thread-safe communication between DSP and Godot UI
 * Part of the Enlil/GodotVST Framework
 */

#include "fatsat_bridge.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

FatSatBridge* FatSatBridge::singleton = nullptr;

FatSatBridge::FatSatBridge()
    : fFatness(0.0f),
      fOutput(1.0f),
      fRmsLeft(0.0f),
      fRmsRight(0.0f),
      fPeakLeft(0.0f),
      fPeakRight(0.0f)
{
    singleton = this;
}

FatSatBridge::~FatSatBridge()
{
    if (singleton == this) {
        singleton = nullptr;
    }
}

FatSatBridge* FatSatBridge::get_singleton()
{
    return singleton;
}

float FatSatBridge::get_fatness() const
{
    return fFatness.load(std::memory_order_acquire);
}

void FatSatBridge::set_fatness(float value)
{
    fFatness.store(value, std::memory_order_release);
}

float FatSatBridge::get_output() const
{
    return fOutput.load(std::memory_order_acquire);
}

void FatSatBridge::set_output(float value)
{
    fOutput.store(value, std::memory_order_release);
}

float FatSatBridge::get_rms_left() const
{
    return fRmsLeft;
}

float FatSatBridge::get_rms_right() const
{
    return fRmsRight;
}

float FatSatBridge::get_peak_left() const
{
    return fPeakLeft;
}

float FatSatBridge::get_peak_right() const
{
    return fPeakRight;
}

void FatSatBridge::push_visualization(float rmsL, float rmsR, float peakL, float peakR)
{
    VisualizationData data;
    data.rmsLeft = rmsL;
    data.rmsRight = rmsR;
    data.peakLeft = peakL;
    data.peakRight = peakR;
    fVisualizationBuffer.push(data);
}

void FatSatBridge::poll_visualization()
{
    VisualizationData data;
    while (fVisualizationBuffer.pop(data)) {
        fRmsLeft = data.rmsLeft;
        fRmsRight = data.rmsRight;
        fPeakLeft = data.peakLeft;
        fPeakRight = data.peakRight;
    }
}

void FatSatBridge::_bind_methods()
{
    // Parameter properties
    ClassDB::bind_method(D_METHOD("get_fatness"), &FatSatBridge::get_fatness);
    ClassDB::bind_method(D_METHOD("set_fatness", "value"), &FatSatBridge::set_fatness);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fatness"), "set_fatness", "get_fatness");

    ClassDB::bind_method(D_METHOD("get_output"), &FatSatBridge::get_output);
    ClassDB::bind_method(D_METHOD("set_output", "value"), &FatSatBridge::set_output);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "output"), "set_output", "get_output");

    // Visualization getters
    ClassDB::bind_method(D_METHOD("get_rms_left"), &FatSatBridge::get_rms_left);
    ClassDB::bind_method(D_METHOD("get_rms_right"), &FatSatBridge::get_rms_right);
    ClassDB::bind_method(D_METHOD("get_peak_left"), &FatSatBridge::get_peak_left);
    ClassDB::bind_method(D_METHOD("get_peak_right"), &FatSatBridge::get_peak_right);

    // Polling method
    ClassDB::bind_method(D_METHOD("poll_visualization"), &FatSatBridge::poll_visualization);
}

} // namespace godot
