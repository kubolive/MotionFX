#include "motionfx/motionfx_easing.h"
#include <algorithm>
#include <numbers>

namespace motionfx {

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

namespace {

inline double bounce_out_impl(double t) {
    constexpr double n1 = 7.5625;
    constexpr double d1 = 2.75;

    if (t < 1.0 / d1) {
        return n1 * t * t;
    } else if (t < 2.0 / d1) {
        t -= 1.5 / d1;
        return n1 * t * t + 0.75;
    } else if (t < 2.5 / d1) {
        t -= 2.25 / d1;
        return n1 * t * t + 0.9375;
    } else {
        t -= 2.625 / d1;
        return n1 * t * t + 0.984375;
    }
}

inline double bounce_in_impl(double t) {
    return 1.0 - bounce_out_impl(1.0 - t);
}

inline double bounce_in_out_impl(double t) {
    if (t < 0.5) {
        return 0.5 * bounce_in_impl(t * 2.0);
    } else {
        return 0.5 * bounce_out_impl(t * 2.0 - 1.0) + 0.5;
    }
}

inline double elastic_in_impl(double t, double a, double p) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    double s = p / (2.0 * M_PI) * std::asin(std::clamp(1.0 / a, -1.0, 1.0));
    t -= 1.0;
    return -(a * std::pow(2.0, 10.0 * t) * std::sin((t - s) * (2.0 * M_PI) / p));
}

inline double elastic_out_impl(double t, double a, double p) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    double s = p / (2.0 * M_PI) * std::asin(std::clamp(1.0 / a, -1.0, 1.0));
    return a * std::pow(2.0, -10.0 * t) * std::sin((t - s) * (2.0 * M_PI) / p) + 1.0;
}

inline double elastic_in_out_impl(double t, double a, double p) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;
    double s = p / (2.0 * M_PI) * std::asin(std::clamp(1.0 / a, -1.0, 1.0));
    t = t * 2.0 - 1.0;
    if (t < 0.0) {
        return -0.5 * (a * std::pow(2.0, 10.0 * t) * std::sin((t - s) * (2.0 * M_PI) / p));
    }
    return 0.5 * (a * std::pow(2.0, -10.0 * t) * std::sin((t - s) * (2.0 * M_PI) / p)) + 1.0;
}

} // anonymous namespace

double evaluate_cubic_bezier(double target_x, double p1x, double p1y, double p2x, double p2y) {
    if (target_x <= 0.0) return 0.0;
    if (target_x >= 1.0) return 1.0;

    // Polynomial coefficients for x(u) and y(u)
    // x(u) = 3(1-u)^2 u * p1x + 3(1-u) u^2 * p2x + u^3
    //      = u * (3 * p1x) + u^2 * (3 * p2x - 6 * p1x) + u^3 * (1 - 3 * p2x + 3 * p1x)
    const double cx = 3.0 * p1x;
    const double bx = 3.0 * (p2x - p1x) - cx;
    const double ax = 1.0 - cx - bx;

    const double cy = 3.0 * p1y;
    const double by = 3.0 * (p2y - p1y) - cy;
    const double ay = 1.0 - cy - by;

    // Solve x(u) = target_x using Newton-Raphson
    double u = target_x; // initial guess
    for (int i = 0; i < 8; ++i) {
        double current_x = ((ax * u + bx) * u + cx) * u;
        double dx = (3.0 * ax * u + 2.0 * bx) * u + cx;
        if (std::abs(dx) < 1e-6) break;
        double error = current_x - target_x;
        if (std::abs(error) < 1e-7) break;
        u -= error / dx;
        u = std::clamp(u, 0.0, 1.0);
    }

    // Binary search refinement if needed
    if (std::abs(((ax * u + bx) * u + cx) * u - target_x) > 1e-4) {
        double low = 0.0, high = 1.0;
        u = target_x;
        for (int i = 0; i < 16; ++i) {
            double current_x = ((ax * u + bx) * u + cx) * u;
            if (std::abs(current_x - target_x) < 1e-5) break;
            if (current_x < target_x) low = u;
            else high = u;
            u = 0.5 * (low + high);
        }
    }

    // Evaluate y(u)
    return ((ay * u + by) * u + cy) * u;
}

