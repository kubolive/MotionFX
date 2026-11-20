#include "motionfx/motionfx_core.h"
#include "motionfx/motionfx_engine.h"
#include "motionfx/motionfx_dispatcher.h"
#include "motionfx/motionfx_presets.h"
#include "motionfx/motionfx_3d.h"
#include "motionfx/motionfx_effects.h"

#ifdef OBS_PLUGIN_BUILD
#include <obs-module.h>
#include <graphics/graphics.h>
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("motionfx", "en-US")

MODULE_EXPORT const char *obs_module_description(void)
{
    return "MotionFX: Advanced Animation Engine for OBS Studio (Scene Item Show/Hide Transitions)";
}

MODULE_EXPORT const char *obs_module_name(void)
{
    return "MotionFX Animation Engine";
}

// ============================================================================
// Data Structure for MotionFX Scene Item Transition Source
// ============================================================================
struct motionfx_tr_data {
    obs_source_t *source{nullptr};

    gs_effect_t *effect_3d{nullptr};
    gs_eparam_t *image_param{nullptr};
    gs_eparam_t *opacity_param{nullptr};
    gs_eparam_t *color_param{nullptr};
    gs_eparam_t *blur_radius_param{nullptr};
    gs_eparam_t *mask_type_param{nullptr};
    gs_eparam_t *mask_progress_param{nullptr};
    gs_eparam_t *mask_angle_param{nullptr};
    gs_eparam_t *mask_feather_param{nullptr};

    gs_effect_t *effect_blur{nullptr};
    gs_effect_t *effect_mask{nullptr};

    // Composable animation configuration
    motionfx::ComposableAnimation comp;
    std::string preset_name{"3d_flip_x"};
    std::string transition_mode{"auto"}; // "auto", "show", "hide"
    std::string anchor_point{"center"};  // "center", "top", "bottom", "left", "right"
    uint32_t duration_ms{500};
};

static const char *motionfx_tr_get_name(void *unused)
{
    UNUSED_PARAMETER(unused);
    return "MotionFX Transition";
}

static void motionfx_tr_get_defaults(obs_data_t *settings)
{
    obs_data_set_default_string(settings, "preset", "3d_flip_x");
    obs_data_set_default_string(settings, "transition_mode", "auto");
    obs_data_set_default_string(settings, "anchor_point", "center");
    obs_data_set_default_bool(settings, "custom_override", false);
    obs_data_set_default_string(settings, "easing", "ease_out");
    obs_data_set_default_bool(settings, "enable_slide", false);
    obs_data_set_default_double(settings, "slide_x", 0.0);
    obs_data_set_default_double(settings, "slide_y", 0.0);
    obs_data_set_default_double(settings, "slide_z", 0.0);
    obs_data_set_default_bool(settings, "enable_fade", false);
    obs_data_set_default_double(settings, "fade_start", 0.0);
    obs_data_set_default_double(settings, "fade_end", 1.0);
    obs_data_set_default_bool(settings, "enable_scale", false);
    obs_data_set_default_double(settings, "scale_start", 0.0);
    obs_data_set_default_double(settings, "scale_end", 1.0);
    obs_data_set_default_bool(settings, "enable_3d", false);
    obs_data_set_default_double(settings, "rot_x", 90.0);
    obs_data_set_default_double(settings, "rot_y", 0.0);
    obs_data_set_default_bool(settings, "enable_rotation", false);
    obs_data_set_default_double(settings, "rot_z", 0.0);

    // Mask / Wipe defaults
    obs_data_set_default_bool(settings, "enable_mask", false);
    obs_data_set_default_int(settings, "mask_type", 1);
    obs_data_set_default_double(settings, "mask_angle", 45.0);
    obs_data_set_default_double(settings, "mask_feather", 0.05);

    // Blur defaults
    obs_data_set_default_bool(settings, "enable_blur", false);
    obs_data_set_default_double(settings, "blur_radius", 12.0);
}

static bool is_mask_or_text_preset(const std::string& name) {
    return (name == "mask_reveal" || name == "mask_radial" || name == "mask_box" ||
            name == "text_reveal" || name == "character_stagger");
}

