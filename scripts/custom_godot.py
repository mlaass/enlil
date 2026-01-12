"""
Custom Godot Build Options for Enlil/GodotVST Framework

This configuration strips Godot to ~30MB by disabling unused features.
Copy relevant options to godot/custom.py before building.
"""

# Build as shared library for embedding
library_type = "shared_library"

# Optimization
optimize = "size"
debug_symbols = "no"
deprecated = "no"

# Disable 3D (major size reduction)
disable_3d = "yes"

# Disable physics (not needed for UI)
module_bullet_enabled = "no"
module_godot_physics_2d_enabled = "no"
module_godot_physics_3d_enabled = "no"

# Disable navigation (not needed)
module_navigation_enabled = "no"

# Disable XR (not needed)
module_webxr_enabled = "no"
module_openxr_enabled = "no"

# Disable multiplayer/networking (not needed)
module_multiplayer_enabled = "no"
module_enet_enabled = "no"
module_websocket_enabled = "no"
module_upnp_enabled = "no"
module_mbedtls_enabled = "no"

# Use simple text server (smaller than advanced)
module_text_server_adv_enabled = "no"
module_text_server_fb_enabled = "yes"

# Disable heavy 3D formats
module_gltf_enabled = "no"
module_gridmap_enabled = "no"
module_csg_enabled = "no"
module_lightmapper_rd_enabled = "no"

# Disable unused texture formats
module_basis_universal_enabled = "no"
module_cvtt_enabled = "no"
module_squish_enabled = "no"
module_etcpak_enabled = "no"
module_astcenc_enabled = "no"

# Keep these enabled for UI
module_gdscript_enabled = "yes"
module_svg_enabled = "yes"
module_freetype_enabled = "yes"

# Production settings for release
production = "yes"
