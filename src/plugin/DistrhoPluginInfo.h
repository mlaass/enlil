/*
 * FatSat - One-knob saturation and dynamics plugin
 * Part of the Enlil/GodotVST Framework
 */

#ifndef DISTRHO_PLUGIN_INFO_H
#define DISTRHO_PLUGIN_INFO_H

#define DISTRHO_PLUGIN_BRAND   "Enlil"
#define DISTRHO_PLUGIN_NAME    "FatSat"
#define DISTRHO_PLUGIN_URI     "https://github.com/mlaass/enlil"
#define DISTRHO_PLUGIN_CLAP_ID "com.enlil.fatsat"

#define DISTRHO_PLUGIN_BRAND_ID  Enli
#define DISTRHO_PLUGIN_UNIQUE_ID eFat

#define DISTRHO_PLUGIN_HAS_UI           1
#define DISTRHO_PLUGIN_IS_RT_SAFE       1
#define DISTRHO_PLUGIN_NUM_INPUTS       2
#define DISTRHO_PLUGIN_NUM_OUTPUTS      2
#define DISTRHO_PLUGIN_WANT_TIMEPOS     0
#define DISTRHO_PLUGIN_WANT_STATE       1
#define DISTRHO_PLUGIN_WANT_FULL_STATE  0

// UI configuration for Godot-based interface
#define DISTRHO_UI_USER_RESIZABLE       0
#define DISTRHO_UI_DEFAULT_WIDTH        600
#define DISTRHO_UI_DEFAULT_HEIGHT       400

#endif // DISTRHO_PLUGIN_INFO_H