static void motionfx_tr_update(void *data, obs_data_t *settings)
{
    auto *tr = static_cast<motionfx_tr_data *>(data);
    if (!tr || !settings) return;

    const char *preset = obs_data_get_string(settings, "preset");
    if (preset && strlen(preset) > 0) {
        tr->preset_name = preset;
        motionfx::PresetManager::instance().apply_preset(tr->comp, tr->preset_name);
    }

    const char *mode = obs_data_get_string(settings, "transition_mode");
    if (mode && strlen(mode) > 0) {
        tr->transition_mode = mode;
    } else {
        const char *legacy = obs_data_get_string(settings, "target_type");
        if (legacy && strlen(legacy) > 0) {
            tr->transition_mode = legacy;
        }
    }

    const char *anchor = obs_data_get_string(settings, "anchor_point");
    if (anchor && strlen(anchor) > 0) {
        tr->anchor_point = anchor;
    }

    // Manual component overrides if user customized them
    bool custom_override = obs_data_get_bool(settings, "custom_override");
    if (custom_override) {
        bool enable_slide = obs_data_get_bool(settings, "enable_slide");
        double slide_x = obs_data_get_double(settings, "slide_x");
        double slide_y = obs_data_get_double(settings, "slide_y");
        double slide_z = obs_data_get_double(settings, "slide_z");

        bool enable_fade = obs_data_get_bool(settings, "enable_fade");
        double fade_start = obs_data_get_double(settings, "fade_start");
        double fade_end = obs_data_get_double(settings, "fade_end");

        bool enable_scale = obs_data_get_bool(settings, "enable_scale");
        double scale_start = obs_data_get_double(settings, "scale_start");
        double scale_end = obs_data_get_double(settings, "scale_end");

        bool enable_rot = obs_data_get_bool(settings, "enable_rotation");
        double rot_z = obs_data_get_double(settings, "rot_z");

        bool enable_3d = obs_data_get_bool(settings, "enable_3d");
        double rot_x = obs_data_get_double(settings, "rot_x");
        double rot_y = obs_data_get_double(settings, "rot_y");

        bool enable_mask = obs_data_get_bool(settings, "enable_mask");
        long long mask_type_val = obs_data_get_int(settings, "mask_type");
        double mask_angle_val = obs_data_get_double(settings, "mask_angle");
        double mask_feather_val = obs_data_get_double(settings, "mask_feather");

        bool enable_blur = obs_data_get_bool(settings, "enable_blur");
        double blur_radius_val = obs_data_get_double(settings, "blur_radius");

        const char *easing_str = obs_data_get_string(settings, "easing");
        motionfx::EasingConfig ec;
        if (easing_str && strlen(easing_str) > 0) {
            ec.type = motionfx::easing_from_string(easing_str);
        }

        if (enable_slide) {
            tr->comp.configure_slide(slide_x, slide_y, tr->duration_ms, ec);
            if (std::abs(slide_z) > 1e-4) {
                tr->comp.pos_z().enabled = true;
                tr->comp.pos_z().start_val = slide_z;
                tr->comp.pos_z().end_val = 0.0;
                tr->comp.pos_z().duration_ms = tr->duration_ms;
                tr->comp.pos_z().easing = ec;
            }
        } else if (!tr->comp.pos_x().enabled) {
            tr->comp.pos_x().enabled = false;
            tr->comp.pos_y().enabled = false;
            tr->comp.pos_z().enabled = false;
        }

        if (enable_fade) {
            tr->comp.configure_fade(fade_start, fade_end, tr->duration_ms, ec);
        } else if (!tr->comp.opacity().enabled) {
            tr->comp.opacity().enabled = false;
        }

        if (enable_scale) {
            tr->comp.configure_scale(scale_start, scale_end, tr->duration_ms, ec);
        } else if (!tr->comp.scale_x().enabled) {
            tr->comp.scale_x().enabled = false;
            tr->comp.scale_y().enabled = false;
        }

        if (enable_rot) {
            tr->comp.configure_rotation(rot_z, 0.0, tr->duration_ms, ec);
        } else if (!tr->comp.rot_z().enabled) {
            tr->comp.rot_z().enabled = false;
        }

        if (enable_3d) {
            tr->comp.configure_3d_flip(rot_x, rot_y, 0.0, 0.0, tr->duration_ms, ec);
        } else if (!tr->comp.rot_x().enabled && !tr->comp.rot_y().enabled) {
            tr->comp.rot_x().enabled = false;
            tr->comp.rot_y().enabled = false;
        }

        if (enable_mask) {
            if (mask_type_val == 4) { // Typewriter
                tr->comp.configure_text(1, tr->duration_ms, ec);
            } else if (mask_type_val == 5) { // Character stagger
                tr->comp.configure_text(2, tr->duration_ms, ec);
            } else {
                tr->comp.configure_mask(0.0, 1.0, (int)mask_type_val, (float)mask_angle_val, tr->duration_ms, ec);
            }
            tr->comp.mask_feather = (float)mask_feather_val;
        } else if (!is_mask_or_text_preset(tr->preset_name)) {
            tr->comp.mask_progress().enabled = false;
            tr->comp.mask_type = 0;
            tr->comp.text_progress().enabled = false;
            tr->comp.text_mode = 0;
        }

        if (enable_blur) {
            tr->comp.configure_blur(blur_radius_val, 0.0, tr->duration_ms, ec);
        } else if (tr->preset_name != "blur_reveal") {
            tr->comp.blur_radius().enabled = false;
        }
    }
}

