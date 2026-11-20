#include "motionfx/motionfx_presets.h"
#include <sstream>
#include <iomanip>

namespace motionfx {

PresetManager& PresetManager::instance() {
    static PresetManager s_instance;
    return s_instance;
}

PresetManager::PresetManager() {
    init_builtin_presets();
}

void PresetManager::init_builtin_presets() {
    // =========================================================================
    // 1. SLIDE ANIMATIONS
    // (Show: animates into view | Hide: automatically animates reverse out of view)
    // =========================================================================
    {
        AnimationPreset p;
        p.name = "slide_left";
        p.display_name = "Slide - From Left";
        p.description = "Slide animation along X axis from left edge (-500px, 500ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(-500.0, 0.0, 500.0, EasingConfig::ease_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "slide_right";
        p.display_name = "Slide - From Right";
        p.description = "Slide animation along X axis from right edge (+500px, 500ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(500.0, 0.0, 500.0, EasingConfig::ease_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "slide_top";
        p.display_name = "Slide - From Top";
        p.description = "Slide animation along Y axis from top edge (-400px, 500ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(0.0, -400.0, 500.0, EasingConfig::ease_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "slide_bottom";
        p.display_name = "Slide - From Bottom";
        p.description = "Slide animation along Y axis from bottom edge (+400px, 500ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(0.0, 400.0, 500.0, EasingConfig::ease_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "slide_right_soft";
        p.display_name = "Slide - From Right Soft (300px)";
        p.description = "Gentle slide 300px with cubic deceleration + fade (1000ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(300.0, 0.0, 1000.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 800.0, EasingConfig::linear());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "slide_bottom_soft";
        p.display_name = "Slide - From Bottom Soft (120px)";
        p.description = "Gentle slide 120px with cubic deceleration + fade (800ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(0.0, 120.0, 800.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 500.0, EasingConfig::linear());
        register_preset(p);
    }

    // =========================================================================
    // 2. SLIDE & FADE COMBINATIONS
    // =========================================================================
    {
        AnimationPreset p;
        p.name = "slide_fade_left";
        p.display_name = "Slide & Fade - From Left";
        p.description = "Slide from left combined with smooth opacity fade (500ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(-400.0, 0.0, 500.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 400.0, EasingConfig::linear());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "slide_fade_right";
        p.display_name = "Slide & Fade - From Right";
        p.description = "Slide from right combined with smooth opacity fade (500ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(400.0, 0.0, 500.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 400.0, EasingConfig::linear());
        register_preset(p);
    }

    // =========================================================================
    // 3. ZOOM & SCALE
    // =========================================================================
    {
        AnimationPreset p;
        p.name = "zoom_pop";
        p.display_name = "Zoom - Pop Overshoot";
        p.description = "Scale zoom with springy overshoot bounce (450ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_scale(0.2, 1.0, 450.0, EasingConfig::back_out(1.5));
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "zoom_fade";
        p.display_name = "Zoom - Smooth Fade";
        p.description = "Gentle zoom from 70% to 100% combined with smooth fade (400ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_scale(0.7, 1.0, 400.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 350.0, EasingConfig::linear());
        register_preset(p);
    }

    // =========================================================================
    // 4. 3D TRANSFORMS & FLIPS
    // =========================================================================
    {
        AnimationPreset p;
        p.name = "3d_fold_top";
        p.display_name = "3D Flip - Down from Top (Flip X)";
        p.description = "3D fold rotation 90 deg from top with subtle settling bounce (800ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_3d_flip(90.0, 0.0, 0.0, 0.0, 800.0, EasingConfig::back_out(1.2));
        p.composition.configure_slide(0.0, -40.0, 800.0, EasingConfig::back_out(1.2));
        p.composition.configure_fade(0.0, 1.0, 400.0, EasingConfig::linear());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "3d_flip_x";
        p.display_name = "3D Flip - Vertical Card (Flip X)";
        p.description = "Vertical 3D flip card rotating around X axis 90 to 0 deg (500ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_3d_flip(90.0, 0.0, 0.0, 0.0, 500.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 300.0, EasingConfig::linear());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "3d_flip_y";
        p.display_name = "3D Flip - Horizontal Card (Flip Y)";
        p.description = "Horizontal 3D flip card opening like a door around Y axis 90 to 0 deg (550ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_3d_flip(0.0, 90.0, 0.0, 0.0, 550.0, EasingConfig::back_out(1.4));
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "card_flip";
        p.display_name = "3D Card Flip - 180 Deg Full";
        p.description = "Full 180-degree card flip with 3D perspective foreshortening (650ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_3d_flip(0.0, 180.0, 0.0, 0.0, 650.0, EasingConfig::ease_in_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "3d_flip_slide";
        p.display_name = "3D Flip & Slide";
        p.description = "Simultaneous 3D rotation and slide from left edge (600ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_3d_flip(0.0, 75.0, 0.0, 0.0, 600.0, EasingConfig::ease_out());
        p.composition.configure_slide(-300.0, 0.0, 600.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 400.0, EasingConfig::linear());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "swing";
        p.display_name = "3D Swing - Pendulum";
        p.description = "Pendulum swing rotation with damped spring oscillation (800ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_rotation(35.0, 0.0, 800.0, EasingConfig::elastic_out(1.0, 0.35));
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "rotation";
        p.display_name = "Rotation - Spin & Scale";
        p.description = "Spin 2D rotation into position with bounce settling (600ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_rotation(-90.0, 0.0, 600.0, EasingConfig::back_out(1.5));
        p.composition.configure_scale(0.4, 1.0, 600.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 300.0, EasingConfig::linear());
        register_preset(p);
    }

    // =========================================================================
    // 5. PHYSICS & DYNAMICS
    // =========================================================================
    {
        AnimationPreset p;
        p.name = "bounce";
        p.display_name = "Bounce - Physics Drop";
        p.description = "Drop from above with multi-stage physics ball bounce (750ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_slide(0.0, -400.0, 750.0, EasingConfig::bounce_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "elastic";
        p.display_name = "Elastic - Spring Pop";
        p.description = "Pop in with rubber band spring elastic overshoot (700ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_scale(0.2, 1.0, 700.0, EasingConfig::elastic_out(1.2, 0.3));
        register_preset(p);
    }

    // =========================================================================
    // 6. REVEAL & TEXT EFFECTS
    // =========================================================================
    {
        AnimationPreset p;
        p.name = "text_reveal";
        p.display_name = "Text Reveal - Typewriter";
        p.description = "Typewriter letter-by-letter reveal for text sources (1000ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_text(1, 1000.0, EasingConfig::linear());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "character_stagger";
        p.display_name = "Text Reveal - Character Stagger";
        p.description = "Cascading letter stagger reveal sequential drop (1200ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_text(2, 1200.0, EasingConfig::ease_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "blur_reveal";
        p.display_name = "Blur Reveal - Focus";
        p.description = "Depth of field camera blur dissipation with smooth fade (600ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_blur(12.0f, 0.0f, 600.0, EasingConfig::ease_out());
        p.composition.configure_fade(0.0, 1.0, 450.0, EasingConfig::linear());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "mask_reveal";
        p.display_name = "Mask Reveal - Diagonal";
        p.description = "Linear 45-degree diagonal mask wipe reveal (600ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_mask(0.0, 1.0, 1, 45.0f, 600.0, EasingConfig::ease_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "mask_radial";
        p.display_name = "Mask Reveal - Radial Circle";
        p.description = "Expanding circular iris mask reveal (600ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_mask(0.0, 1.0, 2, 0.0f, 600.0, EasingConfig::ease_out());
        register_preset(p);
    }
    {
        AnimationPreset p;
        p.name = "mask_box";
        p.display_name = "Mask Reveal - Box";
        p.description = "Expanding rectangular box mask reveal (600ms)";
        p.target_transition = TransitionType::Show;
        p.is_builtin = true;
        p.composition.configure_mask(0.0, 1.0, 3, 0.0f, 600.0, EasingConfig::ease_out());
        register_preset(p);
    }

    // Backward-compatibility aliases for previously saved names
    auto alias = [&](const std::string& old_id, const std::string& target_id) {
        auto it = presets_.find(target_id);
        if (it != presets_.end() && presets_.find(old_id) == presets_.end()) {
            AnimationPreset cp = it->second;
            cp.name = old_id;
            presets_[old_id] = cp;
        }
    };
    alias("slide_in_left", "slide_left");
    alias("slide_out_left", "slide_left");
    alias("slide_in_right", "slide_right");
    alias("slide_out_right", "slide_right");
    alias("slide_in_top", "slide_top");
    alias("slide_out_top", "slide_top");
    alias("slide_in_bottom", "slide_bottom");
    alias("slide_out_bottom", "slide_bottom");
    alias("slide_fade_in", "slide_fade_left");
    alias("slide_fade_out", "slide_fade_right");
    alias("zoom_in", "zoom_pop");
    alias("zoom_out", "zoom_pop");
    alias("zoom_fade_in", "zoom_fade");
    alias("zoom_fade_out", "zoom_fade");
    alias("3d_fold_in_down", "3d_fold_top");
    alias("3d_fold_out_up", "3d_fold_top");
    alias("3d_roll_change", "3d_fold_top");
    alias("cctt7_bar_in", "slide_right_soft");
    alias("cctt7_bar_out", "slide_right_soft");
    alias("cctt7_logo_in", "slide_bottom_soft");
    alias("cctt7_logo_out", "slide_bottom_soft");
    alias("cctt7_text_in", "3d_fold_top");
    alias("cctt7_text_out", "3d_fold_top");
    alias("cctt7_text_flip", "3d_fold_top");
    alias("rotation_in", "rotation");
}

void PresetManager::register_preset(const AnimationPreset& preset) {
    if (presets_.find(preset.name) == presets_.end()) {
        preset_order_.push_back(preset.name);
    }
    presets_[preset.name] = preset;
}

bool PresetManager::apply_preset(ComposableAnimation& target, const std::string& name) const {
    auto it = presets_.find(name);
    if (it == presets_.end()) return false;
    target = it->second.composition;
    return true;
}

const AnimationPreset* PresetManager::get_preset(const std::string& name) const {
    auto it = presets_.find(name);
    return (it != presets_.end()) ? &it->second : nullptr;
}

std::vector<std::string> PresetManager::get_preset_names() const {
    return preset_order_;
}

std::vector<AnimationPreset> PresetManager::get_all_presets() const {
    std::vector<AnimationPreset> result;
    result.reserve(preset_order_.size());
    for (const auto& name : preset_order_) {
        auto it = presets_.find(name);
        if (it != presets_.end()) {
            result.push_back(it->second);
        }
    }
    return result;
}

bool PresetManager::save_custom_preset(const std::string& name, const std::string& description,
                                      const ComposableAnimation& comp, TransitionType target) {
    if (name.empty()) return false;
    AnimationPreset p;
    p.name = name;
    p.display_name = "Custom - " + name;
    p.description = description;
    p.target_transition = target;
    p.composition = comp;
    p.is_builtin = false;
    register_preset(p);
    return true;
}

bool PresetManager::delete_custom_preset(const std::string& name) {
    auto it = presets_.find(name);
    if (it == presets_.end() || it->second.is_builtin) return false;
    presets_.erase(it);
    for (auto o_it = preset_order_.begin(); o_it != preset_order_.end(); ++o_it) {
        if (*o_it == name) {
            preset_order_.erase(o_it);
            break;
        }
    }
    return true;
}

std::string PresetManager::serialize_preset_to_json(const AnimationPreset& preset) const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"name\": \"" << preset.name << "\",\n";
    ss << "  \"display_name\": \"" << preset.display_name << "\",\n";
    ss << "  \"description\": \"" << preset.description << "\",\n";
    ss << "  \"target\": \"" << (preset.target_transition == TransitionType::Show ? "show" : "hide") << "\",\n";
    ss << "  \"is_builtin\": " << (preset.is_builtin ? "true" : "false") << "\n";
    ss << "}";
    return ss.str();
}

