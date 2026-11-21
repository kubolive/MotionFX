#!/usr/bin/env python3
"""
MotionFX OBS-WebSocket v5 Client
=================================
A standalone Python client to automate MotionFX transitions over OBS-WebSocket v5.

Features:
- Connect & authenticate with OBS Studio 28+ / 31+
- List Scene Items & Scene Names
- Trigger MotionFX Show / Hide / Toggle animations remotely
- Can be integrated into Twitch bots, Discord bots, Streamer.bot, or custom webapps.

Requirements:
    pip install websocket-client
    (Or runs with standard python if websocket-client is installed)
"""

import sys
import json
import time
import base64
import hashlib
import uuid

try:
    import websocket
except ImportError:
    print("[Error] 'websocket-client' is required. Run: pip install websocket-client")
    websocket = None


class MotionFXWebSocketClient:
    def __init__(self, host="localhost", port=4455, password=""):
        self.url = f"ws://{host}:{port}"
        self.password = password
        self.ws = None
        self.authenticated = False

    def connect(self):
        if not websocket:
            raise RuntimeError("websocket-client module not installed.")

        print(f"[MotionFX-WS] Connecting to {self.url}...")
        self.ws = websocket.create_connection(self.url, timeout=10)

        # 1. Receive Hello (op 0)
        raw = self.ws.recv()
        hello = json.loads(raw)
        if hello.get("op") != 0:
            raise RuntimeError(f"Unexpected initial message: {hello}")

        # 2. Authenticate (Identify - op 1)
        identify_data = {
            "rpcVersion": 1,
            "eventSubscriptions": 33  # General + SceneItemEnableStateChanged
        }

        auth_data = hello.get("d", {}).get("authentication")
        if auth_data and self.password:
            challenge = auth_data["challenge"]
            salt = auth_data["salt"]

            # OBS SHA256 auth formula: base64(sha256(base64(sha256(secret + salt)) + challenge))
            secret_hash = base64.b64encode(hashlib.sha256((self.password + salt).encode('utf-8')).digest()).decode('utf-8')
            auth_response = base64.b64encode(hashlib.sha256((secret_hash + challenge).encode('utf-8')).digest()).decode('utf-8')
            identify_data["authentication"] = auth_response

        self.ws.send(json.dumps({"op": 1, "d": identify_data}))

        # 3. Receive Identified (op 2)
        raw_identified = self.ws.recv()
        identified = json.loads(raw_identified)
        if identified.get("op") == 2:
            self.authenticated = True
            print("[MotionFX-WS] Successfully connected and authenticated!")
        else:
            raise RuntimeError(f"Authentication failed: {identified}")

    def call(self, request_type, request_data=None):
        """Send a request to OBS WebSocket v5 and await response."""
        req_id = str(uuid.uuid4())
        msg = {
            "op": 6,
            "d": {
                "requestType": request_type,
                "requestId": req_id,
                "requestData": request_data or {}
            }
        }
        self.ws.send(json.dumps(msg))

        while True:
            resp_raw = self.ws.recv()
            resp = json.loads(resp_raw)
            # Response (op 7)
            if resp.get("op") == 7 and resp.get("d", {}).get("requestId") == req_id:
                status = resp["d"].get("requestStatus", {})
                if status.get("result"):
                    return resp["d"].get("responseData")
                else:
                    code = status.get("code")
                    comment = status.get("comment", "Unknown error")
                    raise RuntimeError(f"OBS WebSocket Error {code}: {comment}")

    def get_scene_item_id(self, scene_name, source_name):
        """Helper to get sceneItemId from sceneName and sourceName."""
        data = self.call("GetSceneItemId", {
            "sceneName": scene_name,
            "sourceName": source_name
        })
        return data.get("sceneItemId")

    def show_item(self, scene_name, source_name_or_id):
        """
        Triggers MotionFX Show Transition on a scene item.
        """
        if isinstance(source_name_or_id, str):
            item_id = self.get_scene_item_id(scene_name, source_name_or_id)
        else:
            item_id = source_name_or_id

        print(f"[MotionFX] Playing SHOW transition on item {item_id} in '{scene_name}'")
        return self.call("SetSceneItemEnabled", {
            "sceneName": scene_name,
            "sceneItemId": item_id,
            "sceneItemEnabled": True
        })

    def hide_item(self, scene_name, source_name_or_id):
        """
        Triggers MotionFX Hide Transition (Reverse Animation) on a scene item.
        """
        if isinstance(source_name_or_id, str):
            item_id = self.get_scene_item_id(scene_name, source_name_or_id)
        else:
            item_id = source_name_or_id

        print(f"[MotionFX] Playing HIDE (reverse) transition on item {item_id} in '{scene_name}'")
        return self.call("SetSceneItemEnabled", {
            "sceneName": scene_name,
            "sceneItemId": item_id,
            "sceneItemEnabled": False
        })

    def toggle_item(self, scene_name, source_name_or_id):
        """
        Toggles visibility of a scene item, triggering MotionFX Show or Hide transition.
        """
        if isinstance(source_name_or_id, str):
            item_id = self.get_scene_item_id(scene_name, source_name_or_id)
        else:
            item_id = source_name_or_id

        # Query current state
        status = self.call("GetSceneItemEnabled", {
            "sceneName": scene_name,
            "sceneItemId": item_id
        })
        is_enabled = status.get("sceneItemEnabled", False)

        if is_enabled:
            return self.hide_item(scene_name, item_id)
        else:
            return self.show_item(scene_name, item_id)

    def close(self):
        if self.ws:
            self.ws.close()
            print("[MotionFX-WS] Connection closed.")


# ============================================================================
# INTERACTIVE CLI DEMO
# ============================================================================

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage:")
        print("  python obs_websocket_client.py <SceneName> <SourceName> [show|hide|toggle] [password]")
        print("\nExamples:")
        print("  python obs_websocket_client.py 'Gaming Scene' 'Webcam' toggle")
        print("  python obs_websocket_client.py 'Main Scene' 'AlertBox' show myObsPassword")
        sys.exit(0)

    scene = sys.argv[1]
    source = sys.argv[2]
    action = sys.argv[3] if len(sys.argv) > 3 else "toggle"
    pwd = sys.argv[4] if len(sys.argv) > 4 else ""

    client = MotionFXWebSocketClient(host="localhost", port=4455, password=pwd)
    try:
        client.connect()
        if action == "show":
            client.show_item(scene, source)
        elif action == "hide":
            client.hide_item(scene, source)
        else:
            client.toggle_item(scene, source)
    finally:
        client.close()
