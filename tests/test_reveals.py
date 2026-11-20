import subprocess
import time
import os
import glob
import obsws_python as obs

OBS_PATH = r"C:\Program Files\obs-studio\bin\64bit\obs64.exe"
OBS_DIR = r"C:\Program Files\obs-studio\bin\64bit"
WS_PORT = 4455
WS_PASS = "3xbvu1tK7iYVZB09"

# 1. Start OBS
print("[1] Starting OBS Studio...")
proc = subprocess.Popen([OBS_PATH, "--disable-shutdown-check"], cwd=OBS_DIR)

client = None
for i in range(20):
    time.sleep(1.5)
    try:
        client = obs.ReqClient(host="localhost", port=WS_PORT, password=WS_PASS, timeout=3)
        v = client.get_version()
        print(f"Connected to OBS {v.obs_version} (WebSocket {v.obs_web_socket_version})")
        break
    except Exception:
        pass

if not client:
    print("Could not connect to OBS WebSocket!")
    exit(1)

# 2. Check scenes and test reveal transitions
scenes = client.get_scene_list().scenes
print(f"[2] Program Scene List: {[s['sceneName'] for s in scenes]}")

# Switch to 'BAR TS 2025' or 'LOWER CCTT7 MS'
target_scene = "LOWER CCTT7 MS"
client.set_current_program_scene(target_scene)
time.sleep(1.0)

items = client.get_scene_item_list(target_scene).scene_items
print(f"[3] Scene items in '{target_scene}':")
target_item = None
for it in items:
    if "TIEUDE01" in it['sourceName']:
        target_item = it
        print(f"  -> Found target item: {it['sourceName']} (ID: {it['sceneItemId']})")

if target_item:
    item_id = target_item['sceneItemId']
    print("\n[4] Triggering SHOW on TIEUDE01...")
    client.set_scene_item_enabled(target_scene, item_id, True)
    time.sleep(2.0)
    
    print("\n[5] Triggering HIDE on TIEUDE01...")
    client.set_scene_item_enabled(target_scene, item_id, False)
    time.sleep(2.0)
    print("Visibility cycle completed!")

# 3. Check OBS log for MotionFX shader loading messages
time.sleep(1.0)
log_dir = os.path.expandvars(r"%APPDATA%\obs-studio\logs")
logs = sorted(glob.glob(os.path.join(log_dir, "*.txt")), key=os.path.getmtime, reverse=True)
if logs:
    latest_log = logs[0]
    print(f"\n[6] Checking latest OBS log: {latest_log}")
    with open(latest_log, "r", encoding="utf-8", errors="ignore") as f:
        log_content = f.read()
    
    fx_logs = [line for line in log_content.splitlines() if "[MotionFX]" in line or "motionfx" in line.lower()]
    print("MotionFX Log Entries:")
    for l in fx_logs:
        print("  ", l)

print("\n>>> TEST FINISHED - OBS Studio left running <<<")
