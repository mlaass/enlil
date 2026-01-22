/*
 * Frame Bridge - Headless rendering data transfer between Godot and DPF
 * Part of the Enlil/GodotVST Framework
 *
 * Provides:
 * - Double-buffered frame data (Godot → DPF)
 * - Lock-free input event queue (DPF → Godot)
 * - Resize request handling (DPF → Godot)
 */

#ifndef FRAME_BRIDGE_HPP
#define FRAME_BRIDGE_HPP

#include <atomic>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <vector>

namespace enlil {

// Input event types for DPF → Godot communication
struct InputEvent {
    enum Type {
        MOUSE_MOTION,
        MOUSE_BUTTON,
        KEY,
        SCROLL
    };

    Type type;
    float x;           // Mouse X position
    float y;           // Mouse Y position
    int button;        // Mouse button index or key scancode
    bool pressed;      // Button/key pressed state
    float scrollX;     // Horizontal scroll delta
    float scrollY;     // Vertical scroll delta
};

// Lock-free SPSC ring buffer for input events
template<typename T, size_t Capacity>
class InputRingBuffer {
public:
    InputRingBuffer() : fReadPos(0), fWritePos(0) {}

    bool push(const T& item) {
        const size_t currentWrite = fWritePos.load(std::memory_order_relaxed);
        const size_t nextWrite = (currentWrite + 1) % Capacity;

        if (nextWrite == fReadPos.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }

        fBuffer[currentWrite] = item;
        fWritePos.store(nextWrite, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        const size_t currentRead = fReadPos.load(std::memory_order_relaxed);

        if (currentRead == fWritePos.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }

        item = fBuffer[currentRead];
        fReadPos.store((currentRead + 1) % Capacity, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return fReadPos.load(std::memory_order_acquire) ==
               fWritePos.load(std::memory_order_acquire);
    }

    void clear() {
        fReadPos.store(0, std::memory_order_relaxed);
        fWritePos.store(0, std::memory_order_relaxed);
    }

private:
    T fBuffer[Capacity];
    std::atomic<size_t> fReadPos;
    std::atomic<size_t> fWritePos;
};

using InputEventQueue = InputRingBuffer<InputEvent, 256>;

class FrameBridge {
public:
    // Get the singleton instance
    static FrameBridge& instance() {
        static FrameBridge inst;
        return inst;
    }

    // === Frame Export (Godot → DPF) ===

    // Called by Godot to submit a rendered frame
    // Copies RGBA data to the back buffer
    void submitFrame(const uint8_t* rgba, int width, int height) {
        if (!rgba || width <= 0 || height <= 0) {
            return;
        }

        size_t dataSize = static_cast<size_t>(width) * height * 4;

        std::lock_guard<std::mutex> lock(fSwapMutex);

        // Resize back buffer if needed
        if (fBackBuffer.size() != dataSize) {
            fBackBuffer.resize(dataSize);
        }

        // Copy frame data
        std::memcpy(fBackBuffer.data(), rgba, dataSize);
        fBackWidth = width;
        fBackHeight = height;
        fNewFrame.store(true, std::memory_order_release);
    }

    // Called by DPF to get the current frame data
    // Returns nullptr if no frame available
    const uint8_t* getFrameData() const {
        if (fFrontBuffer.empty()) {
            return nullptr;
        }
        return fFrontBuffer.data();
    }

    // Get frame dimensions
    int getFrameWidth() const {
        return fFrontWidth;
    }

    int getFrameHeight() const {
        return fFrontHeight;
    }

    // Check if a new frame is available and swap buffers
    // Returns true if a new frame was swapped in
    bool hasNewFrame() {
        if (!fNewFrame.load(std::memory_order_acquire)) {
            return false;
        }

        std::lock_guard<std::mutex> lock(fSwapMutex);

        // Swap front and back buffers
        std::swap(fFrontBuffer, fBackBuffer);
        fFrontWidth = fBackWidth;
        fFrontHeight = fBackHeight;

        fNewFrame.store(false, std::memory_order_release);
        return true;
    }

    // === Input Injection (DPF → Godot) ===

    // Push an input event from DPF
    void pushInputEvent(const InputEvent& event) {
        fInputQueue.push(event);
    }

    // Pop an input event for Godot
    // Returns true if an event was available
    bool popInputEvent(InputEvent& event) {
        return fInputQueue.pop(event);
    }

    // Convenience methods for common input events
    void pushMouseMotion(float x, float y) {
        InputEvent event;
        event.type = InputEvent::MOUSE_MOTION;
        event.x = x;
        event.y = y;
        event.button = 0;
        event.pressed = false;
        event.scrollX = 0;
        event.scrollY = 0;
        pushInputEvent(event);
    }

    void pushMouseButton(float x, float y, int button, bool pressed) {
        InputEvent event;
        event.type = InputEvent::MOUSE_BUTTON;
        event.x = x;
        event.y = y;
        event.button = button;
        event.pressed = pressed;
        event.scrollX = 0;
        event.scrollY = 0;
        pushInputEvent(event);
    }

    void pushScroll(float x, float y, float scrollX, float scrollY) {
        InputEvent event;
        event.type = InputEvent::SCROLL;
        event.x = x;
        event.y = y;
        event.button = 0;
        event.pressed = false;
        event.scrollX = scrollX;
        event.scrollY = scrollY;
        pushInputEvent(event);
    }

    void pushKey(int keycode, bool pressed) {
        InputEvent event;
        event.type = InputEvent::KEY;
        event.x = 0;
        event.y = 0;
        event.button = keycode;
        event.pressed = pressed;
        event.scrollX = 0;
        event.scrollY = 0;
        pushInputEvent(event);
    }

    // === Resize Handling (DPF → Godot) ===

    // Set the requested viewport size from DPF
    void setRequestedSize(int width, int height) {
        fRequestedWidth.store(width, std::memory_order_release);
        fRequestedHeight.store(height, std::memory_order_release);
        fSizeChanged.store(true, std::memory_order_release);
    }

    // Get requested size if changed
    // Returns true if size changed, fills width/height
    bool getRequestedSize(int& width, int& height) {
        if (!fSizeChanged.load(std::memory_order_acquire)) {
            return false;
        }

        width = fRequestedWidth.load(std::memory_order_acquire);
        height = fRequestedHeight.load(std::memory_order_acquire);
        fSizeChanged.store(false, std::memory_order_release);
        return true;
    }

    // Get current requested size without clearing the changed flag
    void getCurrentSize(int& width, int& height) const {
        width = fRequestedWidth.load(std::memory_order_acquire);
        height = fRequestedHeight.load(std::memory_order_acquire);
    }

private:
    FrameBridge()
        : fFrontWidth(0)
        , fFrontHeight(0)
        , fBackWidth(0)
        , fBackHeight(0)
        , fNewFrame(false)
        , fRequestedWidth(600)
        , fRequestedHeight(400)
        , fSizeChanged(false)
    {}

    // Prevent copying
    FrameBridge(const FrameBridge&) = delete;
    FrameBridge& operator=(const FrameBridge&) = delete;

    // Double buffer for frame data
    std::vector<uint8_t> fFrontBuffer;
    std::vector<uint8_t> fBackBuffer;
    int fFrontWidth, fFrontHeight;
    int fBackWidth, fBackHeight;
    std::atomic<bool> fNewFrame;
    std::mutex fSwapMutex;

    // Input event queue
    InputEventQueue fInputQueue;

    // Resize request
    std::atomic<int> fRequestedWidth;
    std::atomic<int> fRequestedHeight;
    std::atomic<bool> fSizeChanged;
};

} // namespace enlil

#endif // FRAME_BRIDGE_HPP