static void *motionfx_tr_create(obs_data_t *settings, obs_source_t *source)
{
    auto *tr = new motionfx_tr_data();
    tr->source = source;

    obs_enter_graphics();
    char *effect_file = obs_module_file("motionfx_3d.effect");
    char *error_string = nullptr;

    if (effect_file) {
        tr->effect_3d = gs_effect_create_from_file(effect_file, &error_string);
        if (!tr->effect_3d && error_string) {
            blog(LOG_ERROR, "[MotionFX] Failed to parse %s: %s", effect_file, error_string);
            bfree(error_string);
            error_string = nullptr;
        }
        bfree(effect_file);
    }

    if (!tr->effect_3d) {
        const char *fallback_paths[] = {
            "C:/ProgramData/obs-studio/plugins/motionfx/data/motionfx_3d.effect",
            "C:/Users/Dell/AppData/Roaming/obs-studio/plugins/motionfx/data/motionfx_3d.effect",
            "../../data/motionfx_3d.effect",
            "shaders/motionfx_3d.effect",
            "data/motionfx_3d.effect"
        };
        for (const char *fb : fallback_paths) {
            tr->effect_3d = gs_effect_create_from_file(fb, &error_string);
            if (tr->effect_3d) {
                blog(LOG_INFO, "[MotionFX] Successfully loaded effect from fallback path: %s", fb);
                break;
            } else if (error_string) {
                blog(LOG_ERROR, "[MotionFX] Error compiling fallback %s: %s", fb, error_string);
                bfree(error_string);
                error_string = nullptr;
            }
        }
    }

    if (tr->effect_3d) {
        blog(LOG_INFO, "[MotionFX] motionfx_3d.effect loaded successfully (%p)", tr->effect_3d);
        tr->image_param = gs_effect_get_param_by_name(tr->effect_3d, "image");
        tr->opacity_param = gs_effect_get_param_by_name(tr->effect_3d, "opacity");
        tr->color_param = gs_effect_get_param_by_name(tr->effect_3d, "color");
        tr->blur_radius_param = gs_effect_get_param_by_name(tr->effect_3d, "blur_radius");
        tr->mask_type_param = gs_effect_get_param_by_name(tr->effect_3d, "mask_type");
        tr->mask_progress_param = gs_effect_get_param_by_name(tr->effect_3d, "mask_progress");
        tr->mask_angle_param = gs_effect_get_param_by_name(tr->effect_3d, "mask_angle");
        tr->mask_feather_param = gs_effect_get_param_by_name(tr->effect_3d, "mask_feather");
    } else {
        blog(LOG_ERROR, "[MotionFX] CRITICAL: Could not load motionfx_3d.effect from any location!");
    }

    char *blur_file = obs_module_file("motionfx_blur.effect");
    if (blur_file) {
        tr->effect_blur = gs_effect_create_from_file(blur_file, NULL);
        bfree(blur_file);
    }

    char *mask_file = obs_module_file("motionfx_mask.effect");
    if (mask_file) {
        tr->effect_mask = gs_effect_create_from_file(mask_file, NULL);
        bfree(mask_file);
    }
    obs_leave_graphics();

    // Check source name on creation to see if OBS created this as Show or Hide Transition
    const char *src_name = obs_source_get_name(source);
    if (src_name && (strstr(src_name, "Hide") != nullptr || strstr(src_name, "hide") != nullptr)) {
        tr->transition_mode = "hide";
    } else if (src_name && (strstr(src_name, "Show") != nullptr || strstr(src_name, "show") != nullptr)) {
        tr->transition_mode = "show";
    } else {
        tr->transition_mode = "auto";
    }

    // Default configuration: 3D Flip X
    motionfx::PresetManager::instance().apply_preset(tr->comp, "3d_flip_x");

    obs_source_update(source, settings);
    return tr;
}

