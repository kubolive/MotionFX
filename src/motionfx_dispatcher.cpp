#include "motionfx/motionfx_dispatcher.h"
#include <sstream>
#include <iomanip>

namespace motionfx {

CommandDispatcher& CommandDispatcher::instance() {
    static CommandDispatcher s_instance;
    return s_instance;
}

std::shared_ptr<SceneItemAnimationState> CommandDispatcher::resolve_item(const CommandRequest& req) {
    AnimationEngine& engine = AnimationEngine::instance();

    // 1. Priority: Scene Name + Scene Item ID
    if (req.item_id > 0 && !req.scene_name.empty()) {
        auto item = engine.find(req.scene_name, req.item_id);
        if (item) return item;

        // If not yet registered, create it
        SceneItemKey key{req.scene_name, req.item_id, req.source_name};
        return engine.get_or_create(key);
    }

    // 2. Fallback: Scene Name + Source Name
    if (!req.source_name.empty() && !req.scene_name.empty()) {
        return engine.find_by_source(req.scene_name, req.source_name);
    }

    return nullptr;
}

CommandResponse CommandDispatcher::execute(const CommandRequest& req) {
    CommandResponse resp;

    // Handle queries that do not require a specific scene item
    if (req.request_type == "GetPresets") {
        resp.success = true;
        resp.data_json = get_presets_json();
        return resp;
    }

    if (req.request_type == "GetSceneItems") {
        resp.success = true;
        resp.data_json = get_scene_items_json(req.scene_name);
        return resp;
    }

    // Item-specific requests require resolving the target Scene Item
    auto item = resolve_item(req);
    if (!item) {
        resp.success = false;
        resp.error_message = "Scene item not found for scene: '" + req.scene_name +
                             "', item_id: " + std::to_string(req.item_id) +
                             ", source: '" + req.source_name + "'";
        return resp;
    }

    if (req.request_type == "GetAnimationState") {
        resp.success = true;
        resp.data_json = get_animation_state_json(req.scene_name, item->get_key().item_id);
        return resp;
    }

    if (req.request_type == "PlayShow") {
        item->trigger_show();
        resp.success = true;
        resp.data_json = "{\"status\": \"started\", \"transition\": \"show\"}";
        return resp;
    }

    if (req.request_type == "PlayHide") {
        item->trigger_hide();
        resp.success = true;
        resp.data_json = "{\"status\": \"started\", \"transition\": \"hide\"}";
        return resp;
    }

    if (req.request_type == "PlayPreset") {
        if (req.preset_name.empty()) {
            resp.success = false;
            resp.error_message = "Preset name cannot be empty for PlayPreset";
            return resp;
        }

        const auto* preset = PresetManager::instance().get_preset(req.preset_name);
        if (!preset) {
            resp.success = false;
            resp.error_message = "Preset '" + req.preset_name + "' does not exist";
            return resp;
        }

        if (preset->target_transition == TransitionType::Hide) {
            PresetManager::instance().apply_preset(item->hide_config(), req.preset_name);
            item->trigger_hide();
        } else {
            PresetManager::instance().apply_preset(item->show_config(), req.preset_name);
            item->trigger_show();
        }

        resp.success = true;
        resp.data_json = "{\"status\": \"started\", \"preset\": \"" + req.preset_name + "\"}";
        return resp;
    }

    if (req.request_type == "StopAnimation") {
        item->stop();
        resp.success = true;
        resp.data_json = "{\"status\": \"stopped\"}";
        return resp;
    }

    if (req.request_type == "PauseAnimation") {
        item->pause();
        resp.success = true;
        resp.data_json = "{\"status\": \"paused\"}";
        return resp;
    }

    if (req.request_type == "ResumeAnimation") {
        item->resume();
        resp.success = true;
        resp.data_json = "{\"status\": \"resumed\"}";
        return resp;
    }

    if (req.request_type == "ReverseAnimation") {
        item->reverse();
        resp.success = true;
        resp.data_json = "{\"status\": \"reversed\"}";
        return resp;
    }

    if (req.request_type == "SetAnimationParameter") {
        bool ok = item->set_parameter(req.parameter, req.value, req.target_transition);
        resp.success = ok;
        if (!ok) resp.error_message = "Invalid or unsupported parameter: " + req.parameter;
        else resp.data_json = "{\"parameter\": \"" + req.parameter + "\", \"value\": " + std::to_string(req.value) + "}";
        return resp;
    }

    if (req.request_type == "SetAnimationEnabled") {
        bool ok = item->set_enabled(req.parameter, req.enabled, req.target_transition);
        resp.success = ok;
        if (!ok) resp.error_message = "Invalid component: " + req.parameter;
        else resp.data_json = "{\"component\": \"" + req.parameter + "\", \"enabled\": " + (req.enabled ? "true" : "false") + "}";
        return resp;
    }

    if (req.request_type == "SetAnimationTime") {
        item->set_time(req.duration_ms, req.target_transition);
        resp.success = true;
        resp.data_json = "{\"duration_ms\": " + std::to_string(req.duration_ms) + "}";
        return resp;
    }

    if (req.request_type == "SetAnimationEasing") {
        item->set_easing(req.easing, req.target_transition);
        resp.success = true;
        resp.data_json = "{\"easing\": \"" + req.easing + "\"}";
        return resp;
    }

    resp.success = false;
    resp.error_message = "Unknown request type: " + req.request_type;
    return resp;
}

bool CommandDispatcher::play_show(const std::string& scene, int64_t item_id, const std::string& source) {
    CommandRequest req;
    req.request_type = "PlayShow";
    req.scene_name = scene;
    req.item_id = item_id;
    req.source_name = source;
    return execute(req).success;
}

bool CommandDispatcher::play_hide(const std::string& scene, int64_t item_id, const std::string& source) {
    CommandRequest req;
    req.request_type = "PlayHide";
    req.scene_name = scene;
    req.item_id = item_id;
    req.source_name = source;
    return execute(req).success;
}

bool CommandDispatcher::play_preset(const std::string& scene, int64_t item_id, const std::string& preset, const std::string& source) {
    CommandRequest req;
    req.request_type = "PlayPreset";
    req.scene_name = scene;
    req.item_id = item_id;
    req.source_name = source;
    req.preset_name = preset;
    return execute(req).success;
}

bool CommandDispatcher::stop(const std::string& scene, int64_t item_id) {
    CommandRequest req;
    req.request_type = "StopAnimation";
    req.scene_name = scene;
    req.item_id = item_id;
    return execute(req).success;
}

bool CommandDispatcher::pause(const std::string& scene, int64_t item_id) {
    CommandRequest req;
    req.request_type = "PauseAnimation";
    req.scene_name = scene;
    req.item_id = item_id;
    return execute(req).success;
}

bool CommandDispatcher::resume(const std::string& scene, int64_t item_id) {
    CommandRequest req;
    req.request_type = "ResumeAnimation";
    req.scene_name = scene;
    req.item_id = item_id;
    return execute(req).success;
}

bool CommandDispatcher::reverse(const std::string& scene, int64_t item_id) {
    CommandRequest req;
    req.request_type = "ReverseAnimation";
    req.scene_name = scene;
    req.item_id = item_id;
    return execute(req).success;
}

bool CommandDispatcher::set_parameter(const std::string& scene, int64_t item_id, const std::string& param, double val) {
    CommandRequest req;
    req.request_type = "SetAnimationParameter";
    req.scene_name = scene;
    req.item_id = item_id;
    req.parameter = param;
    req.value = val;
    return execute(req).success;
}

bool CommandDispatcher::set_enabled(const std::string& scene, int64_t item_id, const std::string& component, bool enabled) {
    CommandRequest req;
    req.request_type = "SetAnimationEnabled";
    req.scene_name = scene;
    req.item_id = item_id;
    req.parameter = component;
    req.enabled = enabled;
    return execute(req).success;
}

bool CommandDispatcher::set_time(const std::string& scene, int64_t item_id, TimeMs duration_ms) {
    CommandRequest req;
    req.request_type = "SetAnimationTime";
    req.scene_name = scene;
    req.item_id = item_id;
    req.duration_ms = duration_ms;
    return execute(req).success;
}

bool CommandDispatcher::set_easing(const std::string& scene, int64_t item_id, const std::string& easing) {
    CommandRequest req;
    req.request_type = "SetAnimationEasing";
    req.scene_name = scene;
    req.item_id = item_id;
    req.easing = easing;
    return execute(req).success;
}

std::string CommandDispatcher::get_animation_state_json(const std::string& scene, int64_t item_id) {
    auto item = AnimationEngine::instance().find(scene, item_id);
    if (!item) return "{\"error\": \"item_not_found\"}";

    Transform2D t2d = item->get_current_transform2d();
    Transform3D t3d = item->get_current_transform3d();
    EffectsState eff = item->get_current_effects();

    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"scene\": \"" << scene << "\",\n";
    ss << "  \"item_id\": " << item_id << ",\n";
    ss << "  \"status\": \"" << playback_state_to_string(item->get_playback_state()) << "\",\n";
    ss << "  \"transition\": \"" << transition_type_to_string(item->get_active_transition()) << "\",\n";
    ss << "  \"progress\": " << std::fixed << std::setprecision(4) << item->get_progress() << ",\n";
    ss << "  \"transform_2d\": {\n";
    ss << "    \"pos_x\": " << t2d.pos_x << ",\n";
    ss << "    \"pos_y\": " << t2d.pos_y << ",\n";
    ss << "    \"rot\": " << t2d.rot << ",\n";
    ss << "    \"scale_x\": " << t2d.scale_x << ",\n";
    ss << "    \"scale_y\": " << t2d.scale_y << "\n";
    ss << "  },\n";
    ss << "  \"transform_3d\": {\n";
    ss << "    \"pos_z\": " << t3d.pos_z << ",\n";
    ss << "    \"rot_x\": " << t3d.rot_x << ",\n";
    ss << "    \"rot_y\": " << t3d.rot_y << ",\n";
    ss << "    \"rot_z\": " << t3d.rot_z << ",\n";
    ss << "    \"fov\": " << t3d.fov << "\n";
    ss << "  },\n";
    ss << "  \"effects\": {\n";
    ss << "    \"opacity\": " << eff.opacity << ",\n";
    ss << "    \"blur_radius\": " << eff.blur_radius << ",\n";
    ss << "    \"mask_progress\": " << eff.mask_progress << "\n";
    ss << "  }\n";
    ss << "}";
    return ss.str();
}

std::string CommandDispatcher::get_presets_json() {
    auto presets = PresetManager::instance().get_all_presets();
    std::ostringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < presets.size(); ++i) {
        if (i > 0) ss << ",\n";
        ss << "  {\n";
        ss << "    \"name\": \"" << presets[i].name << "\",\n";
        ss << "    \"description\": \"" << presets[i].description << "\",\n";
        ss << "    \"target\": \"" << transition_type_to_string(presets[i].target_transition) << "\",\n";
        ss << "    \"duration_ms\": " << presets[i].composition.get_total_duration_ms() << ",\n";
        ss << "    \"is_builtin\": " << (presets[i].is_builtin ? "true" : "false") << "\n";
        ss << "  }";
    }
    ss << "\n]";
    return ss.str();
}

std::string CommandDispatcher::get_scene_items_json(const std::string& scene) {
    auto keys = AnimationEngine::instance().get_active_keys();
    std::ostringstream ss;
    ss << "[\n";
    bool first = true;
    for (const auto& k : keys) {
        if (!scene.empty() && k.scene_name != scene) continue;
        if (!first) ss << ",\n";
        first = false;
        ss << "  {\n";
        ss << "    \"scene\": \"" << k.scene_name << "\",\n";
        ss << "    \"item_id\": " << k.item_id << ",\n";
        ss << "    \"source\": \"" << k.source_name << "\"\n";
        ss << "  }";
    }
    ss << "\n]";
    return ss.str();
}

} // namespace motionfx
