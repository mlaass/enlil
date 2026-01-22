extends Control
## Plugin UI - Main UI controller for FatSat
## Part of the Enlil/GodotVST Framework

# Node references
@onready var fatness_knob: Panel = $FatnessKnob
@onready var knob_indicator: ColorRect = $FatnessKnob/KnobIndicator
@onready var value_label: Label = $ValueLabel
@onready var meter_left: ColorRect = $MeterLeft
@onready var meter_right: ColorRect = $MeterRight

# DSP bridge for parameter sync
var dsp_bridge: FatSatBridge

# Knob state
var fatness_value: float = 0.0
var is_dragging: bool = false
var drag_start_y: float = 0.0
var drag_start_value: float = 0.0

# Knob configuration
const KNOB_SENSITIVITY: float = 0.005
const KNOB_MIN_ANGLE: float = -135.0
const KNOB_MAX_ANGLE: float = 135.0

# Meter configuration
const METER_MIN_HEIGHT: float = 10.0
const METER_MAX_HEIGHT: float = 240.0
const METER_DECAY: float = 0.9

# Meter state
var meter_left_value: float = 0.0
var meter_right_value: float = 0.0

func _ready() -> void:
	# Create DSP bridge instance
	dsp_bridge = FatSatBridge.new()

	# Set up knob for input
	fatness_knob.mouse_filter = Control.MOUSE_FILTER_STOP

	# Initial UI state
	_update_knob_visual()
	_update_value_label()

	print("[PluginUI] Initialized")

func _process(delta: float) -> void:
	if not dsp_bridge:
		return

	# Poll visualization data from DSP
	dsp_bridge.poll_visualization()

	# Update meters with decay
	var target_left := dsp_bridge.get_rms_left()
	var target_right := dsp_bridge.get_rms_right()

	# Smooth decay
	if target_left > meter_left_value:
		meter_left_value = target_left
	else:
		meter_left_value = lerpf(meter_left_value, target_left, 1.0 - pow(METER_DECAY, delta * 60.0))

	if target_right > meter_right_value:
		meter_right_value = target_right
	else:
		meter_right_value = lerpf(meter_right_value, target_right, 1.0 - pow(METER_DECAY, delta * 60.0))

	_update_meters()

func _gui_input(event: InputEvent) -> void:
	# Handle input on the main control
	pass

func _input(event: InputEvent) -> void:
	# Handle knob interaction
	if event is InputEventMouseButton:
		_handle_mouse_button(event)
	elif event is InputEventMouseMotion:
		_handle_mouse_motion(event)

func _handle_mouse_button(event: InputEventMouseButton) -> void:
	if event.button_index != MOUSE_BUTTON_LEFT:
		return

	var knob_rect := fatness_knob.get_global_rect()

	if event.pressed:
		# Check if click is on knob
		if knob_rect.has_point(event.position):
			is_dragging = true
			drag_start_y = event.position.y
			drag_start_value = fatness_value
			# Capture mouse
			get_viewport().set_input_as_handled()
	else:
		if is_dragging:
			is_dragging = false
			get_viewport().set_input_as_handled()

func _handle_mouse_motion(event: InputEventMouseMotion) -> void:
	if not is_dragging:
		return

	# Calculate value change based on vertical drag
	var delta_y := drag_start_y - event.position.y
	var delta_value := delta_y * KNOB_SENSITIVITY

	# Update fatness value
	fatness_value = clampf(drag_start_value + delta_value, 0.0, 1.0)

	# Update visuals
	_update_knob_visual()
	_update_value_label()

	# Send to DSP bridge
	if dsp_bridge:
		dsp_bridge.set_fatness(fatness_value)

	get_viewport().set_input_as_handled()

func _update_knob_visual() -> void:
	if not knob_indicator:
		return

	# Calculate rotation angle
	var angle := lerpf(KNOB_MIN_ANGLE, KNOB_MAX_ANGLE, fatness_value)

	# Rotate the knob indicator
	fatness_knob.rotation_degrees = angle

func _update_value_label() -> void:
	if not value_label:
		return

	value_label.text = "%d%%" % roundi(fatness_value * 100.0)

func _update_meters() -> void:
	if not meter_left or not meter_right:
		return

	# Calculate meter heights
	var left_height := lerpf(METER_MIN_HEIGHT, METER_MAX_HEIGHT, meter_left_value)
	var right_height := lerpf(METER_MIN_HEIGHT, METER_MAX_HEIGHT, meter_right_value)

	# Update meter sizes (grow from bottom)
	var base_top := 80.0
	var base_bottom := 320.0

	meter_left.offset_top = base_bottom - left_height
	meter_right.offset_top = base_bottom - right_height

	# Color based on level (green -> yellow -> red)
	meter_left.color = _level_to_color(meter_left_value)
	meter_right.color = _level_to_color(meter_right_value)

func _level_to_color(level: float) -> Color:
	if level < 0.6:
		# Green
		return Color(0.2, 0.8, 0.3, 1.0)
	elif level < 0.85:
		# Yellow
		var t := (level - 0.6) / 0.25
		return Color(0.2 + 0.8 * t, 0.8, 0.3 * (1.0 - t), 1.0)
	else:
		# Red
		return Color(1.0, 0.2, 0.1, 1.0)

# Called by host to set parameter value
func set_fatness(value: float) -> void:
	fatness_value = clampf(value, 0.0, 1.0)
	_update_knob_visual()
	_update_value_label()
