extends Node
## Window Hider - Hides the main Godot window when running as embedded UI
## Part of the Enlil/GodotVST Framework

func _ready() -> void:
	# Hide the main window since we're rendering to an external DPF window
	# The SubViewport renders our UI and FrameExporter blits it to DPF
	call_deferred("_hide_window")

func _hide_window() -> void:
	# Minimize the window to hide it
	DisplayServer.window_set_mode(DisplayServer.WINDOW_MODE_MINIMIZED)
