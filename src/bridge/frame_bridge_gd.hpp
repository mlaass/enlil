/*
 * Frame Bridge GDExtension Wrapper
 * Exposes FrameBridge functionality to GDScript
 * Part of the Enlil/GodotVST Framework
 */

#ifndef FRAME_BRIDGE_GD_HPP
#define FRAME_BRIDGE_GD_HPP

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector2i.hpp>

namespace godot {

class FrameBridgeGD : public Object {
    GDCLASS(FrameBridgeGD, Object)

public:
    FrameBridgeGD();
    ~FrameBridgeGD();

    // Submit a rendered frame to the bridge
    // Called by FrameExporter.gd each frame
    void submit_frame(const Ref<Image>& image);

    // Pop the next input event from the queue
    // Returns empty Dictionary if no events available
    // Dictionary keys: type (String), x, y (float), button (int), pressed (bool), keycode (int)
    Dictionary pop_input_event();

    // Get the requested viewport size if it has changed
    // Returns Vector2i(0, 0) if size hasn't changed
    Vector2i get_requested_size();

    // Singleton access
    static FrameBridgeGD* get_singleton();

protected:
    static void _bind_methods();

private:
    static FrameBridgeGD* singleton;
};

} // namespace godot

#endif // FRAME_BRIDGE_GD_HPP
