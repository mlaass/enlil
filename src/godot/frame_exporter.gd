extends Node
## Frame Exporter - Copies SubViewport frames to the FrameBridge
## Part of the Enlil/GodotVST Framework

var bridge: FrameBridgeGD
var sub_viewport: SubViewport
var frame_count: int = 0

func _ready() -> void:
	# Create bridge instance
	bridge = FrameBridgeGD.new()

	# Get reference to SubViewport
	sub_viewport = get_node("../SubViewport")
	if not sub_viewport:
		push_error("[FrameExporter] SubViewport not found!")
		return

func _process(_delta: float) -> void:
	if not bridge or not sub_viewport:
		return

	# Skip first few frames to let Godot fully initialize
	frame_count += 1
	if frame_count < 5:
		return

	# Check for resize requests from the host
	var new_size := bridge.get_requested_size()
	if new_size != Vector2i.ZERO:
		sub_viewport.size = new_size

	# Export frame to bridge (with safety checks)
	var texture := sub_viewport.get_texture()
	if texture == null:
		return

	var image := texture.get_image()
	if image == null or image.is_empty():
		return

	bridge.submit_frame(image)
