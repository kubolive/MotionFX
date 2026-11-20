#pragma once

#include "motionfx/motionfx_core.h"
#include "motionfx/motionfx_easing.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace motionfx {

struct PropertyTrack {
    std::string name;
    bool enabled{false};
    TimeMs delay_ms{0.0};
    TimeMs duration_ms{500.0};
    EasingConfig easing{EasingConfig::ease_out()};
    double start_val{0.0};
    double end_val{0.0};
    double default_val{0.0}; // value used when disabled
    double current_val{0.0};

    void evaluate(TimeMs elapsed_ms, bool reverse = false) {
        if (!enabled) {
            current_val = default_val;
            return;
        }

        if (duration_ms <= 0.0) {
            current_val = reverse ? start_val : end_val;
            return;
        }

        if (elapsed_ms < delay_ms) {
            current_val = reverse ? end_val : start_val;
            return;
        }

        TimeMs active_time = elapsed_ms - delay_ms;
        if (active_time >= duration_ms) {
            current_val = reverse ? start_val : end_val;
            return;
        }

        double t = active_time / duration_ms;
        double eased_t;
        if (reverse) {
            // In reverse (Hide), start at end_val (on-screen resting state at t=0)
            // and move to start_val (hidden/offset state at t=1).
            // If easing is an Out curve (typical Show preset default), mirror it to In for smooth exit acceleration.
            // If user explicitly selected an In curve or Linear, evaluate it directly!
            bool is_out = (easing.type == EasingType::CubicOut ||
                           easing.type == EasingType::QuadOut ||
                           easing.type == EasingType::SineOut ||
                           easing.type == EasingType::QuartOut ||
                           easing.type == EasingType::QuintOut ||
                           easing.type == EasingType::ExpoOut ||
                           easing.type == EasingType::CircOut ||
                           easing.type == EasingType::BackOut ||
                           easing.type == EasingType::ElasticOut ||
                           easing.type == EasingType::BounceOut);
            if (is_out) {
                eased_t = 1.0 - evaluate_easing(easing, 1.0 - t);
            } else {
                eased_t = evaluate_easing(easing, t);
            }
            current_val = end_val + (start_val - end_val) * eased_t;
        } else {
            eased_t = evaluate_easing(easing, t);
            current_val = start_val + (end_val - start_val) * eased_t;
        }
    }

    TimeMs get_total_time_ms() const {
        return enabled ? (delay_ms + duration_ms) : 0.0;
    }
};

class ComposableAnimation {
public:
    ComposableAnimation();

    // Track accessors
    PropertyTrack& pos_x() { return track_pos_x_; }
    PropertyTrack& pos_y() { return track_pos_y_; }
    PropertyTrack& pos_z() { return track_pos_z_; }
    PropertyTrack& scale_x() { return track_scale_x_; }
    PropertyTrack& scale_y() { return track_scale_y_; }
    PropertyTrack& rot_z() { return track_rot_z_; }
    PropertyTrack& rot_x() { return track_rot_x_; }
    PropertyTrack& rot_y() { return track_rot_y_; }
    PropertyTrack& opacity() { return track_opacity_; }
    PropertyTrack& blur_radius() { return track_blur_radius_; }
    PropertyTrack& motion_blur() { return track_motion_blur_; }
    PropertyTrack& mask_progress() { return track_mask_progress_; }
    PropertyTrack& text_progress() { return track_text_progress_; }

    const PropertyTrack& pos_x() const { return track_pos_x_; }
    const PropertyTrack& pos_y() const { return track_pos_y_; }
    const PropertyTrack& pos_z() const { return track_pos_z_; }
    const PropertyTrack& scale_x() const { return track_scale_x_; }
    const PropertyTrack& scale_y() const { return track_scale_y_; }
    const PropertyTrack& rot_z() const { return track_rot_z_; }
    const PropertyTrack& rot_x() const { return track_rot_x_; }
    const PropertyTrack& rot_y() const { return track_rot_y_; }
    const PropertyTrack& opacity() const { return track_opacity_; }
    const PropertyTrack& blur_radius() const { return track_blur_radius_; }
    const PropertyTrack& motion_blur() const { return track_motion_blur_; }
    const PropertyTrack& mask_progress() const { return track_mask_progress_; }
    const PropertyTrack& text_progress() const { return track_text_progress_; }

    PropertyTrack* get_track(const std::string& name);
    const PropertyTrack* get_track(const std::string& name) const;

    // Evaluates all tracks at a given timeline position in ms (supports reverse for Hide transitions)
    void evaluate(TimeMs elapsed_ms, bool reverse = false);

    // Calculates maximum duration among all enabled tracks
    TimeMs get_total_duration_ms() const;

    // Apply calculated delta to a base 2D transform
    Transform2D apply_to_transform2d(const Transform2D& base) const;
    Transform3D get_transform3d() const;
    EffectsState get_effects_state() const;

    // Helper setup methods
    void configure_slide(double offset_x, double offset_y, TimeMs duration = 500.0,
                         const EasingConfig& easing = EasingConfig::ease_out());
    void configure_fade(double start_alpha, double end_alpha, TimeMs duration = 500.0,
                        const EasingConfig& easing = EasingConfig::linear());
    void configure_scale(double start_scale, double end_scale, TimeMs duration = 500.0,
                         const EasingConfig& easing = EasingConfig::ease_out());
    void configure_rotation(double start_angle_deg, double end_angle_deg, TimeMs duration = 500.0,
                            const EasingConfig& easing = EasingConfig::ease_out());
    void configure_3d_flip(double start_rx, double start_ry, double end_rx, double end_ry,
                           TimeMs duration = 600.0, const EasingConfig& easing = EasingConfig::ease_out());
    void configure_blur(double start_blur, double end_blur, TimeMs duration = 500.0,
                        const EasingConfig& easing = EasingConfig::ease_out());
    void configure_mask(double start_p, double end_p, int mask_type, float angle,
                        TimeMs duration = 500.0, const EasingConfig& easing = EasingConfig::ease_out());
    void configure_text(int mode, TimeMs duration = 1000.0,
                        const EasingConfig& easing = EasingConfig::linear());

    // Mask metadata
    int mask_type{0};       // 0=none, 1=linear wipe, 2=radial, 3=box
    float mask_angle{0.0f};
    float mask_feather{0.05f};

    // Text metadata
    int text_mode{0};       // 0=none, 1=typewriter, 2=word stagger, 3=line reveal
    int text_stagger_ms{50};

private:
    PropertyTrack track_pos_x_;
    PropertyTrack track_pos_y_;
    PropertyTrack track_pos_z_;
    PropertyTrack track_scale_x_;
    PropertyTrack track_scale_y_;
    PropertyTrack track_rot_z_;
    PropertyTrack track_rot_x_;
    PropertyTrack track_rot_y_;
    PropertyTrack track_opacity_;
    PropertyTrack track_blur_radius_;
    PropertyTrack track_motion_blur_;
    PropertyTrack track_mask_progress_;
    PropertyTrack track_text_progress_;
};

} // namespace motionfx
