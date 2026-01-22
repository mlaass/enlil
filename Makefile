# Enlil/GodotVST Framework - Convenience Makefile
#
# This wraps SCons for common build tasks.
# Run 'make help' for available targets.

.PHONY: all godot godot-editor godot-cpp extension-api bridge plugin libgodot-test run-libgodot-test clean help setup test test-standalone test-lv2

# Default number of parallel jobs
JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Default target
all: bridge

# Build LibGodot (stripped shared library, ~30 min, target <30MB)
# Output: godot/bin/libgodot.linuxbsd.template_release.x86_64.so
godot:
	cd godot && scons \
		platform=linux \
		target=template_release \
		library_type=shared_library \
		optimize=size \
		debug_symbols=no \
		deprecated=yes \
		production=yes \
		disable_path_overrides=no \
		disable_3d=no \
		disable_physics_2d=no \
		disable_navigation_2d=no \
		vulkan=no \
		use_volk=no \
		wayland=no \
		sdl=no \
		accesskit=no \
		brotli=no \
		module_bullet_enabled=no \
		module_navigation_enabled=no \
		module_openxr_enabled=no \
		module_multiplayer_enabled=no \
		module_enet_enabled=no \
		module_websocket_enabled=no \
		module_text_server_adv_enabled=no \
		module_theora_enabled=no \
		module_vorbis_enabled=no \
		module_ogg_enabled=no \
		module_noise_enabled=no \
		module_camera_enabled=no \
		module_cvtt_enabled=no \
		module_squish_enabled=no \
		module_etcpak_enabled=no \
		module_astcenc_enabled=no \
		module_basis_universal_enabled=no \
		module_miniupnpc_enabled=no \
		module_mbedtls_enabled=no \
		module_upnp_enabled=no \
		module_jsonrpc_enabled=no \
		module_lightmapper_rd_enabled=no \
		module_embree_enabled=no \
		module_raycast_enabled=no \
		module_meshoptimizer_enabled=no \
		module_xatlas_unwrap_enabled=no \
		module_glslang_enabled=no \
		module_gltf_enabled=no \
		module_gridmap_enabled=no \
		module_csg_enabled=no \
		module_hdr_enabled=no \
		module_ktx_enabled=no \
		module_tga_enabled=no \
		module_bmp_enabled=no \
		module_dds_enabled=no \
		module_exr_enabled=no \
		module_tinyexr_enabled=no \
		-j$(JOBS)

# Build LibGodot debug (for development)
godot-debug:
	cd godot && scons \
		platform=linux \
		target=template_debug \
		library_type=shared_library \
		disable_3d=yes \
		disable_physics_2d=yes \
		disable_navigation_2d=yes \
		vulkan=no \
		use_volk=no \
		wayland=no \
		sdl=no \
		accesskit=no \
		brotli=no \
		module_bullet_enabled=no \
		module_navigation_enabled=no \
		module_openxr_enabled=no \
		module_multiplayer_enabled=no \
		module_enet_enabled=no \
		module_websocket_enabled=no \
		module_text_server_adv_enabled=no \
		module_theora_enabled=no \
		module_vorbis_enabled=no \
		module_ogg_enabled=no \
		module_noise_enabled=no \
		module_camera_enabled=no \
		module_cvtt_enabled=no \
		module_squish_enabled=no \
		module_etcpak_enabled=no \
		module_astcenc_enabled=no \
		module_basis_universal_enabled=no \
		module_miniupnpc_enabled=no \
		module_mbedtls_enabled=no \
		module_upnp_enabled=no \
		module_jsonrpc_enabled=no \
		module_lightmapper_rd_enabled=no \
		module_embree_enabled=no \
		module_raycast_enabled=no \
		module_meshoptimizer_enabled=no \
		module_xatlas_unwrap_enabled=no \
		module_glslang_enabled=no \
		module_gltf_enabled=no \
		module_gridmap_enabled=no \
		module_csg_enabled=no \
		module_hdr_enabled=no \
		module_ktx_enabled=no \
		module_tga_enabled=no \
		module_bmp_enabled=no \
		module_dds_enabled=no \
		module_exr_enabled=no \
		module_tinyexr_enabled=no \
		-j$(JOBS)

