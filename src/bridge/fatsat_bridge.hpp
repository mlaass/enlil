/*
 * FatSat Bridge - Thread-safe communication between DSP and Godot UI
 * Part of the Enlil/GodotVST Framework
 */

#ifndef FATSAT_BRIDGE_HPP
#define FATSAT_BRIDGE_HPP

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <atomic>

#include "ring_buffer.hpp"

namespace godot {

class FatSatBridge : public Object {
    GDCLASS(FatSatBridge, Object)

public:
    FatSatBridge();
    ~FatSatBridge();

    // Parameter access (UI thread)
    float get_fatness() const;
    void set_fatness(float value);
    float get_output() const;
    void set_output(float value);

    // Visualization data (UI thread reads)
    float get_rms_left() const;
    float get_rms_right() const;
    float get_peak_left() const;
    float get_peak_right() const;

    // Called by DSP thread to push visualization data
    void push_visualization(float rmsL, float rmsR, float peakL, float peakR);

    // Called by UI thread to poll latest visualization data
    void poll_visualization();

    // Singleton access
    static FatSatBridge* get_singleton();

protected:
    static void _bind_methods();

private:
    static FatSatBridge* singleton;

    // Atomic parameters (DSP reads, UI writes)
    std::atomic<float> fFatness;
    std::atomic<float> fOutput;

    // Lock-free ring buffer for visualization data
    VisualizationRingBuffer fVisualizationBuffer;

    // Latest visualization values (UI thread only)
    float fRmsLeft;
    float fRmsRight;
    float fPeakLeft;
    float fPeakRight;
};

} // namespace godot

#endif // FATSAT_BRIDGE_HPP
