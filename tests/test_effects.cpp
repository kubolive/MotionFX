#include "motionfx/motionfx_effects.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    using namespace motionfx;

    std::cout << "[TEST] Testing MotionFX Phase 3: Advanced Effects (Mask, Blur, Text)..." << std::endl;

    // Test 1: Mask Reveal
    MaskParams mp_linear;
    mp_linear.type = MaskType::LinearWipe;
    mp_linear.angle_deg = 0.0f; // Horizontal wipe left-to-right
    mp_linear.feather = 0.05f;

    mp_linear.progress = 0.0f;
    float a_start = calculate_mask_alpha(mp_linear, 0.5f, 0.5f);
    assert(a_start < 0.05f); // Fully hidden

    mp_linear.progress = 1.0f;
    float a_end = calculate_mask_alpha(mp_linear, 0.5f, 0.5f);
    assert(a_end > 0.95f); // Fully revealed

    // Radial circle mask
    MaskParams mp_radial;
    mp_radial.type = MaskType::RadialCircle;
    mp_radial.center_x = 0.5f;
    mp_radial.center_y = 0.5f;
    mp_radial.progress = 0.5f; // Halfway circle expanded

    float a_center = calculate_mask_alpha(mp_radial, 0.5f, 0.5f); // Center
    float a_corner = calculate_mask_alpha(mp_radial, 1.0f, 1.0f); // Distant corner
    assert(a_center > 0.95f);
    assert(a_corner < 0.05f);
    std::cout << "  -> Mask Reveal (Linear Wipe & Radial Circle) passed." << std::endl;

    // Test 2: Gaussian Blur Kernel
    BlurKernel kernel = generate_gaussian_kernel(10.0f);
    float sum_w = 0.0f;
    for (float w : kernel.weights) sum_w += w;
    assert(std::abs(sum_w - 1.0f) < 1e-4); // Weights must sum to 1.0
    assert(kernel.weights.size() > 10);
    // Symmetry check
    size_t n = kernel.weights.size();
    assert(std::abs(kernel.weights[0] - kernel.weights[n - 1]) < 1e-5);
    std::cout << "  -> Separable Gaussian Blur Kernel (sum=" << sum_w << ", taps=" << n << ") passed." << std::endl;

    // Test 3: Motion Blur Directional Vector
    MotionBlurParams mb_params;
    mb_params.velocity_x = 1.2f; // px/ms
    mb_params.velocity_y = 0.0f;
    mb_params.shutter_angle_deg = 180.0f; // 50% frame exposure
    MotionBlurVector mb_vec = calculate_motion_blur_vector(mb_params, 16.666);
    // Length = 1.2 * 16.666 * 0.5 = ~10.0 px
    assert(std::abs(mb_vec.length_px - 10.0f) < 0.5f);
    assert(std::abs(mb_vec.dir_x - 1.0f) < 1e-4);
    assert(std::abs(mb_vec.dir_y - 0.0f) < 1e-4);
    std::cout << "  -> Directional Motion Blur Vector (length=" << mb_vec.length_px << "px) passed." << std::endl;

    // Test 4: Text Animation
    TextAnimParams text_params;
    text_params.full_text = "Motion Graphics for OBS";
    text_params.mode = TextAnimationMode::Typewriter;

    text_params.progress = 0.0f;
    AnimatedTextResult r0 = evaluate_text_animation(text_params);
    assert(r0.displayed_text.empty());

    text_params.progress = 0.5f;
    AnimatedTextResult r_mid = evaluate_text_animation(text_params);
    assert(!r_mid.displayed_text.empty() && r_mid.displayed_text.length() < text_params.full_text.length());
    std::cout << "  -> Typewriter mid-text: \"" << r_mid.displayed_text << "\"" << std::endl;

    text_params.progress = 1.0f;
    AnimatedTextResult r_full = evaluate_text_animation(text_params);
    assert(r_full.displayed_text == "Motion Graphics for OBS");
    std::cout << "  -> Typewriter full-text: \"" << r_full.displayed_text << "\" passed." << std::endl;

    // Word stagger
    text_params.mode = TextAnimationMode::WordStagger;
    text_params.progress = 0.5f;
    AnimatedTextResult r_word = evaluate_text_animation(text_params);
    std::cout << "  -> Word Stagger at 50%: \"" << r_word.displayed_text << "\" passed." << std::endl;
    assert(r_word.displayed_text == "Motion Graphics");

    std::cout << "[SUCCESS] All Phase 3 Advanced Effects tests passed successfully!" << std::endl;
    return 0;
}