double evaluate_easing(const EasingConfig& config, double t) {
    if (t <= 0.0) return 0.0;
    if (t >= 1.0) return 1.0;

    const double s = config.back_overshoot;
    const double a = config.elastic_amplitude;
    const double p = config.elastic_period;

    switch (config.type) {
        case EasingType::Linear:
            return t;

        // Sine
        case EasingType::SineIn:
            return 1.0 - std::cos((t * M_PI) / 2.0);
        case EasingType::SineOut:
            return std::sin((t * M_PI) / 2.0);
        case EasingType::SineInOut:
            return -0.5 * (std::cos(M_PI * t) - 1.0);

        // Quad
        case EasingType::QuadIn:
            return t * t;
        case EasingType::QuadOut:
            return 1.0 - (1.0 - t) * (1.0 - t);
        case EasingType::QuadInOut:
            return t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;

        // Cubic
        case EasingType::CubicIn:
            return t * t * t;
        case EasingType::CubicOut:
            return 1.0 - std::pow(1.0 - t, 3.0);
        case EasingType::CubicInOut:
            return t < 0.5 ? 4.0 * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) / 2.0;

        // Quart
        case EasingType::QuartIn:
            return t * t * t * t;
        case EasingType::QuartOut:
            return 1.0 - std::pow(1.0 - t, 4.0);
        case EasingType::QuartInOut:
            return t < 0.5 ? 8.0 * t * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 4.0) / 2.0;

        // Quint
        case EasingType::QuintIn:
            return t * t * t * t * t;
        case EasingType::QuintOut:
            return 1.0 - std::pow(1.0 - t, 5.0);
        case EasingType::QuintInOut:
            return t < 0.5 ? 16.0 * t * t * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 5.0) / 2.0;

        // Expo
        case EasingType::ExpoIn:
            return t == 0.0 ? 0.0 : std::pow(2.0, 10.0 * t - 10.0);
        case EasingType::ExpoOut:
            return t == 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t);
        case EasingType::ExpoInOut:
            if (t == 0.0) return 0.0;
            if (t == 1.0) return 1.0;
            return t < 0.5 ? std::pow(2.0, 20.0 * t - 10.0) / 2.0
                           : (2.0 - std::pow(2.0, -20.0 * t + 10.0)) / 2.0;

        // Circ
        case EasingType::CircIn:
            return 1.0 - std::sqrt(std::max(0.0, 1.0 - t * t));
        case EasingType::CircOut:
            return std::sqrt(std::max(0.0, 1.0 - (t - 1.0) * (t - 1.0)));
        case EasingType::CircInOut:
            return t < 0.5
                ? (1.0 - std::sqrt(std::max(0.0, 1.0 - std::pow(2.0 * t, 2.0)))) / 2.0
                : (std::sqrt(std::max(0.0, 1.0 - std::pow(-2.0 * t + 2.0, 2.0))) + 1.0) / 2.0;

        // Back
        case EasingType::BackIn:
            return (s + 1.0) * t * t * t - s * t * t;
        case EasingType::BackOut: {
            double tm1 = t - 1.0;
            return (s + 1.0) * tm1 * tm1 * tm1 + s * tm1 * tm1 + 1.0;
        }
        case EasingType::BackInOut: {
            double c2 = s * 1.525;
            return t < 0.5
                ? (std::pow(2.0 * t, 2.0) * ((c2 + 1.0) * 2.0 * t - c2)) / 2.0
                : (std::pow(2.0 * t - 2.0, 2.0) * ((c2 + 1.0) * (t * 2.0 - 2.0) + c2) + 2.0) / 2.0;
        }

        // Elastic
        case EasingType::ElasticIn:
            return elastic_in_impl(t, a, p);
        case EasingType::ElasticOut:
            return elastic_out_impl(t, a, p);
        case EasingType::ElasticInOut:
            return elastic_in_out_impl(t, a, p);

        // Bounce
        case EasingType::BounceIn:
            return bounce_in_impl(t);
        case EasingType::BounceOut:
            return bounce_out_impl(t);
        case EasingType::BounceInOut:
            return bounce_in_out_impl(t);

        // Custom Cubic Bézier
        case EasingType::CubicBezier:
            return evaluate_cubic_bezier(t, config.p1x, config.p1y, config.p2x, config.p2y);
    }

    return t;
}

