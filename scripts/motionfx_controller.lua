--[[
    MotionFX Controller for OBS Studio (Lua Script)
    ================================================
    Allows streamers and developers to automate, control, and attach
    MotionFX 3D & animation transitions to any Scene Item directly inside OBS.
    
    Compatible with: OBS Studio 28+ / 29+ / 30+ / 31+
    Author: KuboLive (MotionFX Team)
]]--

local obs = obslua

local current_scene_name = ""
local current_item_name = ""
local current_preset = "3d_flip_x"
local current_duration_ms = 500
local current_target_mode = "both" -- "show", "hide", "both"

local hotkey_toggle_id = obs.OBS_INVALID_HOTKEY_ID
local hotkey_show_id = obs.OBS_INVALID_HOTKEY_ID
local hotkey_hide_id = obs.OBS_INVALID_HOTKEY_ID

local presets = {
    { id = "3d_flip_x", name = "3D Flip X (Vertical Flip)" },
    { id = "3d_flip_y", name = "3D Flip Y (Horizontal Card Flip)" },
    { id = "3d_flip_diagonal", name = "3D Flip Diagonal" },
    { id = "perspective_tilt", name = "Perspective Tilt" },
    { id = "zoom_spin", name = "Zoom Spin (Twist & Scale)" },
    { id = "blur_reveal", name = "Blur Focus (Gaussian Fade)" },
    { id = "elastic_pop", name = "Elastic Pop (Spring Bounce)" },
    { id = "glitch_wipe", name = "Glitch Displace" },
    { id = "mask_reveal", name = "Diagonal Mask Wipe (45°)" },
    { id = "mask_radial", name = "Radial Iris Wipe" },
    { id = "mask_box", name = "Box Rectangle Wipe" },
    { id = "text_reveal", name = "Text Typewriter" },
    { id = "character_stagger", name = "Text Character Stagger" },
    { id = "smooth_slide_left", name = "Smooth Slide Left" },
    { id = "smooth_slide_right", name = "Smooth Slide Right" },
    { id = "smooth_slide_up", name = "Smooth Slide Up" },
    { id = "smooth_slide_down", name = "Smooth Slide Down" },
    { id = "cinematic_fly_through", name = "Cinematic 3D Fly-Through" }
}

-- Helper: Get current scene item pointer
local function get_current_scene_item()
    if current_scene_name == "" or current_item_name == "" then
        return nil, nil
    end

    local scene_source = obs.obs_get_source_by_name(current_scene_name)
    if not scene_source then
        return nil, nil
    end

    local scene = obs.obs_scene_from_source(scene_source)
    if not scene then
        obs.obs_source_release(scene_source)
        return nil, nil
    end

    local item = obs.obs_scene_find_source(scene, current_item_name)
    return scene_source, item
end

-- ============================================================================
-- CORE API FUNCTIONS
-- ============================================================================

--- Creates or configures a MotionFX transition source
-- @param preset string: Name of the preset (e.g. "3d_flip_x")
-- @param duration_ms integer: Duration in milliseconds (default: 500)
-- @return obs_source_t pointer to the transition source
function create_motionfx_transition(preset, duration_ms)
    local settings = obs.obs_data_create()
    obs.obs_data_set_string(settings, "preset", preset or "3d_flip_x")
    obs.obs_data_set_int(settings, "duration", duration_ms or 500)

    local tr_source = obs.obs_source_create_private("motionfx_transition", "MotionFX_Instance", settings)
    obs.obs_data_release(settings)
    return tr_source
end

