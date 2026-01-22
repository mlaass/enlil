#!/usr/bin/make -f
# Makefile for FatSat Plugin #
# -------------------------- #
# Part of the Enlil/GodotVST Framework

# --------------------------------------------------------------
# Project name, used for binaries

NAME = FatSat

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	FatSatPlugin.cpp

# UI files for Godot-based interface
# Include bridge sources directly since we register GDExtension classes in the plugin
FILES_UI = \
	FatSatUI.cpp \
	../bridge/fatsat_bridge.cpp \
	../bridge/frame_bridge_gd.cpp

# --------------------------------------------------------------
# DPF path (can be overridden)

DPF_PATH ?= ../../dpf

# --------------------------------------------------------------
# Build and output directories

DPF_BUILD_DIR ?= ../../build/plugin
DPF_TARGET_DIR ?= ../../build/bin

# --------------------------------------------------------------
# godot-cpp integration for UI

GODOT_CPP_PATH ?= ../../godot-cpp

# Add include paths for godot-cpp, shared code, and bridge (using CXXFLAGS since DPF resets BUILD_CXX_FLAGS)
CXXFLAGS += -I$(GODOT_CPP_PATH)/include
CXXFLAGS += -I$(GODOT_CPP_PATH)/gen/include
CXXFLAGS += -I$(GODOT_CPP_PATH)/gdextension
CXXFLAGS += -I../shared
CXXFLAGS += -I../bridge
CXXFLAGS += -std=c++17

# Add godot-cpp library and OpenGL for UI linking
EXTRA_UI_LIBS += -L$(GODOT_CPP_PATH)/bin -lgodot-cpp.linux.template_release.x86_64 -ldl -lGL

# --------------------------------------------------------------
# Do some magic

include $(DPF_PATH)/Makefile.plugins.mk

# --------------------------------------------------------------
# Plugin targets

TARGETS += lv2_sep
TARGETS += vst3
TARGETS += clap

# Standalone/JACK for testing
ifeq ($(HAVE_JACK),true)
TARGETS += jack
endif

all: $(TARGETS)

# --------------------------------------------------------------