EasingType easing_from_string(const std::string& name) {
    if (name == "linear" || name == "Linear") return EasingType::Linear;
    if (name == "ease_in" || name == "EaseIn" || name == "cubic_in") return EasingType::CubicIn;
    if (name == "ease_out" || name == "EaseOut" || name == "cubic_out") return EasingType::CubicOut;
    if (name == "ease_in_out" || name == "EaseInOut" || name == "cubic_in_out") return EasingType::CubicInOut;

    if (name == "sine_in" || name == "SineIn") return EasingType::SineIn;
    if (name == "sine_out" || name == "SineOut") return EasingType::SineOut;
    if (name == "sine_in_out" || name == "SineInOut") return EasingType::SineInOut;

    if (name == "quad_in" || name == "QuadIn") return EasingType::QuadIn;
    if (name == "quad_out" || name == "QuadOut") return EasingType::QuadOut;
    if (name == "quad_in_out" || name == "QuadInOut") return EasingType::QuadInOut;

    if (name == "quart_in" || name == "QuartIn") return EasingType::QuartIn;
    if (name == "quart_out" || name == "QuartOut") return EasingType::QuartOut;
    if (name == "quart_in_out" || name == "QuartInOut") return EasingType::QuartInOut;

    if (name == "quint_in" || name == "QuintIn") return EasingType::QuintIn;
    if (name == "quint_out" || name == "QuintOut") return EasingType::QuintOut;
    if (name == "quint_in_out" || name == "QuintInOut") return EasingType::QuintInOut;

    if (name == "expo_in" || name == "ExpoIn") return EasingType::ExpoIn;
    if (name == "expo_out" || name == "ExpoOut") return EasingType::ExpoOut;
    if (name == "expo_in_out" || name == "ExpoInOut") return EasingType::ExpoInOut;

    if (name == "circ_in" || name == "CircIn") return EasingType::CircIn;
    if (name == "circ_out" || name == "CircOut") return EasingType::CircOut;
    if (name == "circ_in_out" || name == "CircInOut") return EasingType::CircInOut;

    if (name == "back_in" || name == "BackIn") return EasingType::BackIn;
    if (name == "back_out" || name == "BackOut" || name == "back") return EasingType::BackOut;
    if (name == "back_in_out" || name == "BackInOut") return EasingType::BackInOut;

    if (name == "elastic_in" || name == "ElasticIn") return EasingType::ElasticIn;
    if (name == "elastic_out" || name == "ElasticOut" || name == "elastic") return EasingType::ElasticOut;
    if (name == "elastic_in_out" || name == "ElasticInOut") return EasingType::ElasticInOut;

    if (name == "bounce_in" || name == "BounceIn") return EasingType::BounceIn;
    if (name == "bounce_out" || name == "BounceOut" || name == "bounce") return EasingType::BounceOut;
    if (name == "bounce_in_out" || name == "BounceInOut") return EasingType::BounceInOut;

    if (name == "cubic_bezier" || name == "bezier" || name == "CubicBezier") return EasingType::CubicBezier;

    return EasingType::CubicOut;
}

std::string easing_to_string(EasingType type) {
    switch (type) {
        case EasingType::Linear: return "linear";
        case EasingType::SineIn: return "sine_in";
        case EasingType::SineOut: return "sine_out";
        case EasingType::SineInOut: return "sine_in_out";
        case EasingType::QuadIn: return "quad_in";
        case EasingType::QuadOut: return "quad_out";
        case EasingType::QuadInOut: return "quad_in_out";
        case EasingType::CubicIn: return "cubic_in";
        case EasingType::CubicOut: return "cubic_out";
        case EasingType::CubicInOut: return "cubic_in_out";
        case EasingType::QuartIn: return "quart_in";
        case EasingType::QuartOut: return "quart_out";
        case EasingType::QuartInOut: return "quart_in_out";
        case EasingType::QuintIn: return "quint_in";
        case EasingType::QuintOut: return "quint_out";
        case EasingType::QuintInOut: return "quint_in_out";
        case EasingType::ExpoIn: return "expo_in";
        case EasingType::ExpoOut: return "expo_out";
        case EasingType::ExpoInOut: return "expo_in_out";
        case EasingType::CircIn: return "circ_in";
        case EasingType::CircOut: return "circ_out";
        case EasingType::CircInOut: return "circ_in_out";
        case EasingType::BackIn: return "back_in";
        case EasingType::BackOut: return "back_out";
        case EasingType::BackInOut: return "back_in_out";
        case EasingType::ElasticIn: return "elastic_in";
        case EasingType::ElasticOut: return "elastic_out";
        case EasingType::ElasticInOut: return "elastic_in_out";
        case EasingType::BounceIn: return "bounce_in";
        case EasingType::BounceOut: return "bounce_out";
        case EasingType::BounceInOut: return "bounce_in_out";
        case EasingType::CubicBezier: return "cubic_bezier";
    }
    return "cubic_out";
}

} // namespace motionfx
