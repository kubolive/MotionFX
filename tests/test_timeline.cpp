#include "motionfx/motionfx_timeline.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    using namespace motionfx;

    std::cout << "[TEST] Testing MotionFX Timeline..." << std::endl;

    TimelineConfig config;
    config.duration_ms = 400.0;
    config.delay_ms = 100.0;
    config.direction = PlaybackDirection::Normal;

    Timeline timeline(config);

    bool started = false;
    bool completed = false;
    int progress_count = 0;

    timeline.on_started([&]() { started = true; });
    timeline.on_completed([&]() { completed = true; });
    timeline.on_progress([&](Timeline&, double p) {
        progress_count++;
    });

    timeline.play();
    assert(started);
    assert(timeline.is_playing());
    assert(timeline.get_progress() == 0.0);

    // During delay (50ms elapsed, delay is 100ms)
    timeline.advance_delta(50.0);
    assert(timeline.get_progress() == 0.0);

    // At delay end + 100ms active (100ms delay + 100ms active = 200ms elapsed out of 400ms duration -> 25%)
    timeline.advance_delta(150.0); // total 200ms
    assert(std::abs(timeline.get_progress() - 0.25) < 1e-4);

    // Midpoint active (total 300ms elapsed -> 200ms active out of 400ms -> 50%)
    timeline.advance_delta(100.0);
    assert(std::abs(timeline.get_progress() - 0.50) < 1e-4);

    // Pause and Resume
    timeline.pause();
    assert(timeline.get_state() == PlaybackState::Paused);
    timeline.advance_delta(50.0); // should not advance when paused
    assert(std::abs(timeline.get_progress() - 0.50) < 1e-4);

    timeline.resume();
    assert(timeline.is_playing());

    // Advance to completion
    timeline.advance_delta(200.0); // total 500ms >= 100ms delay + 400ms duration
    assert(completed);
    assert(timeline.is_completed());
    assert(timeline.get_progress() == 1.0);

    std::cout << "  -> Basic playback & delay passed. Progress ticks: " << progress_count << std::endl;

    // Test Reverse
    timeline.play(PlaybackDirection::Reverse);
    assert(timeline.get_progress() == 1.0);
    timeline.advance_delta(100.0); // delay
    assert(timeline.get_progress() == 1.0);
    timeline.advance_delta(200.0); // 50%
    assert(std::abs(timeline.get_progress() - 0.50) < 1e-4);
    timeline.advance_delta(200.0); // complete
    assert(timeline.get_progress() == 0.0);
    assert(timeline.is_completed());

    std::cout << "  -> Reverse playback passed." << std::endl;
    std::cout << "[SUCCESS] All Timeline tests passed successfully!" << std::endl;
    return 0;
}