static void motionfx_tr_destroy(void *data)
{
    auto *tr = static_cast<motionfx_tr_data *>(data);
    if (!tr) return;

    obs_enter_graphics();
    if (tr->effect_3d) gs_effect_destroy(tr->effect_3d);
    if (tr->effect_blur) gs_effect_destroy(tr->effect_blur);
    if (tr->effect_mask) gs_effect_destroy(tr->effect_mask);
    obs_leave_graphics();

    delete tr;
}

static void motionfx_tr_callback(void *data, gs_texture_t *a, gs_texture_t *b, float t, uint32_t cx, uint32_t cy)
{
    auto *tr = static_cast<motionfx_tr_data *>(data);
    if (!tr || (!a && !b)) return;

    // Determine whether this transition is acting as Show or Hide
    bool is_hide = false;
    if (tr->transition_mode == "hide") {
        is_hide = true;
    } else if (tr->transition_mode == "show") {
        is_hide = false;
    } else {
        // "auto" mode:
        // In native OBS scene item transitions:
        // - Hide Transition: Source A is the item, Source B is NULL (transparent).
        // - Show Transition: Source A is NULL (transparent), Source B is the item.
        obs_source_t *sa = obs_transition_get_source(tr->source, OBS_TRANSITION_SOURCE_A);
        obs_source_t *sb = obs_transition_get_source(tr->source, OBS_TRANSITION_SOURCE_B);

        if (sa && !sb) {
            is_hide = true;
        } else if (!sa && sb) {
            is_hide = false;
        } else {
            // OBS Properties Dialog preview window (both dummy sources present) or fallback:
            const char *src_name = obs_source_get_name(tr->source);
            if (src_name && (strstr(src_name, "Hide") != nullptr || strstr(src_name, "hide") != nullptr)) {
                is_hide = true;
            } else {
                is_hide = false;
            }
        }

        if (sa) obs_source_release(sa);
        if (sb) obs_source_release(sb);
    }

    // Target texture:
    // When Hide: source A is the outgoing item quad.
    // When Show: source B is the incoming item quad.
    gs_texture_t *target_tex = is_hide ? a : b;
    if (!target_tex) target_tex = a ? a : b;
    if (!target_tex) return;

    // Evaluate animation:
    // Show (reverse=false): animates start_val -> end_val (enters view into resting place)
    // Hide (reverse=true): animates end_val -> start_val (exits from resting place into hidden state)
    double total_ms = tr->comp.get_total_duration_ms();
    if (total_ms <= 0.0) total_ms = (tr->duration_ms > 0) ? (double)tr->duration_ms : 500.0;

    double elapsed_ms = (double)t * total_ms;
    tr->comp.evaluate(elapsed_ms, is_hide);

    motionfx::Transform2D t2d = tr->comp.apply_to_transform2d(motionfx::Transform2D{0.0f, 0.0f, 0.0f, 1.0f, 1.0f});
    motionfx::Transform3D t3d = tr->comp.get_transform3d();
    motionfx::EffectsState eff = tr->comp.get_effects_state();

    const bool prev_srgb = gs_framebuffer_srgb_enabled();
    gs_enable_framebuffer_srgb(true);

    gs_effect_t *effect = tr->effect_3d ? tr->effect_3d : obs_get_base_effect(OBS_EFFECT_DEFAULT);
    if (effect) {
        if (tr->effect_3d) {
            gs_effect_set_texture_srgb(tr->image_param, target_tex);
            gs_effect_set_float(tr->opacity_param, eff.opacity);

            struct vec4 col;
            vec4_set(&col, 1.0f, 1.0f, 1.0f, 1.0f);
            gs_effect_set_vec4(tr->color_param, &col);

            if (tr->blur_radius_param)
                gs_effect_set_float(tr->blur_radius_param, eff.blur_radius);

            int m_type = eff.mask_type;
            float m_prog = eff.mask_progress;
            if (tr->comp.text_mode == 1) { // Typewriter
                m_type = 4;
                m_prog = (float)tr->comp.text_progress().current_val;
            } else if (tr->comp.text_mode == 2) { // Character stagger
                m_type = 5;
                m_prog = (float)tr->comp.text_progress().current_val;
            }

            if (tr->mask_type_param)
                gs_effect_set_int(tr->mask_type_param, m_type);
            if (tr->mask_progress_param)
                gs_effect_set_float(tr->mask_progress_param, m_prog);
            if (tr->mask_angle_param)
                gs_effect_set_float(tr->mask_angle_param, eff.mask_angle);
            if (tr->mask_feather_param)
                gs_effect_set_float(tr->mask_feather_param, eff.mask_feather);
        } else {
            gs_eparam_t *image_param = gs_effect_get_param_by_name(effect, "image");
            if (image_param) gs_effect_set_texture_srgb(image_param, target_tex);
        }

        gs_matrix_push();

        // Calculate Anchor Point (Pivot)
        float pivot_x = (float)cx * 0.5f;
        float pivot_y = (float)cy * 0.5f;
        if (tr->anchor_point == "top") {
            pivot_y = 0.0f;
        } else if (tr->anchor_point == "bottom") {
            pivot_y = (float)cy;
        } else if (tr->anchor_point == "left") {
            pivot_x = 0.0f;
        } else if (tr->anchor_point == "right") {
            pivot_x = (float)cx;
        }

        gs_matrix_translate3f(pivot_x + t2d.pos_x, pivot_y + t2d.pos_y, t3d.pos_z);

        // Apply 3D Euler rotations (Rotation X = Flip X, Rotation Y = Flip Y, Rotation Z = Spin)
        constexpr float DEG2RAD = 0.01745329251994329577f;
        if (std::abs(t3d.rot_x) > 1e-3f)
            gs_matrix_rotaa4f(1.0f, 0.0f, 0.0f, t3d.rot_x * DEG2RAD);
        if (std::abs(t3d.rot_y) > 1e-3f)
            gs_matrix_rotaa4f(0.0f, 1.0f, 0.0f, t3d.rot_y * DEG2RAD);
        if (std::abs(t2d.rot) > 1e-3f)
            gs_matrix_rotaa4f(0.0f, 0.0f, 1.0f, t2d.rot * DEG2RAD);

        // Apply Scale
        gs_matrix_scale3f(t2d.scale_x, t2d.scale_y, 1.0f);

        // Translate back from pivot
        gs_matrix_translate3f(-pivot_x, -pivot_y, 0.0f);

        enum gs_cull_mode prev_cull = gs_get_cull_mode();
        gs_set_cull_mode(GS_NEITHER);

        while (gs_effect_loop(effect, "Draw")) {
            gs_draw_sprite(NULL, 0, cx, cy);
        }

        gs_set_cull_mode(prev_cull);

        gs_matrix_pop();
    }

    gs_enable_framebuffer_srgb(prev_srgb);
}

