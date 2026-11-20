# MotionFX 🎬✨

> **Advanced Motion Graphics, 3D Transforms & Native Scene Item Show/Hide Transitions for OBS Studio 31+**

[![OBS Studio](https://img.shields.io/badge/OBS%20Studio-31.1%2B-blue.svg)](https://obsproject.com/)
[![License](https://img.shields.io/badge/License-GPLv2-green.svg)](LICENSE)
[![Build Multi-Platform](https://github.com/kubolive/MotionFX/actions/workflows/build.yml/badge.svg)](https://github.com/kubolive/MotionFX/actions/workflows/build.yml)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20macOS%20%7C%20Linux-brightgreen.svg)]()

**MotionFX** is a high-performance native OBS Studio plugin (`OBS_SOURCE_TYPE_TRANSITION`) that brings professional broadcast-grade motion graphics and animation transitions directly to individual **Scene Items**.

Unlike standard OBS scene transitions that only switch between whole scenes, MotionFX integrates directly into the **right-click context menu** of every Scene Item (`Show Transition` and `Hide Transition`).

---

## 🌟 Key Features

### 1. 🎭 Native Scene Item Show & Hide Transitions
- Selectable directly via OBS right-click context menu:
  - `Right-click Scene Item -> Show Transition -> MotionFX Transition`
  - `Right-click Scene Item -> Hide Transition -> MotionFX Transition`
- Independent Show and Hide configurations per item.
- Multiple Scene Items referencing the same source animate independently.

### 2. 🔄 True Mathematical Reverse (Hide Transition)
- **Zero visual jumping or glitching:** When hiding an item, MotionFX automatically evaluates the reverse timeline ($1.0 \to 0.0$), smoothly carrying the item from its resting on-screen position back into its hidden state.

### 3. 📐 3D Perspective & Spatial Transforms
- **Euler 3D Rotations:** Full 3D card flips on X-axis (Vertical Flip) and Y-axis (Horizontal Card Flip).
- **Transform Anchor (Pivot):** Pivot around `Center`, `Top Edge`, `Bottom Edge`, `Left Edge`, or `Right Edge`.
- **Z-Depth Translation:** Move sources forward/backward in 3D perspective space.
- Dual-sided polygon rendering (`GS_NEITHER` cull mode) for full 180° card flips.

### 4. 🎨 Advanced Shader Visual Effects
- **Text Reveal (Typewriter):** Letter-by-letter reveal for lower thirds, headlines, and captions.
- **Character Stagger:** Cascading, stepped letter appearance.
- **Mask Wipes:**
  - Linear Diagonal Wipe (45° or custom angle)
  - Radial Circle Wipe (Expanding Iris)
  - Box Wipe (Expanding rectangle)
  - Configurable edge feathering (softness).
- **Depth-of-field Blur Focus:** Smooth 9-tap Gaussian blur dissipation combined with opacity fade.

### 5. 🎛️ Composable Multi-Track Engine
- Mix and match **Slide (X/Y/Z)**, **Scale (Zoom)**, **Rotation (2D Z)**, **3D Perspective (X/Y)**, **Opacity (Fade)**, **Mask Wipe**, and **Gaussian Blur**.
- Comprehensive Easing library: `Ease Out`, `Ease In`, `Ease In-Out`, `Back Out` (Overshoot), `Elastic Out` (Spring), `Bounce Out` (Physics bounce), and `Linear`.

### 6. 🧹 Clean, Modern Dynamic Properties UI
- Preset-driven by default for fast workflow.
- Collapsible parameter groups with dynamic visibility: manual sliders only expand when `Custom Parameter Overrides` is checked.
- Multi-language support: English (`en-US`) and Vietnamese (`vi-VN`).

---

## 📦 Installation

Download the latest releases from the [Releases](https://github.com/kubolive/MotionFX/releases) page:

### 🪟 Windows (x64)
- **Automatic Installer (`.exe`):**
  Download `MotionFX-v1.0.0-Windows-Installer.exe` and run the wizard. It automatically locates your OBS Studio installation and copies all necessary files.
- **Manual Portable (`.zip`):**
  Extract `MotionFX-v1.0.0-Windows-x64.zip` into your OBS Studio root directory (e.g. `C:\Program Files\obs-studio\`):
  ```text
  C:\Program Files\obs-studio\
  ├── obs-plugins\64bit\motionfx.dll
  └── data\obs-plugins\motionfx\
      ├── motionfx_3d.effect
      └── locale\
          ├── en-US.ini
          └── vi-VN.ini
  ```

### 🐧 Linux (Ubuntu / Debian / x64)
- **Debian Package (`.deb`):**
  ```bash
  sudo dpkg -i MotionFX-v1.0.0-Linux-x64.deb
  # Or via apt:
  sudo apt-get install -f
  ```
- **Portable Tarball (`.tar.gz` / `.zip`):**
  Extract into `~/.config/obs-studio/plugins/motionfx/` or `/usr/lib/obs-plugins/`.

### 🍎 macOS (Apple Silicon M-Series & Intel)
- **Native Package Installer (`.pkg`):**
  Double click `MotionFX-v1.0.0-macOS.pkg` and follow the installer. It installs `motionfx.plugin` into `/Library/Application Support/obs-studio/plugins`.
- **Manual Bundle (`.zip`):**
  Extract `MotionFX-v1.0.0-macOS.zip` and copy `motionfx.plugin` into:
  `~/Library/Application Support/obs-studio/plugins/` (or `/Library/Application Support/obs-studio/plugins/`).

---

## 🛠️ Building from Source

### Requirements
- Windows 10/11 x64
- C++20 compliant compiler (GCC 13+ / MinGW-w64 or MSVC 2022)
- libobs development headers and `libobs.a` / `obs.lib`

### Quick Build with MinGW (w64devkit)
```bash
g++ -shared -std=c++20 -DOBS_PLUGIN_BUILD=1 -Iinclude -Iobs-repo/libobs -O2 \
    src/motionfx_plugin.cpp \
    src/motionfx_easing.cpp \
    src/motionfx_timeline.cpp \
    src/motionfx_tracks.cpp \
    src/motionfx_engine.cpp \
    src/motionfx_3d.cpp \
    src/motionfx_effects.cpp \
    src/motionfx_presets.cpp \
    src/motionfx_dispatcher.cpp \
    libobs.a -o build/motionfx.dll -static-libgcc -static-libstdc++
```

---

## 📜 License

This project is licensed under the **GNU General Public License v2.0 (GPL-2.0)** to remain fully compliant with OBS Studio (libobs). See the [LICENSE](LICENSE) file for details.
