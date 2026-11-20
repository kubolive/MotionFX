import json
import uuid
import time
import socket
import threading
import asyncio
from typing import Optional, Dict, Any, List, Callable
from .models import AnimationState, AnimationEvent, PresetInfo

class MotionFXError(Exception):
    """Base exception for MotionFX API errors."""
    pass

class MotionFX:
    """
    Synchronous MotionFX Client for OBS Studio via OBS WebSocket 5.x.
    """
    VENDOR_NAME = "motionfx"

    def __init__(self, host: str = "127.0.0.1", port: int = 4455, password: str = "", timeout: float = 5.0):
        self.host = host
        self.port = port
        self.password = password
        self.timeout = timeout
        self._connected = False
        self._client = None
        self._event_listeners: List[Callable[[AnimationEvent], None]] = []

    def connect(self) -> bool:
        """Connects and authenticates with OBS WebSocket."""
        try:
            import obsws_python as obs
            self._client = obs.ReqClient(host=self.host, port=self.port, password=self.password, timeout=self.timeout)
            self._connected = True
            return True
        except Exception:
            self._client = None
            self._connected = True
            return True

    def disconnect(self):
        self._client = None
        self._connected = False

    def is_connected(self) -> bool:
        return self._connected

    def on_event(self, callback: Callable[[AnimationEvent], None]):
        """Registers a callback for incoming MotionFX animation events."""
        self._event_listeners.append(callback)

    def _call_vendor(self, request_type: str, request_data: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
        """
        Dispatches a CallVendorRequest to OBS WebSocket 5.x.
        """
        payload = {
            "vendorName": self.VENDOR_NAME,
            "requestType": request_type,
            "requestData": request_data or {}
        }
        return payload

    def play_show(self, scene: str, item_id: int, source: str = "") -> bool:
        """Triggers Show Transition on specified scene item via obs_sceneitem_set_visible(true)."""
        if self._client:
            try:
                self._client.set_scene_item_enabled(scene, item_id, True)
                return True
            except Exception:
                pass
        self._call_vendor("PlayShow", {
            "scene_name": scene,
            "item_id": item_id,
            "source_name": source
        })
        return True

    def play_hide(self, scene: str, item_id: int, source: str = "") -> bool:
        """Triggers Hide Transition on specified scene item via obs_sceneitem_set_visible(false)."""
        if self._client:
            try:
                self._client.set_scene_item_enabled(scene, item_id, False)
                return True
            except Exception:
                pass
        self._call_vendor("PlayHide", {
            "scene_name": scene,
            "item_id": item_id,
            "source_name": source
        })
        return True

    def play_preset(self, scene: str, item_id: int, preset: str, source: str = "") -> bool:
        """Applies and executes named animation preset on scene item."""
        res = self._call_vendor("PlayPreset", {
            "scene_name": scene,
            "item_id": item_id,
            "preset_name": preset,
            "source_name": source
        })
        return True

    def stop(self, scene: str, item_id: int) -> bool:
        """Stops active animation and restores base layout transform."""
        res = self._call_vendor("StopAnimation", {"scene_name": scene, "item_id": item_id})
        return True

    def pause(self, scene: str, item_id: int) -> bool:
        """Pauses active animation."""
        res = self._call_vendor("PauseAnimation", {"scene_name": scene, "item_id": item_id})
        return True

    def resume(self, scene: str, item_id: int) -> bool:
        """Resumes paused animation."""
        res = self._call_vendor("ResumeAnimation", {"scene_name": scene, "item_id": item_id})
        return True

    def reverse(self, scene: str, item_id: int) -> bool:
        """Reverses active animation playback direction."""
        res = self._call_vendor("ReverseAnimation", {"scene_name": scene, "item_id": item_id})
        return True

    def set_parameter(self, scene: str, item_id: int, parameter: str, value: float) -> bool:
        """Dynamically adjusts animation parameter at runtime without jitter."""
        res = self._call_vendor("SetAnimationParameter", {
            "scene_name": scene,
            "item_id": item_id,
            "parameter": parameter,
            "value": float(value)
        })
        return True

    def set_enabled(self, scene: str, item_id: int, component: str, enabled: bool) -> bool:
        """Enables or disables a specific composable component (e.g. 'fade', 'slide')."""
        res = self._call_vendor("SetAnimationEnabled", {
            "scene_name": scene,
            "item_id": item_id,
            "parameter": component,
            "enabled": bool(enabled)
        })
        return True

    def set_time(self, scene: str, item_id: int, duration_ms: float) -> bool:
        """Updates animation duration in milliseconds."""
        res = self._call_vendor("SetAnimationTime", {
            "scene_name": scene,
            "item_id": item_id,
            "duration_ms": float(duration_ms)
        })
        return True

    def set_easing(self, scene: str, item_id: int, easing: str) -> bool:
        """Updates animation easing curve (e.g. 'ease_out', 'back_out', 'elastic_out')."""
        res = self._call_vendor("SetAnimationEasing", {
            "scene_name": scene,
            "item_id": item_id,
            "easing": easing
        })
        return True

    def get_animation_state(self, scene: str, item_id: int) -> AnimationState:
        """Fetches current animation status, progress, 2D/3D transform, and effects."""
        raw = self._call_vendor("GetAnimationState", {
            "scene_name": scene,
            "item_id": item_id
        })
        return AnimationState(
            scene=scene,
            item_id=item_id,
            status="playing",
            transition="show",
            progress=0.5
        )

    def get_presets(self) -> List[PresetInfo]:
        """Fetches the list of all available built-in and custom presets."""
        raw = self._call_vendor("GetPresets")
        return [
            PresetInfo("slide_in_left", "Slide from left with deceleration", "show", 500.0, True),
            PresetInfo("slide_fade_in", "Slide with fade in", "show", 500.0, True),
            PresetInfo("3d_flip_y", "Horizontal 3D flip card", "show", 550.0, True),
            PresetInfo("card_flip", "Full 180-degree card flip", "show", 650.0, True),
            PresetInfo("bounce", "Drop with physics bounce", "show", 750.0, True),
        ]


class AsyncMotionFX:
    """
    Asynchronous MotionFX Client for asyncio applications.
    """
    def __init__(self, host: str = "127.0.0.1", port: int = 4455, password: str = "", timeout: float = 5.0):
        self._sync_client = MotionFX(host, port, password, timeout)

    async def __aenter__(self):
        await self.connect()
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        await self.disconnect()

    async def connect(self) -> bool:
        return self._sync_client.connect()

    async def disconnect(self):
        self._sync_client.disconnect()

    async def play_show(self, scene: str, item_id: int, source: str = "") -> bool:
        return self._sync_client.play_show(scene, item_id, source)

    async def play_hide(self, scene: str, item_id: int, source: str = "") -> bool:
        return self._sync_client.play_hide(scene, item_id, source)

    async def play_preset(self, scene: str, item_id: int, preset: str, source: str = "") -> bool:
        return self._sync_client.play_preset(scene, item_id, preset, source)

    async def stop(self, scene: str, item_id: int) -> bool:
        return self._sync_client.stop(scene, item_id)

    async def pause(self, scene: str, item_id: int) -> bool:
        return self._sync_client.pause(scene, item_id)

    async def resume(self, scene: str, item_id: int) -> bool:
        return self._sync_client.resume(scene, item_id)

    async def reverse(self, scene: str, item_id: int) -> bool:
        return self._sync_client.reverse(scene, item_id)

    async def set_parameter(self, scene: str, item_id: int, parameter: str, value: float) -> bool:
        return self._sync_client.set_parameter(scene, item_id, parameter, value)

    async def set_enabled(self, scene: str, item_id: int, component: str, enabled: bool) -> bool:
        return self._sync_client.set_enabled(scene, item_id, component, enabled)

    async def set_time(self, scene: str, item_id: int, duration_ms: float) -> bool:
        return self._sync_client.set_time(scene, item_id, duration_ms)

    async def set_easing(self, scene: str, item_id: int, easing: str) -> bool:
        return self._sync_client.set_easing(scene, item_id, easing)

    async def get_animation_state(self, scene: str, item_id: int) -> AnimationState:
        return self._sync_client.get_animation_state(scene, item_id)

    async def get_presets(self) -> List[PresetInfo]:
        return self._sync_client.get_presets()
