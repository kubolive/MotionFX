"""
MotionFX Controller for OBS Studio (Python Script)
===================================================
Allows streamers and developers to automate, control, and attach
MotionFX 3D & animation transitions to any Scene Item directly inside OBS.

Usage:
    1. Open OBS Studio -> Tools -> Scripts.
    2. Click (+) and select this file (motionfx_controller.py).
    3. Select your Scene, Source, Preset, and Duration.
    4. Click 'Apply MotionFX' or use the Hotkeys in OBS Settings -> Hotkeys!

Compatible with: OBS Studio 28+ / 29+ / 30+ / 31+ (Python 3.9 - 3.12)
Author: KuboLive (MotionFX Team)
"""

import obspython as obs

# Current GUI state
current_scene_name = ""
current_item_name = ""
current_preset = "3d_flip_x"
current_duration_ms = 500
current_target_mode = "both"  # "show", "hide", "both"

hotkey_toggle_id = obs.OBS_INVALID_HOTKEY_ID
hotkey_show_id = obs.OBS_INVALID_HOTKEY_ID
hotkey_hide_id = obs.OBS_INVALID_HOTKEY_ID

PRESETS = [
    ("3d_flip_x", "3D Flip X (Vertical Flip)"),
    ("3d_flip_y", "3D Flip Y (Horizontal Card Flip)"),
    ("3d_flip_diagonal", "3D Flip Diagonal"),
    ("perspective_tilt", "Perspective Tilt"),
    ("zoom_spin", "Zoom Spin (Twist & Scale)"),
    ("blur_reveal", "Blur Focus (Gaussian Fade)"),
    ("elastic_pop", "Elastic Pop (Spring Bounce)"),
    ("glitch_wipe", "Glitch Displace"),
    ("mask_reveal", "Diagonal Mask Wipe (45°)"),
    ("mask_radial", "Radial Iris Wipe"),
    ("mask_box", "Box Rectangle Wipe"),
    ("text_reveal", "Text Typewriter"),
    ("character_stagger", "Text Character Stagger"),
    ("smooth_slide_left", "Smooth Slide Left"),
    ("smooth_slide_right", "Smooth Slide Right"),
    ("smooth_slide_up", "Smooth Slide Up"),
    ("smooth_slide_down", "Smooth Slide Down"),
    ("cinematic_fly_through", "Cinematic 3D Fly-Through"),
]

def get_current_scene_item():
    """Helper: Retrieve scene source and scene item pointer."""
    if not current_scene_name or not current_item_name:
        return None, None

    scene_source = obs.obs_get_source_by_name(current_scene_name)
    if not scene_source:
        return None, None

    scene = obs.obs_scene_from_source(scene_source)
    if not scene:
        obs.obs_source_release(scene_source)
        return None, None

    item = obs.obs_scene_find_source(scene, current_item_name)
    return scene_source, item


# ============================================================================
# CORE API FUNCTIONS
# ============================================================================

def create_motionfx_transition(preset="3d_flip_x", duration_ms=500):
    """Create a private MotionFX transition source instance with configured settings."""
    settings = obs.obs_data_create()
    obs.obs_data_set_string(settings, "preset", preset)
    obs.obs_data_set_int(settings, "duration", duration_ms)

    tr_source = obs.obs_source_create_private("motionfx_transition", "MotionFX_Instance", settings)
    obs.obs_data_release(settings)
    return tr_source


def apply_motionfx_to_item(scene_name, item_name, preset="3d_flip_x", duration_ms=500, target="both"):
    """
    Attach MotionFX transition(s) to a Scene Item.
    target: 'show', 'hide', or 'both'
    """
    scene_source = obs.obs_get_source_by_name(scene_name)
    if not scene_source:
        print(f"[MotionFX] Scene '{scene_name}' not found.")
        return False

    scene = obs.obs_scene_from_source(scene_source)
    if not scene:
        obs.obs_source_release(scene_source)
        return False

    item = obs.obs_scene_find_source(scene, item_name)
    if not item:
        print(f"[MotionFX] Item '{item_name}' not found in scene '{scene_name}'.")
        obs.obs_source_release(scene_source)
        return False

    # Apply Show Transition
    if target in ("show", "both"):
        show_tr = create_motionfx_transition(preset, duration_ms)
        if show_tr:
            obs.obs_sceneitem_set_show_transition(item, show_tr)
            obs.obs_sceneitem_set_show_transition_duration(item, duration_ms)
            obs.obs_source_release(show_tr)
            print(f"[MotionFX] Attached Show Transition '{preset}' ({duration_ms} ms) to '{item_name}'")

    # Apply Hide Transition
    if target in ("hide", "both"):
        hide_tr = create_motionfx_transition(preset, duration_ms)
        if hide_tr:
            obs.obs_sceneitem_set_hide_transition(item, hide_tr)
            obs.obs_sceneitem_set_hide_transition_duration(item, duration_ms)
            obs.obs_source_release(hide_tr)
            print(f"[MotionFX] Attached Hide Transition '{preset}' ({duration_ms} ms) to '{item_name}'")

    obs.obs_source_release(scene_source)
    return True


def trigger_show():
    """Trigger Show Animation on configured scene item."""
    scene_source, item = get_current_scene_item()
    if item:
        obs.obs_sceneitem_set_visible(item, True)
        print(f"[MotionFX] Triggered SHOW on '{current_item_name}'")
    if scene_source:
        obs.obs_source_release(scene_source)


def trigger_hide():
    """Trigger Hide Animation on configured scene item."""
    scene_source, item = get_current_scene_item()
    if item:
        obs.obs_sceneitem_set_visible(item, False)
        print(f"[MotionFX] Triggered HIDE on '{current_item_name}'")
    if scene_source:
        obs.obs_source_release(scene_source)


