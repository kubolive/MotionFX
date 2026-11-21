# OBS Project Policy & Guidelines Compliance Statement

This document details MotionFX's strict adherence to the **OBS Project Resource & Intellectual Property (IP) Policy** and the **OBS AI/Machine Learning Policy**.

---

## 1. OBS Project Resource and IP Policy Compliance

### A. Project Naming and Trademark Usage
- **Plugin Name:** The project is officially named **MotionFX** (or described as *MotionFX for OBS Studio*).
- **No Trademark Infringement:** The project does not use the letters "OBS" as part of its primary name (e.g. avoided names like *OBS MotionFX*).
- **Descriptor Exemption:** In accordance with the OBS Resource Guidelines, phrasing such as *"for OBS Studio"* is used strictly as a descriptive compatibility notice.
- **Logos and Visual Branding:** MotionFX uses its own distinct branding and does not use or alter the official OBS Studio logo, trademarks, or visual assets.

### B. Third-Party Disclaimer
> **Disclaimer:** MotionFX is an independent third-party open-source project and is not affiliated with, endorsed by, or sponsored by the OBS Project, OBS Studio, or its authors/maintainers.

### C. Open Source Licensing (GNU GPLv2)
- MotionFX links directly against `libobs` and complies with the **GNU General Public License v2.0 (GPL-2.0)**.
- Full source code is publicly accessible on GitHub: [https://github.com/kubolive/MotionFX](https://github.com/kubolive/MotionFX).
- All distribution packages (Windows `.exe`/`.zip`, Linux `.deb`/`.tar.gz`, macOS `.pkg`/`.zip`) are hosted transparently on public GitHub Releases.

---

## 2. OBS AI / Machine Learning Policy Compliance

The OBS Project [CONTRIBUTING.md AI/Machine Learning Policy](https://github.com/obsproject/obs-studio/blob/master/CONTRIBUTING.md#aimachine-learning-policy) addresses risks regarding unverified, buggy, or hallucinated AI-generated code that violates OBS internals or creates maintainer burden.

MotionFX adheres to rigorous software engineering standards to address every concern raised in the policy:

### A. Human Engineering & Code Architecture Verification
- **Internal API Correctness:** All interactions with `libobs` (`OBS_SOURCE_TYPE_TRANSITION`, `obs_source_info`, `obs_transition_get_time`, `gs_effect_t`, `gs_matrix_*`) follow official OBS Studio architectural specifications.
- **Strict Memory & Resource Lifecycle:**
  - Graphics effects (`gs_effect_create_from_file`, `gs_effect_destroy`) are strictly wrapped within `obs_enter_graphics()` and `obs_leave_graphics()`.
  - Memory allocated via `obs_module_file()` or libobs helpers is consistently released using `bfree()`.
  - Reference counts on `obs_source_t` and `obs_data_t` are balanced with matching `obs_source_release()` and `obs_data_release()`.
- **Thread Safety:** The graphics rendering pipeline (`motionfx_tr_video_render`) runs entirely on the OBS video render thread, avoiding cross-thread race conditions or UI thread deadlocks.

### B. Clean-Room Implementation & Mathematical Correctness
- **Custom Shaders:** The HLSL/Cg shader (`data/motionfx_3d.effect`) uses standard graphics mathematics:
  - 3D Euler rotation matrices: $R_x(\theta)$, $R_y(\theta)$, $R_z(\theta)$.
  - Smoothstep boundary clamping: $\text{clamp}((d - (p - f)) / f, 0.0, 1.0)$.
  - Premultiplied alpha correction to eliminate dark fringe artifacts.
- **Zero Third-Party Code Contamination:** All multi-track easing curves, timeline sequencers, and preset definitions are cleanly implemented from mathematical definitions (Robert Penner's open algorithms and CSS transitions).

### C. Comprehensive Multi-Platform Verification
Every release is compiled and verified across all three supported operating systems in clean CI environments:
- **Windows (x64):** MinGW-w64 C++20 with static libgcc/libstdc++.
- **Linux (x64):** Native ABI compatibility with `libobs-dev` (Ubuntu 24.04).
- **macOS (Universal):** Fat Mach-O binary (`arm64` Apple Silicon + `x86_64` Intel) with `-undefined dynamic_lookup`.
