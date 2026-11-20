#pragma once

#include "motionfx/motionfx_core.h"
#include "motionfx/motionfx_tracks.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace motionfx {

struct AnimationPreset {
    std::string name;          // Unique internal ID
    std::string display_name;  // Human-readable grouped display name for UI
    std::string description;
    TransitionType target_transition{TransitionType::Show};
    ComposableAnimation composition;
    bool is_builtin{false};
};

class PresetManager {
public:
    static PresetManager& instance();

    void register_preset(const AnimationPreset& preset);
    bool apply_preset(ComposableAnimation& target, const std::string& name) const;
    const AnimationPreset* get_preset(const std::string& name) const;
    std::vector<std::string> get_preset_names() const;
    std::vector<AnimationPreset> get_all_presets() const;

    // Custom preset management
    bool save_custom_preset(const std::string& name, const std::string& description,
                            const ComposableAnimation& comp, TransitionType target = TransitionType::Show);
    bool delete_custom_preset(const std::string& name);

    // Serialization for persistence
    std::string serialize_preset_to_json(const AnimationPreset& preset) const;
    bool deserialize_preset_from_json(const std::string& json_str, AnimationPreset& out_preset) const;
    std::string export_all_custom_presets_json() const;
    bool import_custom_presets_json(const std::string& json_str);

private:
    PresetManager();
    void init_builtin_presets();

    std::vector<std::string> preset_order_;
    std::unordered_map<std::string, AnimationPreset> presets_;
};

} // namespace motionfx
