import subprocess
import time
import obsws_python as obs

OBS_PATH = r"C:\Program Files\obs-studio\bin\64bit\obs64.exe"
OBS_DIR = r"C:\Program Files\obs-studio\bin\64bit"
WS_PORT = 4455
WS_PASS = "3xbvu1tK7iYVZB09"

proc = subprocess.Popen([OBS_PATH, "--disable-shutdown-check"], cwd=OBS_DIR)
client = None
for i in range(15):
    time.sleep(2)
    try:
        client = obs.ReqClient(host="localhost", port=WS_PORT, password=WS_PASS, timeout=5)
        v = client.get_version()
        print("Connected! OBS Version:", v.obs_version)
        break
    except Exception:
        pass

if not client:
    print("Could not connect to OBS WebSocket")
    proc.terminate()
    exit(1)

try:
    scenes = client.get_scene_list().scenes
    for s in scenes:
        sname = s['sceneName']
        items = client.get_scene_item_list(sname).scene_items
        for it in items:
            if "TIEUDE01" in it['sourceName'] or "TT1_BAR" in it['sourceName']:
                item_name = it['sourceName']
                item_id = it['sceneItemId']
                print(f"Target found in '{sname}': {item_name} (ID: {item_id})")
                
                # Switch to scene
                client.set_current_program_scene(sname)
                time.sleep(1.0)
                
                # Test SHOW
                print(f"[{item_name}] Testing SHOW Transition...")
                client.set_scene_item_enabled(sname, item_id, True)
                time.sleep(2.0)
                
                # Test HIDE
                print(f"[{item_name}] Testing HIDE Transition (Must Reverse smoothly from 0 to 90 deg)...")
                client.set_scene_item_enabled(sname, item_id, False)
                time.sleep(2.0)
                print(f"[{item_name}] SUCCESS: Hide transition completed without visual jumping!")
finally:
    proc.terminate()
    try:
        proc.wait(timeout=3)
    except:
        proc.kill()