bool PresetManager::deserialize_preset_from_json(const std::string& json_str, AnimationPreset& out_preset) const {
    if (json_str.find("\"name\"") == std::string::npos) return false;
    auto extract = [&](const std::string& key) -> std::string {
        size_t pos = json_str.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        size_t colon = json_str.find(":", pos);
        if (colon == std::string::npos) return "";
        size_t q1 = json_str.find("\"", colon);
        if (q1 == std::string::npos) return "";
        size_t q2 = json_str.find("\"", q1 + 1);
        if (q2 == std::string::npos) return "";
        return json_str.substr(q1 + 1, q2 - q1 - 1);
    };
    out_preset.name = extract("name");
    out_preset.display_name = extract("display_name");
    if (out_preset.display_name.empty()) out_preset.display_name = out_preset.name;
    out_preset.description = extract("description");
    out_preset.target_transition = (extract("target") == "hide") ? TransitionType::Hide : TransitionType::Show;
    out_preset.is_builtin = false;
    return true;
}

std::string PresetManager::export_all_custom_presets_json() const {
    std::ostringstream ss;
    ss << "[\n";
    bool first = true;
    for (const auto& pair : presets_) {
        if (!pair.second.is_builtin) {
            if (!first) ss << ",\n";
            ss << serialize_preset_to_json(pair.second);
            first = false;
        }
    }
    ss << "\n]";
    return ss.str();
}

bool PresetManager::import_custom_presets_json(const std::string& json_str) {
    if (json_str.empty()) return false;
    return true;
}

} // namespace motionfx
