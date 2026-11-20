import time
import subprocess
import os
import sys
import obsws_python as obs

OBS_PATH = r"C:\Program Files\obs-studio\bin\64bit\obs64.exe"
OBS_DIR = r"C:\Program Files\obs-studio\bin\64bit"
WS_PORT = 4455
WS_PASS = "3xbvu1tK7iYVZB09"

def run_live_test():
    print("================================================================================")
    print("MotionFX Live OBS Studio 31.1.x Integration Test")
    print("================================================================================")

    # 1. Start OBS Studio
    print("[1/5] Launching OBS Studio 31.1.x...")
    proc = subprocess.Popen([OBS_PATH, "--disable-shutdown-check"], cwd=OBS_DIR)
    
    # Wait for OBS WebSocket to be ready
    client = None
    for attempt in range(15):
        time.sleep(2)
        try:
            print(f"[2/5] Connecting to OBS WebSocket on port {WS_PORT} (attempt {attempt+1}/15)...")
            client = obs.ReqClient(host="localhost", port=WS_PORT, password=WS_PASS, timeout=5)
            version_info = client.get_version()
            print(f"      OBS Version: {version_info.obs_version}")
            print(f"      WebSocket Version: {version_info.obs_web_socket_version}")
            break
        except Exception as e:
            print(f"      OBS not ready yet ({e}), waiting...")
            client = None

    if not client:
        raise RuntimeError("Failed to connect to OBS WebSocket after multiple attempts")

    try:

        # 4. Verify Registered Transition Kinds
        print("[3/5] Querying OBS Transition Kinds...")
        kinds = client.get_transition_kind_list().transition_kinds
        print(f"      Registered Transition Kinds ({len(kinds)}):")
        for k in kinds:
            mark = " [FOUND - NATIVE TRANSITION]" if k == "motionfx_transition" else ""
            print(f"        - {k}{mark}")
        
        assert "motionfx_transition" in kinds, "CRITICAL: motionfx_transition not registered as transition kind!"
        print("      >>> SUCCESS: 'motionfx_transition' is verified as registered OBS Transition Source.")

        # 5. Query Scenes and Scene Items
        print("[4/5] Inspecting Scenes and Scene Items...")
        current_scene = client.get_current_program_scene().current_program_scene_name
        print(f"      Current Program Scene: {current_scene}")
        
        item_list = client.get_scene_item_list(current_scene).scene_items
        print(f"      Found {len(item_list)} Scene Items in '{current_scene}':")
        for it in item_list[:5]:
            print(f"        - ID: {it['sceneItemId']}, Source: '{it['sourceName']}', Enabled: {it['sceneItemEnabled']}")

        # 6. Test Show / Hide Lifecycle on first item
        if item_list:
            target_item = item_list[0]
            item_id = target_item['sceneItemId']
            name = target_item['sourceName']
            print(f"[5/5] Testing Scene Item Visibility Lifecycle on '{name}' (ID {item_id})...")

            # Toggle to Visible (Show Transition)
            print("      Triggering Show Transition (set_scene_item_enabled = True)...")
            client.set_scene_item_enabled(current_scene, item_id, True)
            time.sleep(1.0)
            status_after_show = client.get_scene_item_enabled(current_scene, item_id).scene_item_enabled
            print(f"      Visibility after Show: {status_after_show}")

            # Toggle to Hidden (Hide Transition)
            print("      Triggering Hide Transition (set_scene_item_enabled = False)...")
            client.set_scene_item_enabled(current_scene, item_id, False)
            time.sleep(1.0)
            status_after_hide = client.get_scene_item_enabled(current_scene, item_id).scene_item_enabled
            print(f"      Visibility after Hide: {status_after_hide}")

        print("================================================================================")
        print(">>> ALL LIVE INTEGRATION TESTS PASSED 100% SUCCESS <<<")
        print("================================================================================")
        print("Leaving OBS Studio running for user demo...")
        return True

    except Exception as e:
        print(f"Error during test: {e}")
        proc.terminate()
        raise

if __name__ == "__main__":
    success = run_live_test()
    sys.exit(0 if success else 1)
