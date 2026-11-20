#pragma once

#include "motionfx/motionfx_core.h"
#include "motionfx/motionfx_tracks.h"
#include "motionfx/motionfx_timeline.h"
#include <memory>
#include <mutex>
#include <map>
#include <vector>
#include <string>
#include <functional>

namespace motionfx {

struct AnimationEventData {
    std::string event_type;
    std::string scene_name;
    int64_t item_id{0};
    std::string source_name;
    std::string transition_type;
    std::string preset_name;
    double progress{0.0};
    TimeMs timestamp_ms{0.0};
    std::string status;
};

using AnimationEventListener = std::function<void(const AnimationEventData&)>;

class SceneItemAnimationState : public std::enable_shared_from_this<SceneItemAnimationState> {
public:
    explicit SceneItemAnimationState(const SceneItemKey& key);

    const SceneItemKey& get_key() const { return key_; }

    // Configuration for Show and Hide transitions
    ComposableAnimation& show_config() { return show_config_; }
    const ComposableAnimation& show_config() const { return show_config_; }

    ComposableAnimation& hide_config() { return hide_config_; }
    const ComposableAnimation& hide_config() const { return hide_config_; }

    // Base transform management (user's design layout in OBS)
    void set_base_transform(const Transform2D& base) {
        base_transform_ = base;
        if (!timeline_.is_playing()) {
            current_transform2d_ = base;
        }
    }
    const Transform2D& get_base_transform() const { return base_transform_; }

    // Animation controls
    void trigger_show();
    void trigger_hide();
    void play_preview(TransitionType type);
    void pause();
    void resume();
    void reverse();
    void stop();
    void reset();

    // Timeline tick
    void tick(TimeMs current_time_ms);
    void advance_delta(TimeMs delta_ms);

    // Runtime parameter modification
    bool set_parameter(const std::string& param_name, double value, TransitionType target = TransitionType::None);
    bool set_enabled(const std::string& component_name, bool enabled, TransitionType target = TransitionType::None);
    bool set_time(TimeMs duration_ms, TransitionType target = TransitionType::None);
    bool set_easing(const std::string& easing_name, TransitionType target = TransitionType::None);

    // Current output state (ready for OBS rendering)
    Transform2D get_current_transform2d() const { return current_transform2d_; }
    Transform3D get_current_transform3d() const { return current_transform3d_; }
    EffectsState get_current_effects() const { return current_effects_; }

    TransitionType get_active_transition() const { return active_transition_; }
    PlaybackState get_playback_state() const { return timeline_.get_state(); }
    double get_progress() const { return timeline_.get_progress(); }
    bool is_animating() const { return timeline_.is_playing(); }

    // Visibility state & re-entrancy tracking
    bool is_visible() const { return is_visible_; }
    void set_visible_native(bool v) { is_visible_ = v; }

    bool is_internal_visibility_change() const { return is_internal_visibility_change_; }
    void set_internal_visibility_change(bool b) { is_internal_visibility_change_ = b; }

    // Native OBS hook callbacks (injected when compiled inside OBS module)
    using NativeVisibilitySetter = std::function<void(bool visible)>;
    using NativeTransformSetter = std::function<void(const Transform2D& t2d)>;
    void set_native_hooks(NativeVisibilitySetter vis_setter, NativeTransformSetter trans_setter) {
        native_visibility_setter_ = std::move(vis_setter);
        native_transform_setter_ = std::move(trans_setter);
    }

    void set_event_dispatcher(std::function<void(const AnimationEventData&)> dispatcher) {
        event_dispatcher_ = std::move(dispatcher);
    }

private:
    void dispatch_event(const std::string& type, const std::string& status = "");
    void update_calculated_state(TimeMs elapsed_ms);

    SceneItemKey key_;
    Transform2D base_transform_;

    ComposableAnimation show_config_;
    ComposableAnimation hide_config_;

    Timeline timeline_;
    TransitionType active_transition_{TransitionType::None};

    bool is_visible_{true};
    bool is_internal_visibility_change_{false};
    bool is_preview_{false};

    Transform2D current_transform2d_;
    Transform3D current_transform3d_;
    EffectsState current_effects_;

    NativeVisibilitySetter native_visibility_setter_;
    NativeTransformSetter native_transform_setter_;
    std::function<void(const AnimationEventData&)> event_dispatcher_;
};

class AnimationEngine {
public:
    static AnimationEngine& instance();

    std::shared_ptr<SceneItemAnimationState> get_or_create(const SceneItemKey& key);
    std::shared_ptr<SceneItemAnimationState> find(const SceneItemKey& key);
    std::shared_ptr<SceneItemAnimationState> find(const std::string& scene_name, int64_t item_id);
    std::shared_ptr<SceneItemAnimationState> find_by_source(const std::string& scene_name, const std::string& source_name);

    void remove(const SceneItemKey& key);
    void clear_scene(const std::string& scene_name);
    void clear_all();

    // Advances all active item animations
    void tick(TimeMs current_time_ms);
    void advance_delta(TimeMs delta_ms);

    // Event system
    void add_event_listener(AnimationEventListener listener);
    void dispatch_event(const AnimationEventData& event);

    // Global items query
    std::vector<SceneItemKey> get_active_keys() const;

private:
    AnimationEngine() = default;
    ~AnimationEngine() = default;
    AnimationEngine(const AnimationEngine&) = delete;
    AnimationEngine& operator=(const AnimationEngine&) = delete;

    mutable std::mutex mutex_;
    std::map<SceneItemKey, std::shared_ptr<SceneItemAnimationState>> items_;
    std::vector<AnimationEventListener> listeners_;
};

} // namespace motionfx
