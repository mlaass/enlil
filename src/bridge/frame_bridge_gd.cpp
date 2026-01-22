/*
 * Frame Bridge GDExtension Wrapper Implementation
 * Part of the Enlil/GodotVST Framework
 */

#include "frame_bridge_gd.hpp"
#include "../shared/frame_bridge.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

FrameBridgeGD* FrameBridgeGD::singleton = nullptr;

FrameBridgeGD::FrameBridgeGD() {
    singleton = this;
}

FrameBridgeGD::~FrameBridgeGD() {
    if (singleton == this) {
        singleton = nullptr;
    }
}

FrameBridgeGD* FrameBridgeGD::get_singleton() {
    return singleton;
}

void FrameBridgeGD::submit_frame(const Ref<Image>& image) {
    if (image.is_null() || image->is_empty()) {
        return;
    }

    int width = image->get_width();
    int height = image->get_height();

    if (width <= 0 || height <= 0) {
        return;
    }

    // Ensure image is in RGBA8 format
    Ref<Image> rgba_image = image;
    if (image->get_format() != Image::FORMAT_RGBA8) {
        rgba_image = image->duplicate();
        if (rgba_image.is_null()) {
            return;
        }
        rgba_image->convert(Image::FORMAT_RGBA8);
    }

    // Get raw pixel data
    PackedByteArray data = rgba_image->get_data();
    if (data.size() == 0) {
        return;
    }

    // Submit to the C++ bridge
    enlil::FrameBridge::instance().submitFrame(
        data.ptr(),
        width,
        height
    );
}

Dictionary FrameBridgeGD::pop_input_event() {
    enlil::InputEvent event;

    if (!enlil::FrameBridge::instance().popInputEvent(event)) {
        return Dictionary(); // Empty dictionary means no event
    }

    Dictionary result;

    switch (event.type) {
        case enlil::InputEvent::MOUSE_MOTION:
            result["type"] = "mouse_motion";
            result["x"] = event.x;
            result["y"] = event.y;
            break;

        case enlil::InputEvent::MOUSE_BUTTON:
            result["type"] = "mouse_button";
            result["x"] = event.x;
            result["y"] = event.y;
            result["button"] = event.button;
            result["pressed"] = event.pressed;
            break;

        case enlil::InputEvent::SCROLL:
            result["type"] = "scroll";
            result["x"] = event.x;
            result["y"] = event.y;
            result["scroll_x"] = event.scrollX;
            result["scroll_y"] = event.scrollY;
            break;

        case enlil::InputEvent::KEY:
            result["type"] = "key";
            result["keycode"] = event.button;
            result["pressed"] = event.pressed;
            break;
    }

    return result;
}

Vector2i FrameBridgeGD::get_requested_size() {
    int width, height;

    if (!enlil::FrameBridge::instance().getRequestedSize(width, height)) {
        return Vector2i(0, 0); // No change
    }

    return Vector2i(width, height);
}

void FrameBridgeGD::_bind_methods() {
    // Frame submission
    ClassDB::bind_method(D_METHOD("submit_frame", "image"), &FrameBridgeGD::submit_frame);

    // Input event polling
    ClassDB::bind_method(D_METHOD("pop_input_event"), &FrameBridgeGD::pop_input_event);

    // Resize handling
    ClassDB::bind_method(D_METHOD("get_requested_size"), &FrameBridgeGD::get_requested_size);
}

} // namespace godot
