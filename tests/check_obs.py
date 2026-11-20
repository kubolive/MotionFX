import obsws_python as obs
import json

client = obs.ReqClient(host='localhost', port=4455, password='3xbvu1tK7iYVZB09', timeout=5)

print("Scenes and target items:")
for s in client.get_scene_list().scenes:
    sname = s['sceneName']
    try:
        items = client.get_scene_item_list(sname).scene_items
        for it in items:
            if it['sourceName'] in ['TT1_BAR', 'TIEUDE01']:
                print(f"  [{sname}] ID {it['sceneItemId']}: {it['sourceName']} (enabled={it['sceneItemEnabled']})")
    except Exception as e:
        print(f"  Error on {sname}: {e}")

print("\nAll transition sources (Special inputs):")
try:
    special = client.get_special_inputs()
    print("Special inputs:", special)
except Exception as e:
    print(e)
