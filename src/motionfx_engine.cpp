#include "motionfx/motionfx_engine.h"
#include <iostream>

namespace motionfx {

SceneItemAnimationState::SceneItemAnimationState(const SceneItemKey& key)
    : key_(key) {
    // Default Show: Slide In from Left (-300px) + Fade In (0 to 1) over 500ms
    show_config_.configure_slide(-300.0, 0.0, 500.0, EasingConfig::ease_out());
    show_config_.configure_fade(0.0, 1.0, 400.0, EasingConfig::linear());

    // Default Hide: Slide Out to Right (+300px) without Fade over 400ms
    hide_config_.configure_slide(300.0, 0.0, 400.0, EasingConfig::ease_in());
    hide_config_.opacity().enabled = false; // Hide transition has NO fade by default (Section 3)

    // Default base transform
    base_transform_ = Transform2D{0.0f, 0.0f, 0.0f, 1.0f, 1.0f};
    current_transform2d_ = base_transform_;
    current_effects_ = EffectsState{};
}

void SceneItemAnimationState::dispatch_event(const std::string& type, const std::string& status) {
    if (event_dispatcher_) {
        AnimationEventData evt;
        evt.event_type = type;
        evt.scene_name = key_.scene_name;
        evt.item_id = key_.item_id;
        evt.source_name = key_.source_name;
        evt.transition_type = transition_type_to_string(active_transition_);
        evt.progress = timeline_.get_progress();
        evt.timestamp_ms = get_monotonic_time_ms();
        evt.status = status.empty() ? playback_state_to_string(timeline_.get_state()) : status;
        event_dispatcher_(evt);
    }
}

void SceneItemAnimationState::trigger_show() {
    active_transition_ = TransitionType::Show;
    is_preview_ = false;

    // Ensure item is marked visible in OBS
    if (native_visibility_setter_) {
        is_internal_visibility_change_ = true;
        native_visibility_setter_(true);
        is_internal_visibility_change_ = false;
        is_visible_ = true;
    }

    TimelineConfig tc;
    tc.duration_ms = show_config_.get_total_duration_ms();
    tc.direction = PlaybackDirection::Normal;
    timeline_.set_config(tc);

    timeline_.on_started([this]() {
        dispatch_event("AnimationStarted");
    });

    timeline_.on_progress([this](Timeline&, double) {
        update_calculated_state(timeline_.get_elapsed_ms());
        if (native_transform_setter_) {
            native_transform_setter_(current_transform2d_);
        }
        dispatch_event("AnimationProgress");
    });

    timeline_.on_completed([this]() {
        update_calculated_state(show_config_.get_total_duration_ms());
        if (native_transform_setter_) {
            native_transform_setter_(current_transform2d_);
        }
        dispatch_event("AnimationCompleted");
        dispatch_event("ShowCompleted");
    });

    timeline_.play();
}

void SceneItemAnimationState::trigger_hide() {
    active_transition_ = TransitionType::Hide;
    is_preview_ = false;

    // CRITICAL: Keep item visible in OBS while Hide animation is running!
    if (native_visibility_setter_ && !is_visible_) {
        is_internal_visibility_change_ = true;
        native_visibility_setter_(true);
        is_internal_visibility_change_ = false;
        is_visible_ = true;
    }

    TimelineConfig tc;
    tc.duration_ms = hide_config_.get_total_duration_ms();
    tc.direction = PlaybackDirection::Normal;
    timeline_.set_config(tc);

    timeline_.on_started([this]() {
        dispatch_event("AnimationStarted");
    });

    timeline_.on_progress([this](Timeline&, double) {
        update_calculated_state(timeline_.get_elapsed_ms());
        if (native_transform_setter_) {
            native_transform_setter_(current_transform2d_);
        }
        dispatch_event("AnimationProgress");
    });

    timeline_.on_completed([this]() {
        update_calculated_state(hide_config_.get_total_duration_ms());
        if (native_transform_setter_) {
            native_transform_setter_(current_transform2d_);
        }

        // Only hide the item AFTER the animation completes!
        if (native_visibility_setter_) {
            is_internal_visibility_change_ = true;
            native_visibility_setter_(false);
            is_internal_visibility_change_ = false;
            is_visible_ = false;
        }

        dispatch_event("AnimationCompleted");
        dispatch_event("HideCompleted");
    });

    timeline_.play();
}

void SceneItemAnimationState::play_preview(TransitionType type) {
    if (type == TransitionType::None) type = TransitionType::Show;
    active_transition_ = type;
    is_preview_ = true;

    ComposableAnimation& config = (type == TransitionType::Show) ? show_config_ : hide_config_;

    TimelineConfig tc;
    tc.duration_ms = config.get_total_duration_ms();
    timeline_.set_config(tc);

    timeline_.on_started([this]() {
        dispatch_event("AnimationStarted", "preview");
    });

    timeline_.on_progress([this](Timeline&, double) {
        update_calculated_state(timeline_.get_elapsed_ms());
        if (native_transform_setter_) {
            native_transform_setter_(current_transform2d_);
        }
    });

    timeline_.on_completed([this]() {
        update_calculated_state(timeline_.get_elapsed_ms());
        if (native_transform_setter_) {
            native_transform_setter_(current_transform2d_);
        }
        dispatch_event("AnimationCompleted", "preview");
    });

    timeline_.play();
}

void SceneItemAnimationState::pause() {
    timeline_.pause();
    dispatch_event("AnimationPaused");
}

void SceneItemAnimationState::resume() {
    timeline_.resume();
    dispatch_event("AnimationResumed");
}

void SceneItemAnimationState::reverse() {
    timeline_.reverse();
    dispatch_event("AnimationReversed");
}

void SceneItemAnimationState::stop() {
    timeline_.stop();
    // Restore base transform cleanly
    current_transform2d_ = base_transform_;
    current_effects_ = EffectsState{};
    if (native_transform_setter_) {
        native_transform_setter_(current_transform2d_);
    }
    dispatch_event("AnimationCancelled");
}

void SceneItemAnimationState::reset() {
    timeline_.reset();
    current_transform2d_ = base_transform_;
    current_effects_ = EffectsState{};
    if (native_transform_setter_) {
        native_transform_setter_(current_transform2d_);
    }
}

void SceneItemAnimationState::tick(TimeMs current_time_ms) {
    timeline_.tick(current_time_ms);
}

void SceneItemAnimationState::advance_delta(TimeMs delta_ms) {
    timeline_.advance_delta(delta_ms);
}

void SceneItemAnimationState::update_calculated_state(TimeMs elapsed_ms) {
    if (active_transition_ == TransitionType::Show) {
        show_config_.evaluate(elapsed_ms);
        current_transform2d_ = show_config_.apply_to_transform2d(base_transform_);
        current_transform3d_ = show_config_.get_transform3d();
        current_effects_ = show_config_.get_effects_state();
    } else if (active_transition_ == TransitionType::Hide) {
        hide_config_.evaluate(elapsed_ms);
        current_transform2d_ = hide_config_.apply_to_transform2d(base_transform_);
        current_transform3d_ = hide_config_.get_transform3d();
        current_effects_ = hide_config_.get_effects_state();
    } else {
        current_transform2d_ = base_transform_;
        current_transform3d_ = Transform3D{};
        current_effects_ = EffectsState{};
    }
}

bool SceneItemAnimationState::set_parameter(const std::string& param_name, double value, TransitionType target) {
    auto apply_track = [&](ComposableAnimation& comp) -> bool {
        PropertyTrack* trk = comp.get_track(param_name);
        if (trk) {
            trk->start_val = value;
            return true;
        }
        return false;
    };

    bool applied = false;
    if (target == TransitionType::Show || target == TransitionType::None) {
        applied |= apply_track(show_config_);
    }
    if (target == TransitionType::Hide || target == TransitionType::None) {
        applied |= apply_track(hide_config_);
    }

    // Re-evaluate if active so movement remains continuous without jumps
    if (timeline_.is_playing()) {
        update_calculated_state(timeline_.get_elapsed_ms());
    }
    return applied;
}

bool SceneItemAnimationState::set_enabled(const std::string& component_name, bool enabled, TransitionType target) {
    auto apply_track = [&](ComposableAnimation& comp) -> bool {
        PropertyTrack* trk = comp.get_track(component_name);
        if (trk) {
            trk->enabled = enabled;
            return true;
        }
        return false;
    };

    bool applied = false;
    if (target == TransitionType::Show || target == TransitionType::None) {
        applied |= apply_track(show_config_);
    }
    if (target == TransitionType::Hide || target == TransitionType::None) {
        applied |= apply_track(hide_config_);
    }

    if (timeline_.is_playing()) {
        update_calculated_state(timeline_.get_elapsed_ms());
    }
    return applied;
}

bool SceneItemAnimationState::set_time(TimeMs duration_ms, TransitionType target) {
    auto apply_track = [&](ComposableAnimation& comp) {
        // Set duration on all enabled tracks
        for (const std::string& name : {"pos_x", "pos_y", "pos_z", "scale_x", "scale_y",
                                        "rot_z", "rot_x", "rot_y", "opacity", "blur", "mask", "text"}) {
            PropertyTrack* trk = comp.get_track(name);
            if (trk && trk->enabled) {
                trk->duration_ms = duration_ms;
            }
        }
    };

    if (target == TransitionType::Show || target == TransitionType::None) {
        apply_track(show_config_);
    }
    if (target == TransitionType::Hide || target == TransitionType::None) {
        apply_track(hide_config_);
    }
    return true;
}

bool SceneItemAnimationState::set_easing(const std::string& easing_name, TransitionType target) {
    EasingConfig ec;
    ec.type = easing_from_string(easing_name);

    auto apply_track = [&](ComposableAnimation& comp) {
        for (const std::string& name : {"pos_x", "pos_y", "pos_z", "scale_x", "scale_y",
                                        "rot_z", "rot_x", "rot_y", "opacity", "blur", "mask", "text"}) {
            PropertyTrack* trk = comp.get_track(name);
            if (trk && trk->enabled) {
                trk->easing = ec;
            }
        }
    };

    if (target == TransitionType::Show || target == TransitionType::None) {
        apply_track(show_config_);
    }
    if (target == TransitionType::Hide || target == TransitionType::None) {
        apply_track(hide_config_);
    }
    return true;
}

// ============================================================================
// AnimationEngine Implementation
// ============================================================================

AnimationEngine& AnimationEngine::instance() {
    static AnimationEngine s_instance;
    return s_instance;
}

std::shared_ptr<SceneItemAnimationState> AnimationEngine::get_or_create(const SceneItemKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = items_.find(key);
    if (it != items_.end()) {
        return it->second;
    }

    auto item = std::make_shared<SceneItemAnimationState>(key);
    item->set_event_dispatcher([this](const AnimationEventData& evt) {
        this->dispatch_event(evt);
    });
    items_[key] = item;
    return item;
}

std::shared_ptr<SceneItemAnimationState> AnimationEngine::find(const SceneItemKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = items_.find(key);
    return (it != items_.end()) ? it->second : nullptr;
}

std::shared_ptr<SceneItemAnimationState> AnimationEngine::find(const std::string& scene_name, int64_t item_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : items_) {
        if (pair.first.scene_name == scene_name && pair.first.item_id == item_id) {
            return pair.second;
        }
    }
    return nullptr;
}

