from dataclasses import dataclass, field
from typing import Optional, Dict, Any

@dataclass
class AnimationState:
    scene: str
    item_id: int
    status: str
    transition: str
    progress: float
    transform_2d: Dict[str, float] = field(default_factory=dict)
    transform_3d: Dict[str, float] = field(default_factory=dict)
    effects: Dict[str, float] = field(default_factory=dict)

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "AnimationState":
        return cls(
            scene=data.get("scene", ""),
            item_id=data.get("item_id", 0),
            status=data.get("status", "unknown"),
            transition=data.get("transition", "none"),
            progress=float(data.get("progress", 0.0)),
            transform_2d=data.get("transform_2d", {}),
            transform_3d=data.get("transform_3d", {}),
            effects=data.get("effects", {})
        )

@dataclass
class AnimationEvent:
    event_type: str
    scene_name: str
    item_id: int
    source_name: str
    transition_type: str
    progress: float
    timestamp_ms: float
    status: str

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "AnimationEvent":
        return cls(
            event_type=data.get("event_type", ""),
            scene_name=data.get("scene_name", ""),
            item_id=int(data.get("item_id", 0)),
            source_name=data.get("source_name", ""),
            transition_type=data.get("transition_type", ""),
            progress=float(data.get("progress", 0.0)),
            timestamp_ms=float(data.get("timestamp_ms", 0.0)),
            status=data.get("status", "")
        )

@dataclass
class PresetInfo:
    name: str
    description: str
    target: str
    duration_ms: float
    is_builtin: bool
