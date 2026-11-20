#include "motionfx/motionfx_tracks.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    using namespace motionfx;

    std::cout << "[TEST] Testing MotionFX Composable Animation..." << std::endl;

    ComposableAnimation comp;
    // Scenario 1: Slide In (-400px X) with Ease Out, Fade In (0.0 to 1.0) with Linear
    comp.configure_slide(-400.0, 0.0, 500.0, EasingConfig::ease_out());
    comp.configure_fade(0.0, 1.0, 300.0, EasingConfig::linear());

    Transform2D base{1920.0f / 2.0f, 1080.0f / 2.0f, 0.0f, 1.0f, 1.0f};

    // At t = 0ms
    comp.evaluate(0.0);
    Transform2D t0 = comp.apply_to_transform2d(base);
    EffectsState e0 = comp.get_effects_state();

    assert(std::abs(t0.pos_x - (base.pos_x - 400.0f)) < 1e-4);
    assert(std::abs(e0.opacity - 0.0f) < 1e-4);
    std::cout << "  -> t = 0ms: PosX=" << t0.pos_x << " (offset -400), Opacity=" << e0.opacity << " passed." << std::endl;

    // At t = 300ms: Fade is completed (duration 300ms), Slide is at 300/500ms
    comp.evaluate(300.0);
    Transform2D t300 = comp.apply_to_transform2d(base);
    EffectsState e300 = comp.get_effects_state();

    assert(std::abs(e300.opacity - 1.0f) < 1e-4); // Fade finished!
    assert(t300.pos_x > (base.pos_x - 400.0f) && t300.pos_x < base.pos_x);
    std::cout << "  -> t = 300ms: Fade completed (opacity 1.0), Slide in-progress (PosX=" << t300.pos_x << ") passed." << std::endl;

    // At t = 500ms: Both completed
    comp.evaluate(500.0);
    Transform2D t500 = comp.apply_to_transform2d(base);
    EffectsState e500 = comp.get_effects_state();

    assert(std::abs(t500.pos_x - base.pos_x) < 1e-4);
    assert(std::abs(e500.opacity - 1.0f) < 1e-4);
    std::cout << "  -> t = 500ms: Both completed at base transform passed." << std::endl;

    // Scenario 2: Disable Fade! Requirement: Slide MUST still work normally!
    comp.opacity().enabled = false;
    comp.evaluate(0.0);
    EffectsState e_no_fade = comp.get_effects_state();
    Transform2D t_no_fade = comp.apply_to_transform2d(base);

    assert(std::abs(e_no_fade.opacity - 1.0f) < 1e-4); // Default opacity is 1.0 when disabled!
    assert(std::abs(t_no_fade.pos_x - (base.pos_x - 400.0f)) < 1e-4); // Slide still works!
    std::cout << "  -> Independent component disable: Fade disabled (opacity=1.0) while Slide operates passed." << std::endl;

    // Scenario 3: Multi-track with different easings (X = EaseOut, Y = BackOut, Scale = ElasticOut)
    ComposableAnimation comp_multi;
    comp_multi.pos_x().enabled = true;
    comp_multi.pos_x().start_val = -200.0;
    comp_multi.pos_x().end_val = 0.0;
    comp_multi.pos_x().duration_ms = 400.0;
    comp_multi.pos_x().easing = EasingConfig::ease_out();

    comp_multi.pos_y().enabled = true;
    comp_multi.pos_y().start_val = 200.0;
    comp_multi.pos_y().end_val = 0.0;
    comp_multi.pos_y().duration_ms = 400.0;
    comp_multi.pos_y().easing = EasingConfig::back_out(1.70158);

    comp_multi.scale_x().enabled = true;
    comp_multi.scale_x().start_val = 0.2;
    comp_multi.scale_x().end_val = 1.0;
    comp_multi.scale_x().duration_ms = 600.0;
    comp_multi.scale_x().easing = EasingConfig::elastic_out();

    comp_multi.evaluate(350.0);
    // BackOut on Y overshoots (passes 0.0 into negative before settling)
    double val_y = comp_multi.pos_y().current_val;
    std::cout << "  -> Multi-easing at 350ms: PosX=" << comp_multi.pos_x().current_val
              << ", PosY=" << val_y << " (BackOut overshoot), ScaleX="
              << comp_multi.scale_x().current_val << std::endl;

    std::cout << "[SUCCESS] All Composable Animation tests passed successfully!" << std::endl;
    return 0;
}