static void motionfx_tr_video_render(void *data, gs_effect_t *effect)
{
    UNUSED_PARAMETER(effect);
    auto *tr = static_cast<motionfx_tr_data *>(data);
    if (!tr) return;

    const bool prev_linear = gs_set_linear_srgb(true);
    obs_transition_video_render2(tr->source, motionfx_tr_callback, NULL);
    gs_set_linear_srgb(prev_linear);
}

static float mix_a(void *data, float t)
{
    UNUSED_PARAMETER(data);
    return 1.0f - t;
}

static float mix_b(void *data, float t)
{
    UNUSED_PARAMETER(data);
    return t;
}

static bool motionfx_tr_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio,
                                     uint32_t mixers, size_t channels, size_t sample_rate)
{
    auto *tr = static_cast<motionfx_tr_data *>(data);
    if (!tr) return false;
    return obs_transition_audio_render(tr->source, ts_out, audio, mixers, channels, sample_rate, mix_a, mix_b);
}

static enum gs_color_space motionfx_tr_video_get_color_space(void *data, size_t count,
                                                             const enum gs_color_space *preferred_spaces)
{
    auto *tr = static_cast<motionfx_tr_data *>(data);
    const enum gs_color_space tr_space = obs_transition_video_get_color_space(tr->source);

    enum gs_color_space space = tr_space;
    for (size_t i = 0; i < count; ++i) {
        space = preferred_spaces[i];
        if (space == tr_space) break;
    }
    return space;
}

