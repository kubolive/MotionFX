import sys
import os
import asyncio

# Add python directory to path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "python")))

from motionfx import MotionFX, AsyncMotionFX, AnimationState, AnimationEvent, PresetInfo

def test_sync_sdk():
    print("[TEST] Testing Python SDK (Synchronous)...")
    fx = MotionFX(host="127.0.0.1", port=4455)
    assert fx.connect()

    # PlayShow
    assert fx.play_show("MainScene", 101)
    # PlayPreset
    assert fx.play_preset("MainScene", 101, "3d_flip_y")
    # SetParameter
    assert fx.set_parameter("MainScene", 101, "position_x", 500)
    # SetEnabled
    assert fx.set_enabled("MainScene", 101, "fade", False)
    # Pause, Resume, Stop
    assert fx.pause("MainScene", 101)
    assert fx.resume("MainScene", 101)
    assert fx.reverse("MainScene", 101)
    assert fx.stop("MainScene", 101)
    # PlayHide
    assert fx.play_hide("MainScene", 101)

    # GetAnimationState
    state = fx.get_animation_state("MainScene", 101)
    assert state.scene == "MainScene"
    assert state.item_id == 101

    # GetPresets
    presets = fx.get_presets()
    assert len(presets) >= 5
    assert any(p.name == "card_flip" for p in presets)

    print("  -> Synchronous SDK calls passed successfully.")

async def test_async_sdk():
    print("[TEST] Testing Python SDK (Asynchronous)...")
    async with AsyncMotionFX(host="127.0.0.1", port=4455) as fx:
        assert await fx.play_show("MainScene", 101)
        assert await fx.play_preset("MainScene", 101, "bounce")
        assert await fx.set_parameter("MainScene", 101, "position_y", 250)
        assert await fx.play_hide("MainScene", 101)
        state = await fx.get_animation_state("MainScene", 101)
        assert state.item_id == 101

    print("  -> Asynchronous SDK calls passed successfully.")

if __name__ == "__main__":
    test_sync_sdk()
    asyncio.run(test_async_sdk())
    print("[SUCCESS] All Python SDK tests passed successfully!")
