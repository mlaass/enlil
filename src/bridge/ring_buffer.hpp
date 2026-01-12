/*
 * Lock-free Single-Producer Single-Consumer Ring Buffer
 * Used for audio thread -> UI thread visualization data
 * Part of the Enlil/GodotVST Framework
 */

#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <atomic>
#include <cstddef>
#include <cstring>

template<typename T, size_t Capacity>
class RingBuffer {
public:
    RingBuffer() : fReadPos(0), fWritePos(0) {}

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

// Visualization data sent from audio thread to UI
struct VisualizationData {
    float rmsLeft;
    float rmsRight;
    float peakLeft;
    float peakRight;
};

// Default ring buffer for visualization (64 samples should be plenty at 60fps)
using VisualizationRingBuffer = RingBuffer<VisualizationData, 64>;

#endif // RING_BUFFER_HPP
