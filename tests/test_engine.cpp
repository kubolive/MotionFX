#include "motionfx/motionfx_engine.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    using namespace motionfx;

    std::cout << "[TEST] Testing MotionFX Scene Item Animation Engine..." << std::endl;

    AnimationEngine& engine = AnimationEngine::instance();
    engine.clear_all();

    // Verification 1: Multi-instance isolation of same source
    SceneItemKey key_cam1{"SceneMain", 101, "Webcam"};
    SceneItemKey key_cam2{"ScenePIP", 202, "Webcam"}; // same source name "Webcam", different scene & item id

    auto item1 = engine.get_or_create(key_cam1);
    auto item2 = engine.get_or_create(key_cam2);

    Transform2D base1{100.0f, 100.0f, 0.0f, 1.0f, 1.0f};
    Transform2D base2{800.0f, 500.0f, 0.0f, 0.5f, 0.5f};
    item1->set_base_transform(base1);
    item2->set_base_transform(base2);

    bool item1_native_vis = false;
    Transform2D item1_rendered_trans{};
    item1->set_native_hooks(
        [&](bool v) { item1_native_vis = v; },
        [&](const Transform2D& t) { item1_rendered_trans = t; }
    );

    bool item2_native_vis = true;
    Transform2D item2_rendered_trans = base2;
    item2->set_native_hooks(
        [&](bool v) { item2_native_vis = v; },
        [&](const Transform2D& t) { item2_rendered_trans = t; }
    );

    // Track events
    std::vector<std::string> received_events;
    engine.add_event_listener([&](const AnimationEventData& evt) {
        received_events.push_back(evt.event_type + ":" + evt.scene_name + ":" + std::to_string(evt.item_id));
    });

    // 1. Trigger Show on Item 1
    item1->trigger_show();
    assert(item1->is_animating());
    assert(!item2->is_animating()); // Item 2 MUST NOT be affected!
    assert(item1_native_vis == true); // Set to visible immediately for show

    // Advance 250ms (Show duration is 500ms -> 50%)
    engine.advance_delta(250.0);
    assert(item1->is_animating());
    assert(!item2->is_animating());
    assert(std::abs(item2_rendered_trans.pos_x - 800.0f) < 1e-4); // Item 2 unchanged!

    std::cout << "  -> Multi-instance isolation verified. Item 1 animating, Item 2 unchanged." << std::endl;

    // Advance remaining 250ms -> Show completes
    engine.advance_delta(250.0);
    assert(!item1->is_animating());
    assert(std::abs(item1->get_current_transform2d().pos_x - 100.0f) < 1e-4);
    assert(std::abs(item1->get_current_effects().opacity - 1.0f) < 1e-4);

    std::cout << "  -> Show Transition completed at base transform." << std::endl;

    // 2. Verification of Hide Transition (delays hiding until completion)
    // Customise Hide transition: Slide to +400px over 400ms, NO fade
    item1->hide_config().configure_slide(400.0, 0.0, 400.0, EasingConfig::ease_in());
    item1->hide_config().opacity().enabled = false;

    item1->trigger_hide();
    assert(item1->is_animating());
    // CRITICAL REQUIREMENT 7: Item MUST remain visible in OBS while animating hide!
    assert(item1_native_vis == true);
    std::cout << "  -> Hide started: Item remains native_visible = true during animation." << std::endl;

    // Advance 200ms (50% through hide)
    engine.advance_delta(200.0);
    assert(item1->is_animating());
    assert(item1_native_vis == true); // Still visible!
    assert(std::abs(item1->get_current_effects().opacity - 1.0f) < 1e-4); // Hide has NO fade
    std::cout << "  -> Mid-hide (200ms): Item still visible, Opacity remains 1.0 (no fade)." << std::endl;

    // Advance remaining 200ms -> Hide completes!
    engine.advance_delta(200.0);
    assert(!item1->is_animating());
    // CRITICAL REQUIREMENT 7: Only genuinely hide item after animation completes!
    assert(item1_native_vis == false);
    std::cout << "  -> Hide completed: native_visibility is now false!" << std::endl;

    // 3. Verification of event dispatches
    bool found_show_complete = false;
    bool found_hide_complete = false;
    for (const auto& ev : received_events) {
        if (ev.find("ShowCompleted:SceneMain:101") != std::string::npos) found_show_complete = true;
        if (ev.find("HideCompleted:SceneMain:101") != std::string::npos) found_hide_complete = true;
    }
    assert(found_show_complete);
    assert(found_hide_complete);
    std::cout << "  -> Event broadcasting (ShowCompleted, HideCompleted) verified." << std::endl;

    std::cout << "[SUCCESS] All Scene Item Animation Engine tests passed successfully!" << std::endl;
    return 0;
}