# Build Godot editor executable (needed for extension_api.json generation)
godot-editor:
	cd godot && scons \
		platform=linux \
		target=editor \
		-j$(JOBS)

# Generate extension_api.json from LibGodot and rebuild godot-cpp
# This is needed when GodotInstance or other LibGodot-specific classes are missing from bindings
extension-api: godot-editor
	cd godot/bin && ./godot.linuxbsd.editor.x86_64 --dump-extension-api --headless
	mv godot/bin/extension_api.json godot-cpp/gdextension/
	cd godot-cpp && scons platform=linux target=template_release -j$(JOBS)

# Build godot-cpp bindings
godot-cpp:
	cd godot-cpp && scons platform=linux target=template_release -j$(JOBS)

godot-cpp-debug:
	cd godot-cpp && scons platform=linux target=template_debug -j$(JOBS)

# Build and run LibGodot test sample
libgodot-test:
	$(MAKE) -C src/shared/libgodot_test

run-libgodot-test: libgodot-test
	$(MAKE) -C src/shared/libgodot_test run

# Build FatSat GDExtension bridge
bridge:
	scons bridge -j$(JOBS)

bridge-release:
	scons bridge target=release -j$(JOBS)

# Build FatSat DPF plugin
plugin: bridge
	scons plugin -j$(JOBS)

plugin-release: bridge-release
	scons plugin target=release -j$(JOBS)

# Initialize submodules (first-time setup)
setup:
	git submodule update --init --recursive

# Clean build artifacts
clean:
	rm -rf build/
	scons -c

# Deep clean (including submodule builds)
distclean: clean
	cd godot && scons -c || true
	cd godot-cpp && scons -c || true

# Run Godot editor with the plugin project
editor:
	cd src/godot && ../../godot/bin/godot.* --editor

# Test targets
test: test-standalone

# Run standalone with PipeWire JACK emulation
test-standalone: plugin
	@echo "Running FatSat standalone (press Ctrl+C to stop)..."
	pw-jack ./build/bin/FatSat

# Run LV2 plugin in jalv host
test-lv2: plugin
	@echo "Running FatSat LV2 in jalv (press Ctrl+C to stop)..."
	pw-jack jalv ./build/bin/FatSat.lv2/FatSat.so

# Help
help:
	@echo "Enlil/GodotVST Framework Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Build bridge (default)"
	@echo "  godot           - Build LibGodot stripped (~30 min, ~25MB)"
	@echo "  godot-debug     - Build LibGodot debug (~30 min)"
	@echo "  godot-editor    - Build Godot editor (for extension-api)"
	@echo "  extension-api   - Generate extension_api.json and rebuild godot-cpp"
	@echo "  godot-cpp       - Build godot-cpp bindings (release)"
	@echo "  godot-cpp-debug - Build godot-cpp bindings (debug)"
	@echo "  libgodot-test   - Build LibGodot test sample"
	@echo "  run-libgodot-test - Build and run LibGodot test sample"
	@echo "  bridge          - Build FatSat GDExtension bridge"
	@echo "  bridge-release  - Build bridge (optimized)"
	@echo "  plugin          - Build FatSat VST3/CLAP/LV2 plugin"
	@echo "  plugin-release  - Build plugin (optimized)"
	@echo "  setup           - Initialize git submodules"
	@echo "  clean           - Remove build artifacts"
	@echo "  distclean       - Deep clean including submodules"
	@echo "  editor          - Run Godot editor with plugin project"
	@echo "  test            - Run plugin standalone with audio"
	@echo "  test-standalone - Run JACK standalone via PipeWire"
	@echo "  test-lv2        - Run LV2 plugin in jalv host"
	@echo ""
	@echo "Variables:"
	@echo "  JOBS=N        - Number of parallel jobs (default: $(JOBS))"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build bridge"
	@echo "  make plugin-release     # Build optimized plugin"
	@echo "  make JOBS=16 godot      # Build LibGodot with 16 jobs"
