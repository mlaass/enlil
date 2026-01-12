# Enlil/GodotVST Framework - Convenience Makefile
#
# This wraps SCons for common build tasks.
# Run 'make help' for available targets.

.PHONY: all godot godot-cpp bridge plugin clean help setup

# Default number of parallel jobs
JOBS ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Default target
all: bridge

# Build LibGodot (stripped shared library, ~30 min)
# Output: godot/bin/libgodot.linux.template_release.x86_64.so
godot:
	cd godot && scons \
		platform=linux \
		target=template_release \
		library_type=shared_library \
		optimize=size \
		debug_symbols=no \
		deprecated=no \
		disable_3d=yes \
		module_bullet_enabled=no \
		module_navigation_enabled=no \
		module_openxr_enabled=no \
		module_multiplayer_enabled=no \
		module_enet_enabled=no \
		module_websocket_enabled=no \
		module_text_server_adv_enabled=no \
		-j$(JOBS)

# Build LibGodot debug (for development)
godot-debug:
	cd godot && scons \
		platform=linux \
		target=template_debug \
		library_type=shared_library \
		disable_3d=yes \
		module_bullet_enabled=no \
		module_navigation_enabled=no \
		module_openxr_enabled=no \
		module_multiplayer_enabled=no \
		module_enet_enabled=no \
		module_websocket_enabled=no \
		module_text_server_adv_enabled=no \
		-j$(JOBS)

# Build godot-cpp bindings
godot-cpp:
	cd godot-cpp && scons platform=linux target=template_release -j$(JOBS)

godot-cpp-debug:
	cd godot-cpp && scons platform=linux target=template_debug -j$(JOBS)

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

# Help
help:
	@echo "Enlil/GodotVST Framework Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Build bridge (default)"
	@echo "  godot           - Build LibGodot stripped (~30 min, ~25MB)"
	@echo "  godot-debug     - Build LibGodot debug (~30 min)"
	@echo "  godot-cpp       - Build godot-cpp bindings (release)"
	@echo "  godot-cpp-debug - Build godot-cpp bindings (debug)"
	@echo "  bridge          - Build FatSat GDExtension bridge"
	@echo "  bridge-release  - Build bridge (optimized)"
	@echo "  plugin          - Build FatSat VST3/CLAP/LV2 plugin"
	@echo "  plugin-release  - Build plugin (optimized)"
	@echo "  setup           - Initialize git submodules"
	@echo "  clean           - Remove build artifacts"
	@echo "  distclean       - Deep clean including submodules"
	@echo "  editor          - Run Godot editor with plugin project"
	@echo ""
	@echo "Variables:"
	@echo "  JOBS=N        - Number of parallel jobs (default: $(JOBS))"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build bridge"
	@echo "  make plugin-release     # Build optimized plugin"
	@echo "  make JOBS=16 godot      # Build LibGodot with 16 jobs"
