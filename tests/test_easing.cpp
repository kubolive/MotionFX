#include "motionfx/motionfx_easing.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    using namespace motionfx;

    std::cout << "[TEST] Testing MotionFX Easing Engine..." << std::endl;

    // Test 1: Linear
    auto lin = EasingConfig::linear();
    assert(std::abs(evaluate_easing(lin, 0.0) - 0.0) < 1e-6);
    assert(std::abs(evaluate_easing(lin, 0.5) - 0.5) < 1e-6);
    assert(std::abs(evaluate_easing(lin, 1.0) - 1.0) < 1e-6);
    std::cout << "  -> Linear passed." << std::endl;

    // Test 2: Ease In / Out / InOut (Cubic)
    auto c_out = EasingConfig::ease_out();
    assert(std::abs(evaluate_easing(c_out, 0.0) - 0.0) < 1e-6);
    assert(evaluate_easing(c_out, 0.5) > 0.5); // Ease out accelerates early
    assert(std::abs(evaluate_easing(c_out, 1.0) - 1.0) < 1e-6);
    std::cout << "  -> Cubic Ease Out passed." << std::endl;

    auto c_in = EasingConfig::ease_in();
    assert(std::abs(evaluate_easing(c_in, 0.0) - 0.0) < 1e-6);
    assert(evaluate_easing(c_in, 0.5) < 0.5); // Ease in accelerates late
    assert(std::abs(evaluate_easing(c_in, 1.0) - 1.0) < 1e-6);
    std::cout << "  -> Cubic Ease In passed." << std::endl;

    // Test 3: Back Out (overshoots > 1.0 before settling to 1.0)
    auto b_out = EasingConfig::back_out(1.70158);
    assert(std::abs(evaluate_easing(b_out, 0.0) - 0.0) < 1e-6);
    double mid_back = evaluate_easing(b_out, 0.8);
    assert(mid_back > 1.0); // Overshoots
    assert(std::abs(evaluate_easing(b_out, 1.0) - 1.0) < 1e-6);
    std::cout << "  -> Back Out overshoot (" << mid_back << ") passed." << std::endl;

    // Test 4: Bounce Out
    auto bnc = EasingConfig::bounce_out();
    assert(std::abs(evaluate_easing(bnc, 0.0) - 0.0) < 1e-6);
    assert(std::abs(evaluate_easing(bnc, 1.0) - 1.0) < 1e-6);
    std::cout << "  -> Bounce Out passed." << std::endl;

    // Test 5: Elastic Out
    auto el = EasingConfig::elastic_out();
    assert(std::abs(evaluate_easing(el, 0.0) - 0.0) < 1e-6);
    assert(std::abs(evaluate_easing(el, 1.0) - 1.0) < 1e-6);
    std::cout << "  -> Elastic Out passed." << std::endl;

    // Test 6: Custom Cubic Bézier
    auto bez = EasingConfig::bezier(0.25, 0.1, 0.25, 1.0); // Standard ease
    assert(std::abs(evaluate_easing(bez, 0.0) - 0.0) < 1e-6);
    assert(std::abs(evaluate_easing(bez, 1.0) - 1.0) < 1e-6);
    double b_val = evaluate_easing(bez, 0.5);
    std::cout << "  -> Custom Cubic Bezier at 0.5: " << b_val << std::endl;
    assert(b_val > 0.0 && b_val < 1.0);

    // Test 7: String conversions
    assert(easing_from_string("back_out") == EasingType::BackOut);
    assert(easing_from_string("elastic_out") == EasingType::ElasticOut);
    assert(easing_to_string(EasingType::BounceOut) == "bounce_out");
    std::cout << "  -> String serialization passed." << std::endl;

    std::cout << "[SUCCESS] All Easing Engine tests passed successfully!" << std::endl;
    return 0;
}