std::shared_ptr<SceneItemAnimationState> AnimationEngine::find_by_source(const std::string& scene_name, const std::string& source_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pair : items_) {
        if (pair.first.scene_name == scene_name && pair.first.source_name == source_name) {
            return pair.second;
        }
    }
    return nullptr;
}

void AnimationEngine::remove(const SceneItemKey& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    items_.erase(key);
}

void AnimationEngine::clear_scene(const std::string& scene_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = items_.begin(); it != items_.end(); ) {
        if (it->first.scene_name == scene_name) {
            it = items_.erase(it);
        } else {
            ++it;
        }
    }
}

void AnimationEngine::clear_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    items_.clear();
}

void AnimationEngine::tick(TimeMs current_time_ms) {
    std::vector<std::shared_ptr<SceneItemAnimationState>> active_items;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& pair : items_) {
            if (pair.second->is_animating()) {
                active_items.push_back(pair.second);
            }
        }
    }

    for (auto& item : active_items) {
        item->tick(current_time_ms);
    }
}

void AnimationEngine::advance_delta(TimeMs delta_ms) {
    std::vector<std::shared_ptr<SceneItemAnimationState>> active_items;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& pair : items_) {
            if (pair.second->is_animating()) {
                active_items.push_back(pair.second);
            }
        }
    }

    for (auto& item : active_items) {
        item->advance_delta(delta_ms);
    }
}

void AnimationEngine::add_event_listener(AnimationEventListener listener) {
    std::lock_guard<std::mutex> lock(mutex_);
    listeners_.push_back(std::move(listener));
}

void AnimationEngine::dispatch_event(const AnimationEventData& event) {
    std::vector<AnimationEventListener> listeners_copy;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        listeners_copy = listeners_;
    }
    for (const auto& listener : listeners_copy) {
        if (listener) listener(event);
    }
}

std::vector<SceneItemKey> AnimationEngine::get_active_keys() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SceneItemKey> keys;
    for (const auto& pair : items_) {
        keys.push_back(pair.first);
    }
    return keys;
}

} // namespace motionfx
