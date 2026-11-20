#pragma once

#include <string>
#include <cmath>

namespace motionfx {

enum class EasingType {
    Linear,
    SineIn,
    SineOut,
    SineInOut,
    QuadIn,
    QuadOut,
    QuadInOut,
    CubicIn,
    CubicOut,
    CubicInOut,
    QuartIn,
    QuartOut,
    QuartInOut,
    QuintIn,
    QuintOut,
    QuintInOut,
    ExpoIn,
    ExpoOut,
    ExpoInOut,
    CircIn,
    CircOut,
    CircInOut,
    BackIn,
    BackOut,
    BackInOut,
    ElasticIn,
    ElasticOut,
    ElasticInOut,
    BounceIn,
    BounceOut,
    BounceInOut,
    CubicBezier
};

struct EasingConfig {
    EasingType type{EasingType::CubicOut};
    // Cubic Bézier control points (P0=(0,0) and P3=(1,1) are fixed)
    double p1x{0.25};
    double p1y{0.10};
    double p2x{0.25};
    double p2y{1.00};
    // Back easing overshoot constant (standard 1.70158)
    double back_overshoot{1.70158};
    // Elastic parameters
    double elastic_amplitude{1.0};
    double elastic_period{0.3};

    static EasingConfig linear() {
        EasingConfig c;
        c.type = EasingType::Linear;
        return c;
    }

    static EasingConfig ease_out() {
        EasingConfig c;
        c.type = EasingType::CubicOut;
        return c;
    }

    static EasingConfig ease_in() {
        EasingConfig c;
        c.type = EasingType::CubicIn;
        return c;
    }

    static EasingConfig ease_in_out() {
        EasingConfig c;
        c.type = EasingType::CubicInOut;
        return c;
    }

    static EasingConfig back_out(double s = 1.70158) {
        EasingConfig c;
        c.type = EasingType::BackOut;
        c.back_overshoot = s;
        return c;
    }

    static EasingConfig elastic_out(double a = 1.0, double p = 0.3) {
        EasingConfig c;
        c.type = EasingType::ElasticOut;
        c.elastic_amplitude = a;
        c.elastic_period = p;
        return c;
    }

    static EasingConfig bounce_out() {
        EasingConfig c;
        c.type = EasingType::BounceOut;
        return c;
    }

    static EasingConfig bezier(double p1x, double p1y, double p2x, double p2y) {
        EasingConfig c;
        c.type = EasingType::CubicBezier;
        c.p1x = p1x;
        c.p1y = p1y;
        c.p2x = p2x;
        c.p2y = p2y;
        return c;
    }
};

double evaluate_easing(const EasingConfig& config, double t);
double evaluate_cubic_bezier(double t, double p1x, double p1y, double p2x, double p2y);

EasingType easing_from_string(const std::string& name);
std::string easing_to_string(EasingType type);

} // namespace motionfx
