"""
Custom Godot Build Options for Enlil/GodotVST Framework

This configuration strips Godot to <30MB by disabling unused features.
Copy to godot/custom.py or pass as SCons arguments.

Target: <30MB stripped shared library for VST plugin UI
"""

# Build type
library_type = "shared_library"
target = "template_release"
optimize = "size"
debug_symbols = "no"
deprecated = "no"
production = "yes"

# Disable 3D (major size reduction)
disable_3d = "yes"

# Disable physics (not needed for UI)
disable_physics_2d = "yes"
disable_navigation_2d = "yes"

# Disable Vulkan (use OpenGL only for 2D, ~5-10MB savings)
vulkan = "no"
use_volk = "no"

# Disable SDL input (DPF handles input)
sdl = "no"

# Disable accessibility SDK (VST host handles this)
accesskit = "no"

# Disable Brotli (only needed for WOFF2 fonts)
brotli = "no"

# Modules to disable
module_bullet_enabled = "no"          # Physics
module_navigation_enabled = "no"      # Navigation
module_openxr_enabled = "no"          # XR
module_multiplayer_enabled = "no"     # Multiplayer
module_enet_enabled = "no"            # Networking
module_websocket_enabled = "no"       # WebSocket
module_text_server_adv_enabled = "no" # Advanced text (use fallback)
module_theora_enabled = "no"          # Video playback
module_vorbis_enabled = "no"          # Audio (DPF handles audio)
module_ogg_enabled = "no"             # Audio containers
module_noise_enabled = "no"           # Noise generation
module_camera_enabled = "no"          # Camera access
module_cvtt_enabled = "no"            # Texture compression
module_squish_enabled = "no"          # DXT compression
module_etcpak_enabled = "no"          # ETC compression
module_astcenc_enabled = "no"         # ASTC compression
module_basis_universal_enabled = "no" # Basis Universal textures
module_miniupnpc_enabled = "no"       # UPnP networking
module_mbedtls_enabled = "no"         # TLS/SSL
module_upnp_enabled = "no"            # UPnP
module_jsonrpc_enabled = "no"         # JSON-RPC

# Keep enabled (needed for UI)
module_freetype_enabled = "yes"       # Font rendering
module_svg_enabled = "yes"            # Vector graphics
module_gdscript_enabled = "yes"       # Scripting
module_regex_enabled = "yes"          # GDScript regex
module_jpg_enabled = "yes"            # JPEG images
module_webp_enabled = "yes"           # WebP images
module_png_enabled = "yes"            # PNG images (implicit)