--- Attaches MotionFX transitions to a Scene Item
-- @param scene_name string
-- @param item_name string
-- @param preset string
-- @param duration_ms integer
-- @param target string: "show", "hide", or "both"
function apply_motionfx_to_item(scene_name, item_name, preset, duration_ms, target)
    target = target or "both"
    preset = preset or "3d_flip_x"
    duration_ms = duration_ms or 500

    local scene_source = obs.obs_get_source_by_name(scene_name)
    if not scene_source then
        print(string.format("[MotionFX] Scene '%s' not found.", scene_name))
        return false
    end

    local scene = obs.obs_scene_from_source(scene_source)
    if not scene then
        obs.obs_source_release(scene_source)
        return false
    end

    local item = obs.obs_scene_find_source(scene, item_name)
    if not item then
        print(string.format("[MotionFX] Item '%s' not found in scene '%s'.", item_name, scene_name))
        obs.obs_source_release(scene_source)
        return false
    end

    -- Apply Show Transition
    if target == "show" or target == "both" then
        local show_tr = create_motionfx_transition(preset, duration_ms)
        if show_tr then
            obs.obs_sceneitem_set_show_transition(item, show_tr)
            obs.obs_sceneitem_set_show_transition_duration(item, duration_ms)
            obs.obs_source_release(show_tr)
            print(string.format("[MotionFX] Attached Show Transition '%s' (%d ms) to '%s'", preset, duration_ms, item_name))
        end
    end

    -- Apply Hide Transition
    if target == "hide" or target == "both" then
        local hide_tr = create_motionfx_transition(preset, duration_ms)
        if hide_tr then
            obs.obs_sceneitem_set_hide_transition(item, hide_tr)
            obs.obs_sceneitem_set_hide_transition_duration(item, duration_ms)
            obs.obs_source_release(hide_tr)
            print(string.format("[MotionFX] Attached Hide Transition '%s' (%d ms) to '%s'", preset, duration_ms, item_name))
        end
    end

    obs.obs_source_release(scene_source)
    return true
end

--- Triggers Show Animation on the configured item
function trigger_show()
    local scene_source, item = get_current_scene_item()
    if item then
        obs.obs_sceneitem_set_visible(item, true)
        print(string.format("[MotionFX] Triggered SHOW on '%s'", current_item_name))
    end
    if scene_source then
        obs.obs_source_release(scene_source)
    end
end

--- Triggers Hide Animation on the configured item
function trigger_hide()
    local scene_source, item = get_current_scene_item()
    if item then
        obs.obs_sceneitem_set_visible(item, false)
        print(string.format("[MotionFX] Triggered HIDE on '%s'", current_item_name))
    end
    if scene_source then
        obs.obs_source_release(scene_source)
    end
end

--- Toggles Visibility of the configured item
function trigger_toggle()
    local scene_source, item = get_current_scene_item()
    if item then
        local is_vis = obs.obs_sceneitem_visible(item)
        obs.obs_sceneitem_set_visible(item, not is_vis)
        print(string.format("[MotionFX] Toggled visibility of '%s' to %s", current_item_name, tostring(not is_vis)))
    end
    if scene_source then
        obs.obs_source_release(scene_source)
    end
end

-- ============================================================================
-- OBS SCRIPT INTERFACE & GUI
-- ============================================================================

function script_description()
    return [[<h2>🎬 MotionFX Controller API</h2>
<p>Easily bind, automate, and trigger <b>MotionFX 3D & Motion Transitions</b> on any Scene Item.</p>
<ul>
    <li><b>Attach MotionFX:</b> Automatically registers Show/Hide transitions on selected item.</li>
    <li><b>Instant Triggers:</b> Test animations directly or bind to OBS Hotkeys / Stream Deck.</li>
    <li><b>API & Scripting:</b> Exposes global functions for custom Lua automation.</li>
</ul>]]
end

