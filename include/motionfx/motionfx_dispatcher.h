#pragma once

#include "motionfx/motionfx_core.h"
#include "motionfx/motionfx_engine.h"
#include "motionfx/motionfx_presets.h"
#include <string>
#include <memory>

namespace motionfx {

struct CommandRequest {
    std::string request_type; // e.g., "PlayShow", "PlayHide", "GetAnimationState", etc.
    std::string scene_name;
    int64_t item_id{0};
    std::string source_name;
    std::string preset_name;
    std::string parameter;
    double value{0.0};
    bool enabled{false};
    std::string easing;
    TimeMs duration_ms{0.0};
    TransitionType target_transition{TransitionType::None};
};

struct CommandResponse {
    bool success{false};
    std::string error_message;
    std::string data_json;
};

class CommandDispatcher {
public:
    static CommandDispatcher& instance();

    CommandResponse execute(const CommandRequest& request);

    // High level helper methods for C / Lua bindings
    bool play_show(const std::string& scene, int64_t item_id, const std::string& source = "");
    bool play_hide(const std::string& scene, int64_t item_id, const std::string& source = "");
    bool play_preset(const std::string& scene, int64_t item_id, const std::string& preset, const std::string& source = "");
    bool stop(const std::string& scene, int64_t item_id);
    bool pause(const std::string& scene, int64_t item_id);
    bool resume(const std::string& scene, int64_t item_id);
    bool reverse(const std::string& scene, int64_t item_id);
    bool set_parameter(const std::string& scene, int64_t item_id, const std::string& param, double val);
    bool set_enabled(const std::string& scene, int64_t item_id, const std::string& component, bool enabled);
    bool set_time(const std::string& scene, int64_t item_id, TimeMs duration_ms);
    bool set_easing(const std::string& scene, int64_t item_id, const std::string& easing);

    std::string get_animation_state_json(const std::string& scene, int64_t item_id);
    std::string get_presets_json();
    std::string get_scene_items_json(const std::string& scene);

private:
    CommandDispatcher() = default;
    std::shared_ptr<SceneItemAnimationState> resolve_item(const CommandRequest& req);
};

} // namespace motionfx
