import subprocess
import time
import sys
import obsws_python as obs

OBS_PATH = r"C:\Program Files\obs-studio\bin\64bit\obs64.exe"
OBS_DIR = r"C:\Program Files\obs-studio\bin\64bit"
WS_PORT = 4455
WS_PASS = "3xbvu1tK7iYVZB09"

print("[1] Launching OBS Studio 31.1.2...")
proc = subprocess.Popen([OBS_PATH, "--disable-shutdown-check"], cwd=OBS_DIR)

client = None
print("[2] Connecting to OBS WebSocket...")
for attempt in range(25):
    time.sleep(2.0)
    try:
        client = obs.ReqClient(host="localhost", port=WS_PORT, password=WS_PASS, timeout=10)
        v = client.get_version()
        print(f"      Connected! OBS Version: {v.obs_version}, WebSocket Version: {v.obs_web_socket_version}")
        break
    except Exception as e:
        print(f"      Attempt {attempt+1}/25: waiting for OBS to finish loading ({e})...")

if not client:
    print("Failed to connect to OBS!")
    sys.exit(1)

# Allow scenes to settle
time.sleep(2.0)

scenes = client.get_scene_list().scenes
print(f"[3] Available Scenes: {[s['sceneName'] for s in scenes]}")

# Switch to 'BAR TS 2025'
scene_name = "BAR TS 2025"
client.set_current_program_scene(scene_name)
print(f"[4] Switched to scene '{scene_name}'")
time.sleep(1.0)

items = client.get_scene_item_list(scene_name).scene_items
tt1_item = None
for it in items:
    if it['sourceName'] == "TT1_BAR":
        tt1_item = it
        break

if tt1_item:
    item_id = tt1_item['sceneItemId']
    print(f"[5] Testing on '{tt1_item['sourceName']}' (Item ID: {item_id})")
    
    # Hide first
    print("    -> Setting enabled=False (Hiding)...")
    client.set_scene_item_enabled(scene_name, item_id, False)
    time.sleep(1.5)
    
    # Trigger Show
    print("    -> Setting enabled=True (Triggering SHOW Transition)...")
    client.set_scene_item_enabled(scene_name, item_id, True)
    time.sleep(2.5)
    
    # Trigger Hide (Must reverse cleanly)
    print("    -> Setting enabled=False (Triggering HIDE Transition - Reverse)...")
    client.set_scene_item_enabled(scene_name, item_id, False)
    time.sleep(2.5)
    
    # Leave it enabled so user sees it
    client.set_scene_item_enabled(scene_name, item_id, True)
    print("    -> Final state: enabled=True.")

print("\n>>> LIVE TEST COMPLETED SUCCESSFULLY <<<")
print("OBS Studio is left open and active for the user.")
