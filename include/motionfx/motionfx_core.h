#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <memory>
#include <vector>
#include <functional>

namespace motionfx {

using TimeMs = double;

// High precision monotonic clock in milliseconds
inline TimeMs get_monotonic_time_ms() {
    using namespace std::chrono;
    return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
}

enum class TransitionType {
    None,
    Show,
    Hide
};

inline const char* transition_type_to_string(TransitionType type) {
    switch (type) {
        case TransitionType::Show: return "show";
        case TransitionType::Hide: return "hide";
        default: return "none";
    }
}

inline TransitionType transition_type_from_string(const std::string& str) {
    if (str == "show" || str == "Show") return TransitionType::Show;
    if (str == "hide" || str == "Hide") return TransitionType::Hide;
    return TransitionType::None;
}

enum class PlaybackState {
    Stopped,
    Playing,
    Paused,
    Completed
};

inline const char* playback_state_to_string(PlaybackState state) {
    switch (state) {
        case PlaybackState::Stopped: return "stopped";
        case PlaybackState::Playing: return "playing";
        case PlaybackState::Paused: return "paused";
        case PlaybackState::Completed: return "completed";
        default: return "unknown";
    }
}

enum class PlaybackDirection {
    Normal,
    Reverse
};

struct SceneItemKey {
    std::string scene_name;
    int64_t item_id{0};
    std::string source_name;

    bool operator<(const SceneItemKey& o) const {
        if (scene_name != o.scene_name) return scene_name < o.scene_name;
        return item_id < o.item_id;
    }

    bool operator==(const SceneItemKey& o) const {
        return scene_name == o.scene_name && item_id == o.item_id;
    }
};

struct Transform2D {
    float pos_x{0.0f};
    float pos_y{0.0f};
    float rot{0.0f};      // Degrees around Z
    float scale_x{1.0f};
    float scale_y{1.0f};
};

struct Transform3D {
    float pos_z{0.0f};    // 3D depth offset
    float rot_x{0.0f};    // Degrees around X (Flip X)
    float rot_y{0.0f};    // Degrees around Y (Flip Y / Card Flip)
    float rot_z{0.0f};    // Extra 3D rotation Z
    float fov{60.0f};     // Perspective field of view in degrees
};

struct EffectsState {
    float opacity{1.0f};       // 0.0 to 1.0
    float blur_radius{0.0f};   // Pixels
    float motion_blur_samples{0.0f};
    float mask_progress{1.0f}; // 0.0 to 1.0 (1.0 = fully revealed)
    int mask_type{0};          // 0 = none, 1 = linear wipe, 2 = radial circle, 3 = box
    float mask_angle{0.0f};    // Degrees
    float mask_feather{0.05f}; // Feather softness
};

} // namespace motionfx
