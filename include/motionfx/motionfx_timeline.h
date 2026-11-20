#pragma once

#include "motionfx/motionfx_core.h"
#include <functional>

namespace motionfx {

struct TimelineConfig {
    TimeMs duration_ms{500.0};
    TimeMs delay_ms{0.0};
    int repeat_count{0};        // 0 = play once, >0 = repeat N times, -1 = infinite
    TimeMs repeat_delay_ms{0.0};
    PlaybackDirection direction{PlaybackDirection::Normal};
    bool auto_reverse{false};   // Yoyo effect
};

class Timeline {
public:
    Timeline();
    explicit Timeline(const TimelineConfig& config);

    void set_config(const TimelineConfig& config);
    const TimelineConfig& get_config() const { return config_; }

    void play(PlaybackDirection dir = PlaybackDirection::Normal);
    void pause();
    void resume();
    void reverse();
    void stop();
    void reset();

    // Advance timeline using monotonic timestamp or explicit delta
    void tick(TimeMs current_time_ms);
    void advance_delta(TimeMs delta_ms);

    // Seek to specific timeline position in ms
    void seek(TimeMs time_ms);

    // Getters
    double get_progress() const { return normalized_progress_; }
    TimeMs get_elapsed_ms() const { return elapsed_ms_; }
    PlaybackState get_state() const { return state_; }
    PlaybackDirection get_direction() const { return current_direction_; }
    bool is_playing() const { return state_ == PlaybackState::Playing; }
    bool is_completed() const { return state_ == PlaybackState::Completed; }

    // Event callbacks
    using EventCallback = std::function<void(Timeline&, double progress)>;
    void on_started(std::function<void()> cb) { started_cb_ = std::move(cb); }
    void on_progress(EventCallback cb) { progress_cb_ = std::move(cb); }
    void on_completed(std::function<void()> cb) { completed_cb_ = std::move(cb); }
    void on_paused(std::function<void()> cb) { paused_cb_ = std::move(cb); }
    void on_resumed(std::function<void()> cb) { resumed_cb_ = std::move(cb); }
    void on_stopped(std::function<void()> cb) { stopped_cb_ = std::move(cb); }

private:
    void update_progress();

    TimelineConfig config_;
    PlaybackState state_{PlaybackState::Stopped};
    PlaybackDirection current_direction_{PlaybackDirection::Normal};

    TimeMs elapsed_ms_{0.0};
    TimeMs last_tick_time_{0.0};
    double normalized_progress_{0.0};
    int current_repeat_{0};
    bool is_yoyo_reverse_{false};

    std::function<void()> started_cb_;
    EventCallback progress_cb_;
    std::function<void()> completed_cb_;
    std::function<void()> paused_cb_;
    std::function<void()> resumed_cb_;
    std::function<void()> stopped_cb_;
};

} // namespace motionfx
