#pragma once

#include "motionfx/motionfx_core.h"
#include <string>
#include <vector>
#include <cmath>

namespace motionfx {

// ============================================================================
// Mask Reveal
// ============================================================================
enum class MaskType {
    None = 0,
    LinearWipe = 1,
    RadialCircle = 2,
    BoxWipe = 3
};

struct MaskParams {
    MaskType type{MaskType::LinearWipe};
    float progress{1.0f};   // 0.0 (fully hidden) to 1.0 (fully revealed)
    float angle_deg{0.0f};  // Direction of wipe in degrees
    float feather{0.05f};   // Edge softness
    float center_x{0.5f};   // For radial and box wipes
    float center_y{0.5f};
    bool invert{false};
};

float calculate_mask_alpha(const MaskParams& params, float u, float v);

// ============================================================================
// Blur & Motion Blur
// ============================================================================
struct BlurKernel {
    std::vector<float> weights;
    std::vector<float> offsets;
};

// Generates 1D separable Gaussian kernel weights and offsets for GPU sampling
BlurKernel generate_gaussian_kernel(float radius_px);

struct MotionBlurParams {
    float velocity_x{0.0f}; // Pixels per ms
    float velocity_y{0.0f};
    float shutter_angle_deg{180.0f}; // Standard 180° camera shutter
    int samples{8};
};

struct MotionBlurVector {
    float dir_x{0.0f};
    float dir_y{0.0f};
    float length_px{0.0f};
};

MotionBlurVector calculate_motion_blur_vector(const MotionBlurParams& params, TimeMs frame_duration_ms = 16.666);

// ============================================================================
// Text Animation
// ============================================================================
enum class TextAnimationMode {
    None = 0,
    Typewriter = 1,
    CharacterStagger = 2,
    WordStagger = 3,
    LineReveal = 4,
    Tracking = 5
};

struct TextAnimParams {
    TextAnimationMode mode{TextAnimationMode::Typewriter};
    std::string full_text;
    float progress{1.0f}; // 0.0 to 1.0
    TimeMs stagger_delay_ms{50.0};
    int tracking_extra_spaces{0};
};

struct AnimatedTextResult {
    std::string displayed_text;
    int visible_units{0};
    int total_units{0};
    float current_unit_alpha{1.0f};
    float current_unit_blur{0.0f};
};

AnimatedTextResult evaluate_text_animation(const TextAnimParams& params);

} // namespace motionfx
