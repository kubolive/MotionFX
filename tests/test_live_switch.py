import time
import obsws_python as obs

WS_PORT = 4455
WS_PASS = "3xbvu1tK7iYVZB09"

client = obs.ReqClient(host="localhost", port=WS_PORT, password=WS_PASS, timeout=10)
v = client.get_version()
print(f"Connected to OBS {v.obs_version} (WebSocket {v.obs_web_socket_version})")

# Test on scene 'BAR TS 2025' -> TT1_BAR
scene_name = "BAR TS 2025"
client.set_current_program_scene(scene_name)
time.sleep(1.0)

items = client.get_scene_item_list(scene_name).scene_items
tt1_item = None
for it in items:
    if it['sourceName'] == "TT1_BAR":
        tt1_item = it
        break

if tt1_item:
    item_id = tt1_item['sceneItemId']
    print(f"Testing TT1_BAR (ID {item_id}) in scene '{scene_name}'")
    
    # Hide it first
    print("Hiding TT1_BAR...")
    client.set_scene_item_enabled(scene_name, item_id, False)
    time.sleep(1.5)
    
    # Show it (Triggers Show Transition: Mask Reveal / Typewriter / Flip)
    print("Showing TT1_BAR (Triggering Show Transition)...")
    client.set_scene_item_enabled(scene_name, item_id, True)
    time.sleep(2.0)
    
    # Hide it (Triggers Hide Transition: Reverse)
    print("Hiding TT1_BAR (Triggering Hide Transition)...")
    client.set_scene_item_enabled(scene_name, item_id, False)
    time.sleep(2.0)
    
    # Show it again so user can see it in OBS
    client.set_scene_item_enabled(scene_name, item_id, True)
    print("SUCCESS: TT1_BAR Show & Hide transition test complete!")
else:
    print("TT1_BAR not found in scene", scene_name)
