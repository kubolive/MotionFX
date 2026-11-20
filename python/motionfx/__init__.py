"""
MotionFX Python SDK for OBS Studio
Enables external programmatic control over MotionFX Scene Item Animation Engine via OBS WebSocket 5.x.
"""

from .client import MotionFX, AsyncMotionFX
from .models import AnimationState, AnimationEvent, PresetInfo

__version__ = "1.0.0"
__all__ = [
    "MotionFX",
    "AsyncMotionFX",
    "AnimationState",
    "AnimationEvent",
    "PresetInfo",
]
