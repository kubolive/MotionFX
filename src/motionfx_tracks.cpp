#include "motionfx/motionfx_tracks.h"
#include <algorithm>

namespace motionfx {

ComposableAnimation::ComposableAnimation() {
    track_pos_x_.name = "pos_x";
    track_pos_x_.default_val = 0.0;
    track_pos_x_.current_val = 0.0;

    track_pos_y_.name = "pos_y";
    track_pos_y_.default_val = 0.0;
    track_pos_y_.current_val = 0.0;

    track_pos_z_.name = "pos_z";
    track_pos_z_.default_val = 0.0;
    track_pos_z_.current_val = 0.0;

    track_scale_x_.name = "scale_x";
    track_scale_x_.default_val = 1.0;
    track_scale_x_.current_val = 1.0;

    track_scale_y_.name = "scale_y";
    track_scale_y_.default_val = 1.0;
    track_scale_y_.current_val = 1.0;

    track_rot_z_.name = "rot_z";
    track_rot_z_.default_val = 0.0;
    track_rot_z_.current_val = 0.0;

    track_rot_x_.name = "rot_x";
    track_rot_x_.default_val = 0.0;
    track_rot_x_.current_val = 0.0;

    track_rot_y_.name = "rot_y";
    track_rot_y_.default_val = 0.0;
    track_rot_y_.current_val = 0.0;

    track_opacity_.name = "opacity";
    track_opacity_.default_val = 1.0;
    track_opacity_.current_val = 1.0;

    track_blur_radius_.name = "blur_radius";
    track_blur_radius_.default_val = 0.0;
    track_blur_radius_.current_val = 0.0;

    track_motion_blur_.name = "motion_blur";
    track_motion_blur_.default_val = 0.0;
    track_motion_blur_.current_val = 0.0;

    track_mask_progress_.name = "mask_progress";
    track_mask_progress_.default_val = 1.0;
    track_mask_progress_.current_val = 1.0;

    track_text_progress_.name = "text_progress";
    track_text_progress_.default_val = 1.0;
    track_text_progress_.current_val = 1.0;
}

PropertyTrack* ComposableAnimation::get_track(const std::string& name) {
    if (name == "pos_x" || name == "position_x") return &track_pos_x_;
    if (name == "pos_y" || name == "position_y") return &track_pos_y_;
    if (name == "pos_z" || name == "position_z") return &track_pos_z_;
    if (name == "scale_x") return &track_scale_x_;
    if (name == "scale_y") return &track_scale_y_;
    if (name == "rot_z" || name == "rotation" || name == "rot") return &track_rot_z_;
    if (name == "rot_x" || name == "rotation_x") return &track_rot_x_;
    if (name == "rot_y" || name == "rotation_y") return &track_rot_y_;
    if (name == "opacity" || name == "fade" || name == "alpha") return &track_opacity_;
    if (name == "blur" || name == "blur_radius") return &track_blur_radius_;
    if (name == "motion_blur") return &track_motion_blur_;
    if (name == "mask" || name == "mask_progress") return &track_mask_progress_;
    if (name == "text" || name == "text_progress") return &track_text_progress_;
    return nullptr;
}

const PropertyTrack* ComposableAnimation::get_track(const std::string& name) const {
    if (name == "pos_x" || name == "position_x") return &track_pos_x_;
    if (name == "pos_y" || name == "position_y") return &track_pos_y_;
    if (name == "pos_z" || name == "position_z") return &track_pos_z_;
    if (name == "scale_x") return &track_scale_x_;
    if (name == "scale_y") return &track_scale_y_;
    if (name == "rot_z" || name == "rotation" || name == "rot") return &track_rot_z_;
    if (name == "rot_x" || name == "rotation_x") return &track_rot_x_;
    if (name == "rot_y" || name == "rotation_y") return &track_rot_y_;
    if (name == "opacity" || name == "fade" || name == "alpha") return &track_opacity_;
    if (name == "blur" || name == "blur_radius") return &track_blur_radius_;
    if (name == "motion_blur") return &track_motion_blur_;
    if (name == "mask" || name == "mask_progress") return &track_mask_progress_;
    if (name == "text" || name == "text_progress") return &track_text_progress_;
    return nullptr;
}

void ComposableAnimation::evaluate(TimeMs elapsed_ms, bool reverse) {
    track_pos_x_.evaluate(elapsed_ms, reverse);
    track_pos_y_.evaluate(elapsed_ms, reverse);
    track_pos_z_.evaluate(elapsed_ms, reverse);
    track_scale_x_.evaluate(elapsed_ms, reverse);
    track_scale_y_.evaluate(elapsed_ms, reverse);
    track_rot_z_.evaluate(elapsed_ms, reverse);
    track_rot_x_.evaluate(elapsed_ms, reverse);
    track_rot_y_.evaluate(elapsed_ms, reverse);
    track_opacity_.evaluate(elapsed_ms, reverse);
    track_blur_radius_.evaluate(elapsed_ms, reverse);
    track_motion_blur_.evaluate(elapsed_ms, reverse);
    track_mask_progress_.evaluate(elapsed_ms, reverse);
    track_text_progress_.evaluate(elapsed_ms, reverse);
}

TimeMs ComposableAnimation::get_total_duration_ms() const {
    TimeMs max_time = 0.0;
    max_time = std::max(max_time, track_pos_x_.get_total_time_ms());
    max_time = std::max(max_time, track_pos_y_.get_total_time_ms());
    max_time = std::max(max_time, track_pos_z_.get_total_time_ms());
    max_time = std::max(max_time, track_scale_x_.get_total_time_ms());
    max_time = std::max(max_time, track_scale_y_.get_total_time_ms());
    max_time = std::max(max_time, track_rot_z_.get_total_time_ms());
    max_time = std::max(max_time, track_rot_x_.get_total_time_ms());
    max_time = std::max(max_time, track_rot_y_.get_total_time_ms());
    max_time = std::max(max_time, track_opacity_.get_total_time_ms());
    max_time = std::max(max_time, track_blur_radius_.get_total_time_ms());
    max_time = std::max(max_time, track_motion_blur_.get_total_time_ms());
    max_time = std::max(max_time, track_mask_progress_.get_total_time_ms());
    max_time = std::max(max_time, track_text_progress_.get_total_time_ms());
    return max_time > 0.0 ? max_time : 500.0;
}

Transform2D ComposableAnimation::apply_to_transform2d(const Transform2D& base) const {
    Transform2D result;
    result.pos_x = base.pos_x + static_cast<float>(track_pos_x_.current_val);
    result.pos_y = base.pos_y + static_cast<float>(track_pos_y_.current_val);
    result.rot = base.rot + static_cast<float>(track_rot_z_.current_val);
    result.scale_x = base.scale_x * static_cast<float>(track_scale_x_.current_val);
    result.scale_y = base.scale_y * static_cast<float>(track_scale_y_.current_val);
    return result;
}

Transform3D ComposableAnimation::get_transform3d() const {
    Transform3D t3d;
    t3d.pos_z = static_cast<float>(track_pos_z_.current_val);
    t3d.rot_x = static_cast<float>(track_rot_x_.current_val);
    t3d.rot_y = static_cast<float>(track_rot_y_.current_val);
    t3d.rot_z = static_cast<float>(track_rot_z_.current_val);
    t3d.fov = 60.0f;
    return t3d;
}

EffectsState ComposableAnimation::get_effects_state() const {
    EffectsState eff;
    eff.opacity = static_cast<float>(track_opacity_.current_val);
    eff.blur_radius = static_cast<float>(track_blur_radius_.current_val);
    eff.motion_blur_samples = static_cast<float>(track_motion_blur_.current_val);
    eff.mask_progress = static_cast<float>(track_mask_progress_.current_val);
    eff.mask_type = this->mask_type;
    eff.mask_angle = this->mask_angle;
    eff.mask_feather = this->mask_feather;
    return eff;
}

void ComposableAnimation::configure_slide(double offset_x, double offset_y, TimeMs duration,
                                          const EasingConfig& easing) {
    track_pos_x_.enabled = (std::abs(offset_x) > 1e-4);
    if (track_pos_x_.enabled) {
        track_pos_x_.start_val = offset_x;
        track_pos_x_.end_val = 0.0;
        track_pos_x_.duration_ms = duration;
        track_pos_x_.easing = easing;
    }
    track_pos_y_.enabled = (std::abs(offset_y) > 1e-4);
    if (track_pos_y_.enabled) {
        track_pos_y_.start_val = offset_y;
        track_pos_y_.end_val = 0.0;
        track_pos_y_.duration_ms = duration;
        track_pos_y_.easing = easing;
    }
}

void ComposableAnimation::configure_fade(double start_alpha, double end_alpha, TimeMs duration,
                                         const EasingConfig& easing) {
    track_opacity_.enabled = true;
    track_opacity_.start_val = start_alpha;
    track_opacity_.end_val = end_alpha;
    track_opacity_.duration_ms = duration;
    track_opacity_.easing = easing;
}

void ComposableAnimation::configure_scale(double start_scale, double end_scale, TimeMs duration,
                                          const EasingConfig& easing) {
    track_scale_x_.enabled = true;
    track_scale_x_.start_val = start_scale;
    track_scale_x_.end_val = end_scale;
    track_scale_x_.duration_ms = duration;
    track_scale_x_.easing = easing;

    track_scale_y_.enabled = true;
    track_scale_y_.start_val = start_scale;
    track_scale_y_.end_val = end_scale;
    track_scale_y_.duration_ms = duration;
    track_scale_y_.easing = easing;
}

void ComposableAnimation::configure_rotation(double start_angle_deg, double end_angle_deg,
                                             TimeMs duration, const EasingConfig& easing) {
    track_rot_z_.enabled = true;
    track_rot_z_.start_val = start_angle_deg;
    track_rot_z_.end_val = end_angle_deg;
    track_rot_z_.duration_ms = duration;
    track_rot_z_.easing = easing;
}

void ComposableAnimation::configure_3d_flip(double start_rx, double start_ry,
                                            double end_rx, double end_ry,
                                            TimeMs duration, const EasingConfig& easing) {
    if (std::abs(start_rx - end_rx) > 1e-4) {
        track_rot_x_.enabled = true;
        track_rot_x_.start_val = start_rx;
        track_rot_x_.end_val = end_rx;
        track_rot_x_.duration_ms = duration;
        track_rot_x_.easing = easing;
    }
    if (std::abs(start_ry - end_ry) > 1e-4) {
        track_rot_y_.enabled = true;
        track_rot_y_.start_val = start_ry;
        track_rot_y_.end_val = end_ry;
        track_rot_y_.duration_ms = duration;
        track_rot_y_.easing = easing;
    }
}

void ComposableAnimation::configure_blur(double start_blur, double end_blur, TimeMs duration,
                                         const EasingConfig& easing) {
    track_blur_radius_.enabled = true;
    track_blur_radius_.start_val = start_blur;
    track_blur_radius_.end_val = end_blur;
    track_blur_radius_.duration_ms = duration;
    track_blur_radius_.easing = easing;
}

void ComposableAnimation::configure_mask(double start_p, double end_p, int type, float angle,
                                         TimeMs duration, const EasingConfig& easing) {
    mask_type = type;
    mask_angle = angle;
    mask_feather = 0.05f;
    track_mask_progress_.enabled = true;
    track_mask_progress_.start_val = start_p;
    track_mask_progress_.end_val = end_p;
    track_mask_progress_.duration_ms = duration;
    track_mask_progress_.easing = easing;
}

void ComposableAnimation::configure_text(int mode, TimeMs duration, const EasingConfig& easing) {
    text_mode = mode;
    mask_type = (mode == 1) ? 4 : 5;
    mask_feather = 0.04f;
    track_text_progress_.enabled = true;
    track_text_progress_.start_val = 0.0;
    track_text_progress_.end_val = 1.0;
    track_text_progress_.duration_ms = duration;
    track_text_progress_.easing = easing;
}

} // namespace motionfx
