/*
 * DSP-UI Bridge - Shared state for thread-safe communication
 * Part of the Enlil/GodotVST Framework
 *
 * This provides a pure C++ interface that both the DPF plugin
 * and the GDExtension bridge can use for DSP-UI communication.
 */

#ifndef DSP_BRIDGE_HPP
#define DSP_BRIDGE_HPP

#include <atomic>
#include <cstddef>

// Include the ring buffer (header-only, no dependencies)
#include "../bridge/ring_buffer.hpp"

namespace enlil {

// Shared state for DSP-UI communication
// This is a singleton that both DPF and Godot can access
class DSPBridge {
public:
    // Get the singleton instance
    static DSPBridge& instance() {
        static DSPBridge inst;
        return inst;
    }

    // === DSP Thread Interface (write) ===

    // Push visualization data from DSP thread
    void pushVisualization(float rmsL, float rmsR, float peakL, float peakR) {
        VisualizationData data = {rmsL, rmsR, peakL, peakR};
        fVisualizationBuffer.push(data);
    }

    // Read parameter values (DSP thread reads what UI set)
    float getFatness() const {
        return fFatness.load(std::memory_order_acquire);
    }

    float getOutput() const {
        return fOutput.load(std::memory_order_acquire);
    }

    // === UI Thread Interface (read/write) ===

    // Poll visualization data (UI thread only)
    // Returns true if new data was available
    bool pollVisualization() {
        VisualizationData data;
        bool gotData = false;

        // Drain all available data, keeping the latest
        while (fVisualizationBuffer.pop(data)) {
            fLastRmsLeft = data.rmsLeft;
            fLastRmsRight = data.rmsRight;
            fLastPeakLeft = data.peakLeft;
            fLastPeakRight = data.peakRight;
            gotData = true;
        }

        return gotData;
    }

    // Get latest visualization values (UI thread only)
    float getRmsLeft() const { return fLastRmsLeft; }
    float getRmsRight() const { return fLastRmsRight; }
    float getPeakLeft() const { return fLastPeakLeft; }
    float getPeakRight() const { return fLastPeakRight; }

    // Set parameters from UI
    void setFatness(float value) {
        fFatness.store(value, std::memory_order_release);
    }

    void setOutput(float value) {
        fOutput.store(value, std::memory_order_release);
    }

private:
    DSPBridge()
        : fFatness(0.0f)
        , fOutput(1.0f)
        , fLastRmsLeft(0.0f)
        , fLastRmsRight(0.0f)
        , fLastPeakLeft(0.0f)
        , fLastPeakRight(0.0f)
    {}

    // Prevent copying
    DSPBridge(const DSPBridge&) = delete;
    DSPBridge& operator=(const DSPBridge&) = delete;

    // Atomic parameters (DSP reads, UI writes)
    std::atomic<float> fFatness;
    std::atomic<float> fOutput;

    // Lock-free ring buffer for visualization (DSP writes, UI reads)
    VisualizationRingBuffer fVisualizationBuffer;

    // Latest visualization values (UI thread only)
    float fLastRmsLeft;
    float fLastRmsRight;
    float fLastPeakLeft;
    float fLastPeakRight;
};

} // namespace enlil

// === C API for external access (if needed) ===

extern "C" {

inline void enlil_push_visualization(float rmsL, float rmsR, float peakL, float peakR) {
    enlil::DSPBridge::instance().pushVisualization(rmsL, rmsR, peakL, peakR);
}

inline float enlil_get_fatness() {
    return enlil::DSPBridge::instance().getFatness();
}

inline float enlil_get_output() {
    return enlil::DSPBridge::instance().getOutput();
}

inline void enlil_set_fatness(float value) {
    enlil::DSPBridge::instance().setFatness(value);
}

inline void enlil_set_output(float value) {
    enlil::DSPBridge::instance().setOutput(value);
}

inline bool enlil_poll_visualization() {
    return enlil::DSPBridge::instance().pollVisualization();
}

inline float enlil_get_rms_left() {
    return enlil::DSPBridge::instance().getRmsLeft();
}

inline float enlil_get_rms_right() {
    return enlil::DSPBridge::instance().getRmsRight();
}

inline float enlil_get_peak_left() {
    return enlil::DSPBridge::instance().getPeakLeft();
}

inline float enlil_get_peak_right() {
    return enlil::DSPBridge::instance().getPeakRight();
}

}

#endif // DSP_BRIDGE_HPP