static bool custom_override_modified(obs_properties_t *props, obs_property_t *p, obs_data_t *settings)
{
    UNUSED_PARAMETER(p);
    bool custom = obs_data_get_bool(settings, "custom_override");

    // Easing curve
    obs_property_set_visible(obs_properties_get(props, "easing"), custom);

    // Slide group
    obs_property_set_visible(obs_properties_get(props, "enable_slide"), custom);
    bool slide = custom && obs_data_get_bool(settings, "enable_slide");
    obs_property_set_visible(obs_properties_get(props, "slide_x"), slide);
    obs_property_set_visible(obs_properties_get(props, "slide_y"), slide);
    obs_property_set_visible(obs_properties_get(props, "slide_z"), slide);

    // Fade group
    obs_property_set_visible(obs_properties_get(props, "enable_fade"), custom);
    bool fade = custom && obs_data_get_bool(settings, "enable_fade");
    obs_property_set_visible(obs_properties_get(props, "fade_start"), fade);
    obs_property_set_visible(obs_properties_get(props, "fade_end"), fade);

    // Scale group
    obs_property_set_visible(obs_properties_get(props, "enable_scale"), custom);
    bool scale = custom && obs_data_get_bool(settings, "enable_scale");
    obs_property_set_visible(obs_properties_get(props, "scale_start"), scale);
    obs_property_set_visible(obs_properties_get(props, "scale_end"), scale);

    // 3D group
    obs_property_set_visible(obs_properties_get(props, "enable_3d"), custom);
    bool rot3d = custom && obs_data_get_bool(settings, "enable_3d");
    obs_property_set_visible(obs_properties_get(props, "rot_x"), rot3d);
    obs_property_set_visible(obs_properties_get(props, "rot_y"), rot3d);

    // 2D Rotation group
    obs_property_set_visible(obs_properties_get(props, "enable_rotation"), custom);
    bool rot2d = custom && obs_data_get_bool(settings, "enable_rotation");
    obs_property_set_visible(obs_properties_get(props, "rot_z"), rot2d);

    // Mask group
    obs_property_set_visible(obs_properties_get(props, "enable_mask"), custom);
    bool mask = custom && obs_data_get_bool(settings, "enable_mask");
    obs_property_set_visible(obs_properties_get(props, "mask_type"), mask);
    obs_property_set_visible(obs_properties_get(props, "mask_angle"), mask);
    obs_property_set_visible(obs_properties_get(props, "mask_feather"), mask);

    // Blur group
    obs_property_set_visible(obs_properties_get(props, "enable_blur"), custom);
    bool blur = custom && obs_data_get_bool(settings, "enable_blur");
    obs_property_set_visible(obs_properties_get(props, "blur_radius"), blur);

    return true;
}

