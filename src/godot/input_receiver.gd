extends Node
## Input Receiver - Polls input events from FrameBridge and injects into Godot
## Part of the Enlil/GodotVST Framework

var bridge: FrameBridgeGD
var sub_viewport: SubViewport

# Track mouse state for motion events
var last_mouse_position := Vector2.ZERO
var mouse_button_mask := 0

func _ready() -> void:
	# Create bridge instance
	bridge = FrameBridgeGD.new()

	# Get reference to SubViewport for input injection
	sub_viewport = get_node("../SubViewport")
	if not sub_viewport:
		push_error("[InputReceiver] SubViewport not found!")
		return

func _process(_delta: float) -> void:
	if not bridge or not sub_viewport:
		return

	# Process all pending input events
	while true:
		var event_data := bridge.pop_input_event()
		if event_data.is_empty():
			break
		_inject_event(event_data)

func _inject_event(data: Dictionary) -> void:
	var event_type: String = data.get("type", "")

	match event_type:
		"mouse_motion":
			_handle_mouse_motion(data)
		"mouse_button":
			_handle_mouse_button(data)
		"scroll":
			_handle_scroll(data)
		"key":
			_handle_key(data)
		_:
			push_warning("[InputReceiver] Unknown event type: ", event_type)

func _handle_mouse_motion(data: Dictionary) -> void:
	var event := InputEventMouseMotion.new()
	var pos := Vector2(data.get("x", 0.0), data.get("y", 0.0))

	event.position = pos
	event.global_position = pos
	event.relative = pos - last_mouse_position
	event.button_mask = mouse_button_mask

	last_mouse_position = pos

	sub_viewport.push_input(event)

func _handle_mouse_button(data: Dictionary) -> void:
	var event := InputEventMouseButton.new()
	var pos := Vector2(data.get("x", 0.0), data.get("y", 0.0))
	var button: int = data.get("button", 1)
	var pressed: bool = data.get("pressed", false)

	event.position = pos
	event.global_position = pos
	event.button_index = _dpf_button_to_godot(button)
	event.pressed = pressed

	# Update button mask
	var mask_bit := 1 << (event.button_index - 1)
	if pressed:
		mouse_button_mask |= mask_bit
	else:
		mouse_button_mask &= ~mask_bit
	event.button_mask = mouse_button_mask

	last_mouse_position = pos

	sub_viewport.push_input(event)

	# Also send a motion event to update position
	if pressed:
		var motion := InputEventMouseMotion.new()
		motion.position = pos
		motion.global_position = pos
		motion.button_mask = mouse_button_mask
		sub_viewport.push_input(motion)

func _handle_scroll(data: Dictionary) -> void:
	var pos := Vector2(data.get("x", 0.0), data.get("y", 0.0))
	var scroll_x: float = data.get("scroll_x", 0.0)
	var scroll_y: float = data.get("scroll_y", 0.0)

	# Scroll events are button events in Godot
	if scroll_y != 0.0:
		var event := InputEventMouseButton.new()
		event.position = pos
		event.global_position = pos
		event.button_index = MOUSE_BUTTON_WHEEL_UP if scroll_y > 0 else MOUSE_BUTTON_WHEEL_DOWN
		event.factor = absf(scroll_y)
		event.pressed = true
		sub_viewport.push_input(event)
		# Release
		event.pressed = false
		sub_viewport.push_input(event)

	if scroll_x != 0.0:
		var event := InputEventMouseButton.new()
		event.position = pos
		event.global_position = pos
		event.button_index = MOUSE_BUTTON_WHEEL_LEFT if scroll_x < 0 else MOUSE_BUTTON_WHEEL_RIGHT
		event.factor = absf(scroll_x)
		event.pressed = true
		sub_viewport.push_input(event)
		# Release
		event.pressed = false
		sub_viewport.push_input(event)

func _handle_key(data: Dictionary) -> void:
	var event := InputEventKey.new()
	var keycode: int = data.get("keycode", 0)
	var pressed: bool = data.get("pressed", false)

	event.keycode = keycode
	event.physical_keycode = keycode
	event.pressed = pressed

	sub_viewport.push_input(event)

func _dpf_button_to_godot(dpf_button: int) -> MouseButton:
	# DPF uses 1=left, 2=middle, 3=right
	# Godot uses MOUSE_BUTTON_LEFT=1, MOUSE_BUTTON_RIGHT=2, MOUSE_BUTTON_MIDDLE=3
	match dpf_button:
		1:
			return MOUSE_BUTTON_LEFT
		2:
			return MOUSE_BUTTON_MIDDLE
		3:
			return MOUSE_BUTTON_RIGHT
		_:
			return MOUSE_BUTTON_LEFT