function script_properties()
    local props = obs.obs_properties_create()

    -- 1. Scene Dropdown
    local p_scene = obs.obs_properties_add_list(props, "scene", "Target Scene", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING)
    local scenes = obs.obs_frontend_get_scene_names()
    if scenes then
        for _, name in ipairs(scenes) do
            obs.obs_property_list_add_string(p_scene, name, name)
        end
    end

    -- 2. Scene Item Dropdown
    local p_item = obs.obs_properties_add_list(props, "item", "Target Source / Item", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING)
    if current_scene_name ~= "" then
        local scene_src = obs.obs_get_source_by_name(current_scene_name)
        if scene_src then
            local scene = obs.obs_scene_from_source(scene_src)
            if scene then
                local items = obs.obs_scene_enum_items(scene)
                if items then
                    for _, sc_item in ipairs(items) do
                        local src = obs.obs_sceneitem_get_source(sc_item)
                        if src then
                            local sname = obs.obs_source_get_name(src)
                            obs.obs_property_list_add_string(p_item, sname, sname)
                        end
                    end
                    obs.sceneitem_list_release(items)
                end
            end
            obs.obs_source_release(scene_src)
        end
    end

    -- 3. Preset Dropdown
    local p_preset = obs.obs_properties_add_list(props, "preset", "MotionFX Preset", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING)
    for _, pr in ipairs(presets) do
        obs.obs_property_list_add_string(p_preset, pr.name, pr.id)
    end

    -- 4. Duration Slider
    obs.obs_properties_add_int(props, "duration", "Duration (ms)", 50, 5000, 50)

    -- 5. Target Mode (Show / Hide / Both)
    local p_target = obs.obs_properties_add_list(props, "target_mode", "Apply To", obs.OBS_COMBO_TYPE_LIST, obs.OBS_COMBO_FORMAT_STRING)
    obs.obs_property_list_add_string(p_target, "Both (Show & Hide)", "both")
    obs.obs_property_list_add_string(p_target, "Show Transition Only", "show")
    obs.obs_property_list_add_string(p_target, "Hide Transition Only", "hide")

    -- 6. Action Buttons
    obs.obs_properties_add_button(props, "btn_apply", "✨ Apply MotionFX to Scene Item", function()
        apply_motionfx_to_item(current_scene_name, current_item_name, current_preset, current_duration_ms, current_target_mode)
        return true
    end)

    obs.obs_properties_add_button(props, "btn_show", "▶️ Play SHOW Animation", function()
        trigger_show()
        return true
    end)

    obs.obs_properties_add_button(props, "btn_hide", "◀️ Play HIDE Animation", function()
        trigger_hide()
        return true
    end)

    obs.obs_properties_add_button(props, "btn_toggle", "🔄 Toggle Visibility", function()
        trigger_toggle()
        return true
    end)

    return props
end

function script_update(settings)
    current_scene_name = obs.obs_data_get_string(settings, "scene")
    current_item_name = obs.obs_data_get_string(settings, "item")
    current_preset = obs.obs_data_get_string(settings, "preset")
    current_duration_ms = obs.obs_data_get_int(settings, "duration")
    if current_duration_ms <= 0 then current_duration_ms = 500 end
    current_target_mode = obs.obs_data_get_string(settings, "target_mode")
    if current_target_mode == "" then current_target_mode = "both" end
end

function script_load(settings)
    -- Register global hotkeys
    hotkey_toggle_id = obs.obs_hotkey_register_frontend("motionfx_toggle_hotkey", "MotionFX: Toggle Item Visibility", function(pressed)
        if pressed then trigger_toggle() end
    end)

    hotkey_show_id = obs.obs_hotkey_register_frontend("motionfx_show_hotkey", "MotionFX: Show Item (Animate In)", function(pressed)
        if pressed then trigger_show() end
    end)

    hotkey_hide_id = obs.obs_hotkey_register_frontend("motionfx_hide_hotkey", "MotionFX: Hide Item (Animate Out)", function(pressed)
        if pressed then trigger_hide() end
    end)

    local hotkey_save_array = obs.obs_data_get_array(settings, "motionfx_hotkeys")
    obs.obs_hotkey_load(hotkey_toggle_id, hotkey_save_array)
    obs.obs_data_array_release(hotkey_save_array)
end

function script_save(settings)
    local hotkey_save_array = obs.obs_hotkey_save(hotkey_toggle_id)
    obs.obs_data_set_array(settings, "motionfx_hotkeys", hotkey_save_array)
    obs.obs_data_array_release(hotkey_save_array)
end
