#include "motionfx/motionfx_timeline.h"
#include <algorithm>

namespace motionfx {

Timeline::Timeline() : Timeline(TimelineConfig{}) {}

Timeline::Timeline(const TimelineConfig& config)
    : config_(config), current_direction_(config.direction) {}

void Timeline::set_config(const TimelineConfig& config) {
    config_ = config;
    current_direction_ = config.direction;
}

void Timeline::play(PlaybackDirection dir) {
    current_direction_ = dir;
    state_ = PlaybackState::Playing;
    elapsed_ms_ = 0.0;
    last_tick_time_ = get_monotonic_time_ms();
    current_repeat_ = 0;
    is_yoyo_reverse_ = false;

    update_progress();
    if (started_cb_) started_cb_();
}

void Timeline::pause() {
    if (state_ == PlaybackState::Playing) {
        state_ = PlaybackState::Paused;
        if (paused_cb_) paused_cb_();
    }
}

void Timeline::resume() {
    if (state_ == PlaybackState::Paused) {
        state_ = PlaybackState::Playing;
        last_tick_time_ = get_monotonic_time_ms();
        if (resumed_cb_) resumed_cb_();
    }
}

void Timeline::reverse() {
    if (current_direction_ == PlaybackDirection::Normal) {
        current_direction_ = PlaybackDirection::Reverse;
    } else {
        current_direction_ = PlaybackDirection::Normal;
    }

    if (state_ == PlaybackState::Completed) {
        state_ = PlaybackState::Playing;
        last_tick_time_ = get_monotonic_time_ms();
    }
}

void Timeline::stop() {
    if (state_ != PlaybackState::Stopped) {
        state_ = PlaybackState::Stopped;
        if (stopped_cb_) stopped_cb_();
    }
}

void Timeline::reset() {
    state_ = PlaybackState::Stopped;
    elapsed_ms_ = 0.0;
    current_repeat_ = 0;
    is_yoyo_reverse_ = false;
    update_progress();
}

void Timeline::seek(TimeMs time_ms) {
    elapsed_ms_ = std::max(0.0, time_ms);
    update_progress();
}

void Timeline::tick(TimeMs current_time_ms) {
    if (state_ != PlaybackState::Playing) return;

    if (last_tick_time_ <= 0.0) {
        last_tick_time_ = current_time_ms;
        return;
    }

    TimeMs delta = current_time_ms - last_tick_time_;
    last_tick_time_ = current_time_ms;

    // Clamp huge deltas (e.g. system sleep/stall) to 250ms
    if (delta > 250.0) delta = 250.0;
    if (delta < 0.0) delta = 0.0;

    advance_delta(delta);
}

void Timeline::advance_delta(TimeMs delta_ms) {
    if (state_ != PlaybackState::Playing) return;

    elapsed_ms_ += delta_ms;
    update_progress();

    if (progress_cb_) {
        progress_cb_(*this, normalized_progress_);
    }

    TimeMs total_cycle_duration = config_.delay_ms + config_.duration_ms;

    if (elapsed_ms_ >= total_cycle_duration) {
        // Handle repeat or yoyo
        if (config_.auto_reverse && !is_yoyo_reverse_) {
            is_yoyo_reverse_ = true;
            elapsed_ms_ = 0.0; // restart in reverse
            return;
        }

        is_yoyo_reverse_ = false;

        bool has_more_repeats = (config_.repeat_count < 0) || (current_repeat_ < config_.repeat_count);

        if (has_more_repeats) {
            current_repeat_++;
            elapsed_ms_ = -config_.repeat_delay_ms; // delay before next cycle
        } else {
            state_ = PlaybackState::Completed;
            normalized_progress_ = (current_direction_ == PlaybackDirection::Normal) ? 1.0 : 0.0;
            if (completed_cb_) completed_cb_();
        }
    }
}

void Timeline::update_progress() {
    if (config_.duration_ms <= 0.0) {
        normalized_progress_ = (current_direction_ == PlaybackDirection::Normal) ? 1.0 : 0.0;
        return;
    }

    if (elapsed_ms_ < config_.delay_ms) {
        normalized_progress_ = (current_direction_ == PlaybackDirection::Normal) ? 0.0 : 1.0;
        return;
    }

    TimeMs active_time = elapsed_ms_ - config_.delay_ms;
    double p = std::clamp(active_time / config_.duration_ms, 0.0, 1.0);

    bool effective_reverse = (current_direction_ == PlaybackDirection::Reverse);
    if (is_yoyo_reverse_) {
        effective_reverse = !effective_reverse;
    }

    if (effective_reverse) {
        normalized_progress_ = 1.0 - p;
    } else {
        normalized_progress_ = p;
    }
}

} // namespace motionfx
