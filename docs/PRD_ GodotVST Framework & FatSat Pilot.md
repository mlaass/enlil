# **Product Requirements Document (PRD)**

## **Project: GodotVST Framework & "FatSat" Pilot**

**Version:** 1.0

**Status:** Draft

**Target Platforms:** Windows, macOS, Linux (VST3 / CLAP / LV2)

## **Part 1: The GodotVST Framework**

### **1\. Executive Summary**

The **GodotVST Framework** is a reusable development template designed to solve the hardest problem in audio plugin development: creating rich, animated, high-performance User Interfaces.

By combining the **DISTRHO Plugin Framework (DPF)** for rock-solid cross-platform audio hosting and **LibGodot** (Godot Engine 4.x as a shared library) for the UI, this framework allows developers to:

1. Write DSP code in high-performance **C++**.  
2. Build UIs using the **Godot Editor**, leveraging its animation system, particle effects, and "web-like" flexbox layout containers.  
3. Compile to a single binary (DLL/VST3) with a footprint under 30MB.

### **2\. Technical Architecture**

#### **2.1 Core Components**

| Component | Technology | Responsibility |
| :---- | :---- | :---- |
| **Host Shell** | **DPF** (DISTRHO Plugin Framework) | Handles VST3/CLAP/LV2 entry points, OS window creation, parameter automation, and the audio process loop. |
| **Guest UI** | **LibGodot** (Godot 4.x) | Runs the UI scene. Compiled as a shared library (libgodot), not an executable. Renders into the DPF window handle. |
| **The Bridge** | **GDExtension** (C++) | A custom C++ interface that binds the DSP engine to the Godot Scene Tree. Handles thread-safe communication. |
| **UI Logic** | **GDScript** | Handles knob rotations, animations, and visual logic. |

#### **2.2 Data Flow & Threading**

To ensure audio stability, the DSP and UI threads remain decoupled.

* **Audio Thread (Real-time Priority):**  
  * DPF receives audio buffer \-\> Runs DSP algorithm \-\> Writes to Output.  
  * *Constraint:* Never locks, never allocates memory.  
  * *Reads:* Atomic parameter values (e.g., drive\_amount).  
  * *Writes:* Visualization data (e.g., rms\_level) to a lock-free Ring Buffer.  
* **UI Thread (Main Thread):**  
  * Godot runs its frame loop (60fps).  
  * *Polls:* The Bridge checks the Ring Buffer for new visualization data.  
  * *Updates:* GDScript updates the UI (e.g., knob.rotation, meter.value).  
  * *Input:* User turns a knob \-\> GDScript calls Bridge.set\_param(id, value) \-\> Bridge updates Atomic value for DSP.

### **3\. Critical Implementation Details**

#### **3.1 The "Stripped" Godot Build**

Standard Godot is too large (\~60-80MB). We will compile a custom LibGodot using SCons to strip unnecessary modules, targeting a binary size of **\<30MB**.

* **Disabled Modules:** 3D Engine (disable\_3d=yes), Physics (2D/3D), Navigation, XR, Multiplayer, TextServerAdvanced (unless needed for specific fonts).  
* **Enabled Modules:** 2D Rendering, GDScript, GDExtension, Image/Texture Loaders.

#### **3.2 Single Binary Distribution (PCK Embedding)**

We cannot ask users to install Godot or manage separate .pck files.

* **Mechanism:** The Godot project assets (.pck) will be embedded directly into the C++ binary as a static byte array.  
* **Loading:** The framework will initialize LibGodot using a custom FileAccess class that reads from this memory location instead of the disk.

#### **3.3 Window Parenting**

* **DPF** creates the OS window (HWND/Cocoa View/X11).  
* **The Framework** grabs the native handle void\* windowHandle.  
* **LibGodot** is initialized with this handle as its parent, rendering its Viewport directly inside the VST window.

## **Part 2: Pilot Plugin Specification ("FatSat")**

### **1\. Product Concept**

**Name:** FatSat (Working Title)

**Concept:** A one-knob saturation and dynamics plugin inspired by "Sausage Fattener." It makes sounds "fatter" by combining soft-clipping saturation with compression/limiting.

**Visual Hook:** As the knob turns up, the UI reacts dynamically (e.g., a face gets angrier, or the knob glows hotter).

### **2\. Functional Requirements**

#### **2.1 The DSP Engine**

* **Algorithm:**  
  * Input Gain Stage.  
  * **Tanh Soft Clipper:** output \= tanh(input \* drive).  
  * **Ceiling Limiter:** Hard clip at \-0.1dB to prevent digital nasty clipping.  
  * Output Gain Compensation (Auto-gain roughly 50%).  
* **Parameters:**  
  1. FATNESS (0.0 to 1.0): Controls input drive and saturation curve.  
  2. OUTPUT (0.0 to 1.0): Output trim (optional, maybe hidden or smaller).

#### **2.2 The User Interface (Godot)**

* **Layout:**  
  * Background: Dark, industrial texture.  
  * **Centerpiece:** A massive, high-quality 3D-rendered knob (rendered to 2D spritesheet) or a procedural vector knob.  
  * **Visualizer:** A "Fatness Indicator." This could be a character face that changes sprite frames based on the FATNESS value (Happy \-\> Strained \-\> Furious).  
* **Animations:**  
  * **Idle:** The face/knob gently breathes (scale sine wave).  
  * **Reactive:** The face/knob shakes or pulses with the audio signal amplitude (received via Bridge).

### **3\. Development Roadmap**

#### **Phase 1: The Skeleton (Week 1\)**

* \[ \] Set up the SCons build system to compile DPF and LibGodot together.  
* \[ \] Create the "Stripped" Godot 4.5+ export template.  
* \[ \] **Milestone:** A VST3 that opens, displays a generic "Hello Godot" label, and passes audio through unchanged.

#### **Phase 2: The Bridge (Week 2\)**

* \[ \] Implement GDExtension class FatSatBridge.  
* \[ \] Create the lock-free ring buffer for RMS metering.  
* \[ \] **Milestone:** Turning a knob in Godot prints "Value Changed" in the DAW console; Audio RMS drives a Godot progress bar.

#### **Phase 3: The Pilot Plugin (Week 3\)**

* \[ \] **DSP:** Write the tanh saturation code in the DPF run() function.  
* \[ \] **UI:** Design the "Angry Face" sprites and the Knob in Godot.  
* \[ \] **Binding:** Connect the Knob to the FATNESS parameter. Connect Audio RMS to the Face shake animation.

#### **Phase 4: Packaging (Week 4\)**

* \[ \] Implement PCK embedding (convert .pck to unsigned char\[\] header).  
* \[ \] Finalize SCons scripts for Release builds (LTO enabled, symbols stripped).  
* \[ \] **Deliverable:** FatSat.vst3 (Windows) and FatSat.component (macOS).