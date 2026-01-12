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

# UI disabled for now - will enable when Godot integration is ready
# FILES_UI = \
# 	FatSatUI.cpp

# --------------------------------------------------------------
# DPF path (can be overridden)

DPF_PATH ?= ../../dpf

# --------------------------------------------------------------
# Build and output directories

DPF_BUILD_DIR ?= ../../build/plugin
DPF_TARGET_DIR ?= ../../build/bin

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
