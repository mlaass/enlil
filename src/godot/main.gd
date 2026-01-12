extends Control

# Reference to UI elements
@onready var fatness_label: Label = $VBox/FatnessLabel
@onready var rms_bar: ProgressBar = $VBox/RMSBar

# Bridge singleton (will be available when GDExtension is loaded)
var bridge: Object = null

# Animation state
var idle_time: float = 0.0

func _ready() -> void:
	# Try to get bridge singleton
	if Engine.has_singleton("FatSatBridge"):
		bridge = Engine.get_singleton("FatSatBridge")
		print("FatSat: Bridge connected")
	else:
		print("FatSat: Running without bridge (standalone mode)")

func _process(delta: float) -> void:
	idle_time += delta

	# Idle animation - gentle breathing effect
	var breath = sin(idle_time * 2.0) * 0.02 + 1.0
	scale = Vector2(breath, breath)

	if bridge:
		# Poll visualization data from DSP
		bridge.poll_visualization()

		# Update UI from bridge
		var fatness = bridge.get_fatness()
		fatness_label.text = "FATNESS: %d%%" % int(fatness * 100)

		# RMS meter (average of left and right)
		var rms = (bridge.get_rms_left() + bridge.get_rms_right()) / 2.0
		rms_bar.value = rms
