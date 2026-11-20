#include "motionfx/motionfx_effects.h"
#include <algorithm>
#include <sstream>

namespace motionfx {

#ifndef M_PI
constexpr float M_PI = 3.14159265358979323846f;
#endif

namespace {

inline float smoothstep(float edge0, float edge1, float x) {
    if (edge0 >= edge1) return x >= edge1 ? 1.0f : 0.0f;
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // anonymous namespace

float calculate_mask_alpha(const MaskParams& params, float u, float v) {
    if (params.type == MaskType::None) {
        return 1.0f;
    }

    float alpha = 1.0f;
    float feather = std::max(0.001f, params.feather);
    float p = std::clamp(params.progress, 0.0f, 1.0f);

    switch (params.type) {
        case MaskType::LinearWipe: {
            float rad = params.angle_deg * (M_PI / 180.0f);
            float dir_x = std::cos(rad);
            float dir_y = std::sin(rad);

            // Project UV onto wipe direction vector
            // Center is (0.5, 0.5)
            float proj = (u - 0.5f) * dir_x + (v - 0.5f) * dir_y;
            // Normalise projection range from [-0.707, +0.707] to [0.0, 1.0]
            float norm_proj = (proj / 1.4142f) + 0.5f;

            // Wipe boundary moves from 0.0 to 1.0
            float threshold = p;
            alpha = smoothstep(norm_proj - feather, norm_proj + feather, threshold);
            break;
        }

        case MaskType::RadialCircle: {
            float dx = u - params.center_x;
            float dy = v - params.center_y;
            float dist = std::sqrt(dx * dx + dy * dy);
            // Max distance to corner is ~0.7071
            float norm_dist = dist / 0.7071f;

            // Reveal from center outwards
            alpha = 1.0f - smoothstep(p - feather, p, norm_dist);
            break;
        }

        case MaskType::BoxWipe: {
            float dx = std::abs(u - params.center_x);
            float dy = std::abs(v - params.center_y);
            float max_d = std::max(dx, dy) / 0.5f; // [0.0, 1.0]

            alpha = 1.0f - smoothstep(p - feather, p, max_d);
            break;
        }

        case MaskType::None:
            alpha = 1.0f;
            break;
    }

    if (params.invert) {
        alpha = 1.0f - alpha;
    }
    return std::clamp(alpha, 0.0f, 1.0f);
}

BlurKernel generate_gaussian_kernel(float radius_px) {
    BlurKernel kernel;
    if (radius_px <= 0.1f) {
        kernel.weights.push_back(1.0f);
        kernel.offsets.push_back(0.0f);
        return kernel;
    }

    int r = static_cast<int>(std::ceil(radius_px));
    if (r > 32) r = 32; // Limit to 32 taps for realtime GPU efficiency

    float sigma = std::max(0.5f, radius_px / 2.5f);
    float sum = 0.0f;

    for (int i = -r; i <= r; ++i) {
        float offset = static_cast<float>(i);
        float weight = (1.0f / (std::sqrt(2.0f * M_PI) * sigma)) *
                       std::exp(-(offset * offset) / (2.0f * sigma * sigma));
        kernel.weights.push_back(weight);
        kernel.offsets.push_back(offset);
        sum += weight;
    }

    // Normalize weights
    if (sum > 0.0f) {
        for (float& w : kernel.weights) {
            w /= sum;
        }
    }

    return kernel;
}

MotionBlurVector calculate_motion_blur_vector(const MotionBlurParams& params, TimeMs frame_duration_ms) {
    MotionBlurVector vec{};
    float shutter_fraction = std::clamp(params.shutter_angle_deg / 360.0f, 0.0f, 1.0f);

    float total_dx = params.velocity_x * static_cast<float>(frame_duration_ms) * shutter_fraction;
    float total_dy = params.velocity_y * static_cast<float>(frame_duration_ms) * shutter_fraction;

    float len = std::sqrt(total_dx * total_dx + total_dy * total_dy);
    vec.length_px = len;

    if (len > 1e-4f) {
        vec.dir_x = total_dx / len;
        vec.dir_y = total_dy / len;
    }
    return vec;
}

AnimatedTextResult evaluate_text_animation(const TextAnimParams& params) {
    AnimatedTextResult res{};
    const std::string& full = params.full_text;
    if (full.empty()) return res;

    float p = std::clamp(params.progress, 0.0f, 1.0f);

    switch (params.mode) {
        case TextAnimationMode::Typewriter: {
            size_t total_chars = full.length();
            res.total_units = static_cast<int>(total_chars);
            size_t visible_chars = static_cast<size_t>(p * total_chars);
            if (visible_chars > total_chars) visible_chars = total_chars;

            res.visible_units = static_cast<int>(visible_chars);
            res.displayed_text = full.substr(0, visible_chars);
            res.current_unit_alpha = 1.0f;
            break;
        }

        case TextAnimationMode::WordStagger: {
            std::istringstream iss(full);
            std::string word;
            std::vector<std::string> words;
            while (iss >> word) {
                words.push_back(word);
            }

            res.total_units = static_cast<int>(words.size());
            size_t visible_words = static_cast<size_t>(p * words.size());
            if (visible_words > words.size()) visible_words = words.size();

            res.visible_units = static_cast<int>(visible_words);
            std::string out;
            for (size_t i = 0; i < visible_words; ++i) {
                if (i > 0) out += " ";
                out += words[i];
            }
            res.displayed_text = out;
            break;
        }

        case TextAnimationMode::LineReveal: {
            std::istringstream iss(full);
            std::string line;
            std::vector<std::string> lines;
            while (std::getline(iss, line)) {
                lines.push_back(line);
            }

            res.total_units = static_cast<int>(lines.size());
            size_t visible_lines = static_cast<size_t>(p * lines.size());
            if (visible_lines > lines.size()) visible_lines = lines.size();

            res.visible_units = static_cast<int>(visible_lines);
            std::string out;
            for (size_t i = 0; i < visible_lines; ++i) {
                if (i > 0) out += "\n";
                out += lines[i];
            }
            res.displayed_text = out;
            break;
        }

        case TextAnimationMode::Tracking: {
            // Tracking animation inserts or spaces characters dynamically
            res.total_units = static_cast<int>(full.length());
            res.visible_units = res.total_units;

            // Extra space between letters interpolates from 0 up to tracking_extra_spaces
            int current_spaces = static_cast<int>(p * params.tracking_extra_spaces);
            std::string pad(current_spaces, ' ');

            std::string out;
            for (size_t i = 0; i < full.length(); ++i) {
                out += full[i];
                if (i + 1 < full.length() && full[i] != ' ' && full[i] != '\n') {
                    out += pad;
                }
            }
            res.displayed_text = out;
            break;
        }

        case TextAnimationMode::CharacterStagger: {
            // Character stagger reveals characters with progressive fade/blur
            size_t total_chars = full.length();
            res.total_units = static_cast<int>(total_chars);

            float char_idx_float = p * static_cast<float>(total_chars);
            size_t fully_visible = static_cast<size_t>(char_idx_float);
            if (fully_visible > total_chars) fully_visible = total_chars;

            res.visible_units = static_cast<int>(fully_visible);
            res.displayed_text = full.substr(0, fully_visible);
            res.current_unit_alpha = char_idx_float - static_cast<float>(fully_visible);
            break;
        }

        case TextAnimationMode::None:
            res.displayed_text = full;
            res.total_units = static_cast<int>(full.length());
            res.visible_units = res.total_units;
            break;
    }

    return res;
}

} // namespace motionfx
