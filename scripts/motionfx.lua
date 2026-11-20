-- MotionFX Lua Scripting Module for OBS Studio
-- Conforms to Section 15.C of MotionFX Specification

local obs = obslua

local motionfx = {}

-- C-module or WebSocket bridge dispatcher
local function execute_command(req_type, scene_name, item_id, extra)
    extra = extra or {}
    extra.request_type = req_type
    extra.scene_name = scene_name or ""
    extra.item_id = item_id or 0

    -- When running inside OBS, calls MotionFX C dispatcher directly or via libobs signals
    if _G.motionfx_native_dispatch then
        return _G.motionfx_native_dispatch(extra)
    else
        -- Fallback: execute via OBS Scene API
        local scene_source = obs.obs_get_source_by_name(scene_name)
        if scene_source then
            local scene = obs.obs_scene_from_source(scene_source)
            if scene then
                local item = obs.obs_scene_find_sceneitem_id(scene, item_id)
                if item then
                    if req_type == "PlayShow" then
                        obs.obs_sceneitem_set_visible(item, true)
                    elseif req_type == "PlayHide" then
                        obs.obs_sceneitem_set_visible(item, false)
                    end
                end
            end
            obs.obs_source_release(scene_source)
        end
        return true
    end
end

function motionfx.play_show(scene_name, item_id)
    return execute_command("PlayShow", scene_name, item_id)
end

function motionfx.play_hide(scene_name, item_id)
    return execute_command("PlayHide", scene_name, item_id)
end

function motionfx.play_preset(scene_name, item_id, preset_name)
    return execute_command("PlayPreset", scene_name, item_id, { preset_name = preset_name })
end

function motionfx.stop(scene_name, item_id)
    return execute_command("StopAnimation", scene_name, item_id)
end

function motionfx.pause(scene_name, item_id)
    return execute_command("PauseAnimation", scene_name, item_id)
end

function motionfx.resume(scene_name, item_id)
    return execute_command("ResumeAnimation", scene_name, item_id)
end

function motionfx.reverse(scene_name, item_id)
    return execute_command("ReverseAnimation", scene_name, item_id)
end

function motionfx.set_parameter(scene_name, item_id, parameter, value)
    return execute_command("SetAnimationParameter", scene_name, item_id, {
        parameter = parameter,
        value = value
    })
end

function motionfx.set_enabled(scene_name, item_id, component, enabled)
    return execute_command("SetAnimationEnabled", scene_name, item_id, {
        parameter = component,
        enabled = enabled
    })
end

function motionfx.set_time(scene_name, item_id, duration_ms)
    return execute_command("SetAnimationTime", scene_name, item_id, {
        duration_ms = duration_ms
    })
end

function motionfx.set_easing(scene_name, item_id, easing)
    return execute_command("SetAnimationEasing", scene_name, item_id, {
        easing = easing
    })
end

-- Export module globally and as return table
_G.motionfx = motionfx
return motionfx
