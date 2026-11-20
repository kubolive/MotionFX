#include "motionfx/motionfx_dispatcher.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    using namespace motionfx;

    std::cout << "[TEST] Testing MotionFX Phase 4 & Section 15: Unified Command Dispatcher..." << std::endl;

    CommandDispatcher& dispatcher = CommandDispatcher::instance();
    AnimationEngine& engine = AnimationEngine::instance();
    engine.clear_all();

    // 1. GetPresets request
    CommandRequest req_presets;
    req_presets.request_type = "GetPresets";
    CommandResponse resp_presets = dispatcher.execute(req_presets);
    assert(resp_presets.success);
    assert(resp_presets.data_json.find("slide_in_left") != std::string::npos);
    assert(resp_presets.data_json.find("3d_flip_y") != std::string::npos);
    assert(resp_presets.data_json.find("card_flip") != std::string::npos);
    std::cout << "  -> GetPresets returned valid JSON list with built-in presets." << std::endl;

    // 2. PlayShow via request
    CommandRequest req_show;
    req_show.request_type = "PlayShow";
    req_show.scene_name = "StudioScene";
    req_show.item_id = 42;
    req_show.source_name = "Camera1";

    CommandResponse resp_show = dispatcher.execute(req_show);
    assert(resp_show.success);

    // Verify item is animating
    auto item = engine.find("StudioScene", 42);
    assert(item != nullptr);
    assert(item->is_animating());
    assert(item->get_active_transition() == TransitionType::Show);
    std::cout << "  -> PlayShow successfully triggered animation on StudioScene:42." << std::endl;

    // 3. GetAnimationState query
    CommandRequest req_state;
    req_state.request_type = "GetAnimationState";
    req_state.scene_name = "StudioScene";
    req_state.item_id = 42;
    CommandResponse resp_state = dispatcher.execute(req_state);
    assert(resp_state.success);
    assert(resp_state.data_json.find("\"transition\": \"show\"") != std::string::npos);
    assert(resp_state.data_json.find("\"status\": \"playing\"") != std::string::npos);
    std::cout << "  -> GetAnimationState returned active playing state JSON." << std::endl;

    // 4. Runtime Parameter modification
    CommandRequest req_param;
    req_param.request_type = "SetAnimationParameter";
    req_param.scene_name = "StudioScene";
    req_param.item_id = 42;
    req_param.parameter = "pos_x";
    req_param.value = 650.0;
    CommandResponse resp_param = dispatcher.execute(req_param);
    assert(resp_param.success);
    std::cout << "  -> SetAnimationParameter (pos_x = 650) applied at runtime." << std::endl;

    // 5. Pause, Resume, Stop
    CommandRequest req_pause;
    req_pause.request_type = "PauseAnimation";
    req_pause.scene_name = "StudioScene";
    req_pause.item_id = 42;
    assert(dispatcher.execute(req_pause).success);
    assert(item->get_playback_state() == PlaybackState::Paused);

    CommandRequest req_resume;
    req_resume.request_type = "ResumeAnimation";
    req_resume.scene_name = "StudioScene";
    req_resume.item_id = 42;
    assert(dispatcher.execute(req_resume).success);
    assert(item->get_playback_state() == PlaybackState::Playing);

    CommandRequest req_stop;
    req_stop.request_type = "StopAnimation";
    req_stop.scene_name = "StudioScene";
    req_stop.item_id = 42;
    assert(dispatcher.execute(req_stop).success);
    assert(!item->is_animating());
    std::cout << "  -> Pause, Resume, Stop lifecycle verified." << std::endl;

    // 6. PlayPreset "3d_flip_y"
    CommandRequest req_play_preset;
    req_play_preset.request_type = "PlayPreset";
    req_play_preset.scene_name = "StudioScene";
    req_play_preset.item_id = 42;
    req_play_preset.preset_name = "3d_flip_y";
    CommandResponse resp_preset = dispatcher.execute(req_play_preset);
    assert(resp_preset.success);
    assert(item->is_animating());
    std::cout << "  -> PlayPreset (\"3d_flip_y\") successfully loaded and started." << std::endl;

    // 7. Identification by Source Name fallback
    CommandRequest req_source_fallback;
    req_source_fallback.request_type = "PlayHide";
    req_source_fallback.scene_name = "StudioScene";
    req_source_fallback.item_id = 0; // No item ID provided
    req_source_fallback.source_name = "Camera1"; // Identified by source name
    CommandResponse resp_fallback = dispatcher.execute(req_source_fallback);
    assert(resp_fallback.success);
    assert(item->get_active_transition() == TransitionType::Hide);
    std::cout << "  -> Source Name fallback resolution verified." << std::endl;

    std::cout << "[SUCCESS] All Command Dispatcher tests passed successfully!" << std::endl;
    return 0;
}