static obs_properties_t *motionfx_tr_get_properties(void *data)
{
    obs_properties_t *ppts = obs_properties_create();

    // 1. Preset Dropdown
    obs_property_t *p_preset = obs_properties_add_list(ppts, "preset", "Animation Preset",
                                                       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    auto preset_names = motionfx::PresetManager::instance().get_preset_names();
    for (const auto &name : preset_names) {
        const auto *pr = motionfx::PresetManager::instance().get_preset(name);
        std::string label = (pr && !pr->display_name.empty()) ? pr->display_name : name;
        obs_property_list_add_string(p_preset, label.c_str(), name.c_str());
    }

    // 2. Transition Mode (Auto Detect, Force Show, Force Hide)
    obs_property_t *p_mode = obs_properties_add_list(ppts, "transition_mode", "Transition Mode",
                                                     OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(p_mode, "Auto Detect (Show / Hide)", "auto");
    obs_property_list_add_string(p_mode, "Show (Animate into view)", "show");
    obs_property_list_add_string(p_mode, "Hide (Animate out / Reverse)", "hide");

    // 3. Anchor Point (Pivot)
    obs_property_t *p_anchor = obs_properties_add_list(ppts, "anchor_point", "Transform Anchor (Pivot)",
                                                       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(p_anchor, "Center", "center");
    obs_property_list_add_string(p_anchor, "Top Edge (Fold from top)", "top");
    obs_property_list_add_string(p_anchor, "Bottom Edge (Fold from bottom)", "bottom");
    obs_property_list_add_string(p_anchor, "Left Edge (Open from left)", "left");
    obs_property_list_add_string(p_anchor, "Right Edge (Open from right)", "right");

    // 4. Custom override toggle
    obs_property_t *p_custom = obs_properties_add_bool(ppts, "custom_override", "Custom Parameter Overrides");
    obs_property_set_modified_callback(p_custom, custom_override_modified);

    // 5. Easing
    obs_property_t *p_easing = obs_properties_add_list(ppts, "easing", "Easing Curve",
                                                       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(p_easing, "Ease Out (Decelerate into place)", "ease_out");
    obs_property_list_add_string(p_easing, "Ease In (Accelerate out)", "ease_in");
    obs_property_list_add_string(p_easing, "Ease In Out", "ease_in_out");
    obs_property_list_add_string(p_easing, "Back Out (Overshoot settle)", "back_out");
    obs_property_list_add_string(p_easing, "Elastic Out (Spring pop)", "elastic_out");
    obs_property_list_add_string(p_easing, "Bounce Out (Physics bounce)", "bounce_out");
    obs_property_list_add_string(p_easing, "Linear (Constant speed)", "linear");

    // 6. Slide Parameters
    obs_property_t *p_slide = obs_properties_add_bool(ppts, "enable_slide", "Enable Slide / Position");
    obs_property_set_modified_callback(p_slide, custom_override_modified);
    obs_properties_add_float_slider(ppts, "slide_x", "Slide X Offset (px)", -1920.0, 1920.0, 10.0);
    obs_properties_add_float_slider(ppts, "slide_y", "Slide Y Offset (px)", -1080.0, 1080.0, 10.0);
    obs_properties_add_float_slider(ppts, "slide_z", "Slide Depth Z (3D)", -2000.0, 2000.0, 20.0);

    // 7. Fade / Opacity Parameters
    obs_property_t *p_fade = obs_properties_add_bool(ppts, "enable_fade", "Enable Fade / Opacity");
    obs_property_set_modified_callback(p_fade, custom_override_modified);
    obs_properties_add_float_slider(ppts, "fade_start", "Start Opacity", 0.0, 1.0, 0.05);
    obs_properties_add_float_slider(ppts, "fade_end", "End Opacity", 0.0, 1.0, 0.05);

    // 8. Scale Parameters
    obs_property_t *p_scale = obs_properties_add_bool(ppts, "enable_scale", "Enable Scale / Zoom");
    obs_property_set_modified_callback(p_scale, custom_override_modified);
    obs_properties_add_float_slider(ppts, "scale_start", "Start Scale", 0.0, 3.0, 0.05);
    obs_properties_add_float_slider(ppts, "scale_end", "End Scale", 0.0, 3.0, 0.05);

    // 9. 3D & Rotation Parameters
    obs_property_t *p_3d = obs_properties_add_bool(ppts, "enable_3d", "Enable 3D Perspective (Flip X / Y)");
    obs_property_set_modified_callback(p_3d, custom_override_modified);
    obs_properties_add_float_slider(ppts, "rot_x", "3D Rotation X (Flip X deg)", -180.0, 180.0, 5.0);
    obs_properties_add_float_slider(ppts, "rot_y", "3D Rotation Y (Card Flip deg)", -180.0, 180.0, 5.0);

    obs_property_t *p_rot = obs_properties_add_bool(ppts, "enable_rotation", "Enable 2D Rotation Z");
    obs_property_set_modified_callback(p_rot, custom_override_modified);
    obs_properties_add_float_slider(ppts, "rot_z", "Rotation Z Angle (deg)", -360.0, 360.0, 5.0);

    // 10. Mask & Wipe Parameters
    obs_property_t *p_mask = obs_properties_add_bool(ppts, "enable_mask", "Enable Mask / Wipe Reveal");
    obs_property_set_modified_callback(p_mask, custom_override_modified);
    obs_property_t *p_mtype = obs_properties_add_list(ppts, "mask_type", "Mask Pattern",
                                                      OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
    obs_property_list_add_int(p_mtype, "Linear Wipe (Angle)", 1);
    obs_property_list_add_int(p_mtype, "Radial Circle (Iris)", 2);
    obs_property_list_add_int(p_mtype, "Box Wipe (Rectangle)", 3);
    obs_property_list_add_int(p_mtype, "Typewriter (Letter by Letter)", 4);
    obs_property_list_add_int(p_mtype, "Character Stagger (Stepped)", 5);

    obs_properties_add_float_slider(ppts, "mask_angle", "Mask Angle (deg)", -180.0, 180.0, 5.0);
    obs_properties_add_float_slider(ppts, "mask_feather", "Mask Edge Feather (Softness)", 0.005, 0.25, 0.01);

    // 11. Blur Parameters
    obs_property_t *p_blur = obs_properties_add_bool(ppts, "enable_blur", "Enable Blur / Focus Reveal");
    obs_property_set_modified_callback(p_blur, custom_override_modified);
    obs_properties_add_float_slider(ppts, "blur_radius", "Blur Radius (px)", 1.0, 30.0, 1.0);

    UNUSED_PARAMETER(data);
    return ppts;
}

// Native OBS Source Info Definition
static struct obs_source_info motionfx_transition_info = {};

// Module Lifecycle Entrypoints
bool obs_module_load(void)
{
    blog(LOG_INFO, "[MotionFX] Loading MotionFX Animation Engine v1.0.0 (Native Scene Item Transition Source)...");

    motionfx_transition_info.id = "motionfx_transition";
    motionfx_transition_info.type = OBS_SOURCE_TYPE_TRANSITION;
    motionfx_transition_info.output_flags = OBS_SOURCE_VIDEO;
    motionfx_transition_info.get_name = motionfx_tr_get_name;
    motionfx_transition_info.get_defaults = motionfx_tr_get_defaults;
    motionfx_transition_info.create = motionfx_tr_create;
    motionfx_transition_info.destroy = motionfx_tr_destroy;
    motionfx_transition_info.update = motionfx_tr_update;
    motionfx_transition_info.video_render = motionfx_tr_video_render;
    motionfx_transition_info.audio_render = motionfx_tr_audio_render;
    motionfx_transition_info.get_properties = motionfx_tr_get_properties;
    motionfx_transition_info.video_get_color_space = motionfx_tr_video_get_color_space;

    // OBS 31.1.x libobs binary has sizeof(obs_source_info) == 408 (git master has 424 with icon callbacks)
    size_t reg_size = (sizeof(motionfx_transition_info) > 408) ? 408 : sizeof(motionfx_transition_info);
    obs_register_source_s(&motionfx_transition_info, reg_size);

    blog(LOG_INFO, "[MotionFX] Successfully registered 'motionfx_transition' as native OBS_SOURCE_TYPE_TRANSITION.");
    blog(LOG_INFO, "[MotionFX] Available in OBS right-click context menu under 'Show Transition' and 'Hide Transition'.");
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "[MotionFX] Unloading MotionFX plugin...");
    motionfx::AnimationEngine::instance().clear_all();
}

#endif // OBS_PLUGIN_BUILD
