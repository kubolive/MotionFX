# MotionFX Developer API & Automation Guide

> **Official Automation & Developer Documentation for MotionFX OBS Studio Plugin**  
> Covers **OBS-WebSocket v5**, **Python (`obspython`)**, **Lua (`obslua`)**, and **Stream Deck / Touch Portal** integrations.

---

## Table of Contents
1. [Architecture Overview](#1-architecture-overview)
2. [Supported Animation Presets & Parameters](#2-supported-animation-presets--parameters)
3. [Method 1: OBS-WebSocket v5 (Remote Automation)](#3-method-1-obs-websocket-v5-remote-automation)
4. [Method 2: OBS Python Scripting (`obspython`)](#4-method-2-obs-python-scripting-obspython)
5. [Method 3: OBS Lua Scripting (`obslua`)](#5-method-3-obs-lua-scripting-obslua)
6. [Method 4: Stream Deck & Touch Portal Integration](#6-method-4-stream-deck--touch-portal-integration)

---

## 1. Architecture Overview

MotionFX is implemented as a native **OBS Studio Transition Source** (`OBS_SOURCE_TYPE_TRANSITION` with source ID `motionfx_transition`).

In OBS Studio (version 27+), individual **Scene Items** support dedicated **Show Transitions** and **Hide Transitions**.
When a Scene Item transitions:
- **Show Transition:** Fires when the item becomes visible (`visible = true`). MotionFX smoothly animates the source into view ($0.0 \to 1.0$).
- **Hide Transition:** Fires when the item is hidden (`visible = false`). MotionFX mathematically evaluates the **reverse timeline** ($1.0 \to 0.0$), carrying the source smoothly back to its hidden state.

Because MotionFX hooks directly into the native Scene Item visibility lifecycle:
- Any WebSocket call (`SetSceneItemEnabled`), Python/Lua call (`obs_sceneitem_set_visible`), Hotkey press, or Stream Deck button press automatically triggers the MotionFX animation.

---

## 2. Supported Animation Presets & Parameters

### Built-in Preset IDs
Use these string identifiers in WebSocket requests, Python, and Lua:

| Preset ID | Display Name | Visual Style |
| :--- | :--- | :--- |
| `3d_flip_x` | 3D Flip X | Vertical card flip around X-axis (from 90° to 0°) |
| `3d_flip_y` | 3D Flip Y | Horizontal 3D card flip around Y-axis (from 90° to 0°) |
| `3d_flip_diagonal` | 3D Flip Diagonal | Diagonal dual-axis 3D fold |
| `perspective_tilt` | Perspective Tilt | Dynamic 3D tilt with perspective camera depth |
| `zoom_spin` | Zoom Spin | Combined 2D twist rotation with spring zoom |
| `blur_reveal` | Blur Focus | 9-tap Gaussian blur dissipation with smooth fade |
| `elastic_pop` | Elastic Pop | Physics spring overshoot dynamic bounce |
| `glitch_wipe` | Glitch Displace | Cyberpunk glitch scanline displacement |
| `mask_reveal` | Mask Reveal (Diagonal) | 45° linear angle matte wipe with soft feathering |
| `mask_radial` | Radial Iris Wipe | Circular camera iris expand / contract |
| `mask_box` | Box Wipe | Expanding rectangular vignette reveal |
| `text_reveal` | Text Typewriter | Letter-by-letter reveal for titles & lower thirds |
| `character_stagger` | Character Stagger | Stepped character appearance cascade |
| `smooth_slide_left` | Smooth Slide Left | Horizontal slide from offscreen left |
| `smooth_slide_right` | Smooth Slide Right | Horizontal slide from offscreen right |
| `smooth_slide_up` | Smooth Slide Up | Vertical slide from offscreen top |
| `smooth_slide_down` | Smooth Slide Down | Vertical slide from offscreen bottom |
| `cinematic_fly_through`| 3D Fly-Through | Dramatic 3D Z-depth camera fly-in |

### Transform Anchors (Pivots)
- `"center"`: Rotates/scales around the item center point.
- `"top"`: Folds from the top edge.
- `"bottom"`: Folds from the bottom edge.
- `"left"`: Opens like a door from the left edge.
- `"right"`: Opens from the right edge.

### Easing Curves
`"ease_out"`, `"ease_in"`, `"ease_in_out"`, `"back_out"` (overshoot), `"elastic_out"` (spring), `"bounce_out"` (physical bounce), `"linear"`.

---

## 3. Method 1: OBS-WebSocket v5 (Remote Automation)

OBS-WebSocket v5 is natively built into OBS Studio 28+.

### A. Triggering Show / Hide / Toggle Animations
To trigger MotionFX on any Scene Item, simply toggle the item's visibility using `SetSceneItemEnabled`:

#### Request Payload (Show Animation):
```json
{
  "op": 6,
  "d": {
    "requestType": "SetSceneItemEnabled",
    "requestId": "motionfx-show-req",
    "requestData": {
      "sceneName": "Gaming Scene",
      "sceneItemId": 42,
      "sceneItemEnabled": true
    }
  }
}
```

#### Request Payload (Hide / Reverse Animation):
```json
{
  "op": 6,
  "d": {
    "requestType": "SetSceneItemEnabled",
    "requestId": "motionfx-hide-req",
    "requestData": {
      "sceneName": "Gaming Scene",
      "sceneItemId": 42,
      "sceneItemEnabled": false
    }
  }
}
```

### B. Python Standalone Client Example
Run the bundled `scripts/obs_websocket_client.py`:
```bash
# Toggle item visibility
python scripts/obs_websocket_client.py "Gaming Scene" "Webcam" toggle

# Play Show animation
python scripts/obs_websocket_client.py "Gaming Scene" "AlertBox" show "myObsPassword"

# Play Hide animation
python scripts/obs_websocket_client.py "Gaming Scene" "AlertBox" hide "myObsPassword"
```

### C. Node.js / JavaScript Client Example
Use the bundled `scripts/obs_websocket_client.js`:
```javascript
const MotionFXClient = require('./scripts/obs_websocket_client.js');

async function run() {
    const client = new MotionFXClient('localhost', 4455, 'myPassword');
    await client.connect();

    // Trigger Show on 'Webcam' in 'Main Scene'
    await client.showItem('Main Scene', 'Webcam');

    // Wait 3 seconds, then trigger Hide
    setTimeout(async () => {
        await client.hideItem('Main Scene', 'Webcam');
        client.close();
    }, 3000);
}
run();
```

---

## 4. Method 2: OBS Python Scripting (`obspython`)

### A. Using the Built-in Controller GUI
1. Open OBS Studio.
2. Go to **Tools -> Scripts**.
3. Under the **Python Scripts** tab, click **(+)** and select `scripts/motionfx_controller.py`.
4. Select your **Target Scene**, **Target Source**, **Preset**, and **Duration**.
5. Click **Apply MotionFX to Scene Item**.
6. Click **Play SHOW** or **Play HIDE** to test instantly.

### B. Calling MotionFX in Custom Python Scripts
```python
import obspython as obs

def setup_item_motionfx(scene_name, item_name, preset="3d_flip_y", duration_ms=600):
    # 1. Get Scene & Item
    scene_src = obs.obs_get_source_by_name(scene_name)
    scene = obs.obs_scene_from_source(scene_src)
    item = obs.obs_scene_find_source(scene, item_name)

    # 2. Configure MotionFX Settings
    settings = obs.obs_data_create()
    obs.obs_data_set_string(settings, "preset", preset)
    obs.obs_data_set_int(settings, "duration", duration_ms)

    # 3. Create Transition Sources
    show_tr = obs.obs_source_create_private("motionfx_transition", "MotionFX_Show", settings)
    hide_tr = obs.obs_source_create_private("motionfx_transition", "MotionFX_Hide", settings)

    # 4. Attach to Scene Item
    obs.obs_sceneitem_set_show_transition(item, show_tr)
    obs.obs_sceneitem_set_show_transition_duration(item, duration_ms)
    obs.obs_sceneitem_set_hide_transition(item, hide_tr)
    obs.obs_sceneitem_set_hide_transition_duration(item, duration_ms)

    # Cleanup
    obs.obs_source_release(show_tr)
    obs.obs_source_release(hide_tr)
    obs.obs_data_release(settings)
    obs.obs_source_release(scene_src)

def trigger_animation(scene_name, item_name, show=True):
    scene_src = obs.obs_get_source_by_name(scene_name)
    scene = obs.obs_scene_from_source(scene_src)
    item = obs.obs_scene_find_source(scene, item_name)
    if item:
        obs.obs_sceneitem_set_visible(item, show)
    obs.obs_source_release(scene_src)
```

---

## 5. Method 3: OBS Lua Scripting (`obslua`)

Lua is built directly into all OBS Studio installations with zero setup.

### A. Using the Built-in Lua Controller
1. In OBS Studio, open **Tools -> Scripts**.
2. Click **(+)** and add `scripts/motionfx_controller.lua`.
3. The interactive controller UI is immediately available.

### B. Custom Lua Automation
```lua
local obs = obslua

-- Attach 3D Flip to 'Camera' item
local function apply_motionfx(scene_name, item_name, preset, duration_ms)
    local scene_src = obs.obs_get_source_by_name(scene_name)
    local scene = obs.obs_scene_from_source(scene_src)
    local item = obs.obs_scene_find_source(scene, item_name)

    local settings = obs.obs_data_create()
    obs.obs_data_set_string(settings, "preset", preset or "3d_flip_x")
    obs.obs_data_set_int(settings, "duration", duration_ms or 500)

    local tr = obs.obs_source_create_private("motionfx_transition", "MotionFX", settings)
    obs.obs_sceneitem_set_show_transition(item, tr)
    obs.obs_sceneitem_set_show_transition_duration(item, duration_ms)
    obs.obs_sceneitem_set_hide_transition(item, tr)
    obs.obs_sceneitem_set_hide_transition_duration(item, duration_ms)

    obs.obs_source_release(tr)
    obs.obs_data_release(settings)
    obs.obs_source_release(scene_src)
end

-- Toggle Item
local function toggle_item(scene_name, item_name)
    local scene_src = obs.obs_get_source_by_name(scene_name)
    local scene = obs.obs_scene_from_source(scene_src)
    local item = obs.obs_scene_find_source(scene, item_name)
    if item then
        local current = obs.obs_sceneitem_visible(item)
        obs.obs_sceneitem_set_visible(item, not current)
    end
    obs.obs_source_release(scene_src)
end
```

---

## 6. Method 4: Stream Deck & Touch Portal Integration

### Option A: Via OBS Hotkeys
1. Load `scripts/motionfx_controller.lua` into **Tools -> Scripts**.
2. Open OBS **Settings -> Hotkeys**.
3. Search for `MotionFX`:
   - `MotionFX: Show Item (Animate In)`
   - `MotionFX: Hide Item (Animate Out)`
   - `MotionFX: Toggle Item Visibility`
4. Assign hotkey combinations (e.g. `F13`, `Ctrl+Shift+1`).
5. In **Elgato Stream Deck** or **Touch Portal**, map a button to that Hotkey.

### Option B: Via Native OBS Studio Action in Stream Deck
In Stream Deck:
1. Drag **OBS Studio -> Source Visibility** onto a key.
2. Select your Scene and Source.
3. Because MotionFX is registered as the item's Show and Hide transition, pressing the Stream Deck button triggers the animation smoothly.