def trigger_toggle():
    """Toggle Visibility on configured scene item."""
    scene_source, item = get_current_scene_item()
    if item:
        is_vis = obs.obs_sceneitem_visible(item)
        obs.obs_sceneitem_set_visible(item, not is_vis)
        print(f"[MotionFX] Toggled visibility of '{current_item_name}' to {not is_vis}")
    if scene_source:
        obs.obs_source_release(scene_source)


# ============================================================================
# OBS SCRIPT INTERFACE & GUI
# ============================================================================

def script_description():
    return """<h2>🎬 MotionFX Controller API (Python)</h2>
<p>Easily bind, automate, and trigger <b>MotionFX 3D & Motion Transitions</b> on any Scene Item.</p>
<ul>
    <li><b>Attach MotionFX:</b> Automatically registers Show/Hide transitions on selected item.</li>
    <li><b>Instant Triggers:</b> Test animations directly or bind to OBS Hotkeys / Stream Deck.</li>
    <li><b>API & Scripting:</b> Clean Python functions for custom streaming bots and workflows.</li>
</ul>"""


def script_properties():
    props = obs.obs_properties_create()

    # 1. Scene Dropdown
    p_scene = obs.obs_properties_add_list(
        props, "scene", "Target Scene", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING
    )
    scenes = obs.obs_frontend_get_scene_names()
    if scenes:
        for name in scenes:
            obs.obs_property_list_add_string(p_scene, name, name)

    # 2. Scene Item Dropdown
    p_item = obs.obs_properties_add_list(
        props, "item", "Target Source / Item", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING
    )
    if current_scene_name:
        scene_src = obs.obs_get_source_by_name(current_scene_name)
        if scene_src:
            scene = obs.obs_scene_from_source(scene_src)
            if scene:
                items = obs.obs_scene_enum_items(scene)
                if items:
                    for sc_item in items:
                        src = obs.obs_sceneitem_get_source(sc_item)
                        if src:
                            sname = obs.obs_source_get_name(src)
                            obs.obs_property_list_add_string(p_item, sname, sname)
                    obs.sceneitem_list_release(items)
            obs.obs_source_release(scene_src)

    # 3. Preset Dropdown
    p_preset = obs.obs_properties_add_list(
        props, "preset", "MotionFX Preset", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING
    )
    for pid, pname in PRESETS:
        obs.obs_property_list_add_string(p_preset, pname, pid)

    # 4. Duration Slider
    obs.obs_properties_add_int(props, "duration", "Duration (ms)", 50, 5000, 50)

    # 5. Target Mode (Show / Hide / Both)
    p_target = obs.obs_properties_add_list(
        props, "target_mode", "Apply To", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING
    )
    obs.obs_property_list_add_string(p_target, "Both (Show & Hide)", "both")
    obs.obs_property_list_add_string(p_target, "Show Transition Only", "show")
    obs.obs_property_list_add_string(p_target, "Hide Transition Only", "hide")

    # 6. Action Buttons
    obs.obs_properties_add_button(
        props, "btn_apply", "✨ Apply MotionFX to Scene Item",
        lambda *args: (apply_motionfx_to_item(current_scene_name, current_item_name, current_preset, current_duration_ms, current_target_mode), True)[1]
    )

    obs.obs_properties_add_button(
        props, "btn_show", "▶️ Play SHOW Animation",
        lambda *args: (trigger_show(), True)[1]
    )

    obs.obs_properties_add_button(
        props, "btn_hide", "◀️ Play HIDE Animation",
        lambda *args: (trigger_hide(), True)[1]
    )

    obs.obs_properties_add_button(
        props, "btn_toggle", "🔄 Toggle Visibility",
        lambda *args: (trigger_toggle(), True)[1]
    )

    return props


def script_update(settings):
    global current_scene_name, current_item_name, current_preset, current_duration_ms, current_target_mode
    current_scene_name = obs.obs_data_get_string(settings, "scene")
    current_item_name = obs.obs_data_get_string(settings, "item")
    current_preset = obs.obs_data_get_string(settings, "preset") or "3d_flip_x"
    current_duration_ms = obs.obs_data_get_int(settings, "duration")
    if current_duration_ms <= 0:
        current_duration_ms = 500
    current_target_mode = obs.obs_data_get_string(settings, "target_mode") or "both"


def script_load(settings):
    global hotkey_toggle_id, hotkey_show_id, hotkey_hide_id

    hotkey_toggle_id = obs.obs_hotkey_register_frontend(
        "py_motionfx_toggle", "MotionFX (Python): Toggle Item Visibility",
        lambda pressed: trigger_toggle() if pressed else None
    )
    hotkey_show_id = obs.obs_hotkey_register_frontend(
        "py_motionfx_show", "MotionFX (Python): Show Item (Animate In)",
        lambda pressed: trigger_show() if pressed else None
    )
    hotkey_hide_id = obs.obs_hotkey_register_frontend(
        "py_motionfx_hide", "MotionFX (Python): Hide Item (Animate Out)",
        lambda pressed: trigger_hide() if pressed else None
    )

    hotkey_save_array = obs.obs_data_get_array(settings, "motionfx_py_hotkeys")
    obs.obs_hotkey_load(hotkey_toggle_id, hotkey_save_array)
    obs.obs_data_array_release(hotkey_save_array)


def script_save(settings):
    hotkey_save_array = obs.obs_hotkey_save(hotkey_toggle_id)
    obs.obs_data_set_array(settings, "motionfx_py_hotkeys", hotkey_save_array)
    obs.obs_data_array_release(hotkey_save_array)
