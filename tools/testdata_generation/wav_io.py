"""PCM24 WAV encoding and file-integrity helpers for TESTDATA-001."""

from __future__ import annotations

import hashlib
import math
from pathlib import Path
import struct
import wave

from .specs import CHANNELS, PCM_BITS, SAMPLE_RATE


def validate_float_frames(identifier: str, frames: list[tuple[float, float]]) -> None:
    for frame_index, frame in enumerate(frames):
        if len(frame) != CHANNELS:
            raise ValueError(f"{identifier}: frame {frame_index} does not have two channels")
        for channel_index, value in enumerate(frame):
            if not math.isfinite(value):
                raise ValueError(f"{identifier}: non-finite sample at {frame_index}:{channel_index}")
            if abs(value) > 1.0:
                raise ValueError(
                    f"{identifier}: sample out of range at {frame_index}:{channel_index}: {value}"
                )


def _quantize_pcm24(value: float) -> int:
    if not math.isfinite(value) or abs(value) > 1.0:
        raise ValueError(f"cannot quantize out-of-range sample: {value}")
    if value == -1.0:
        return -8_388_608
    quantized = round(value * 8_388_607.0)
    if not -8_388_608 <= quantized <= 8_388_607:
        raise ValueError(f"PCM24 quantization overflow: {value}")
    return quantized


def _pack_pcm24(value: int) -> bytes:
    return struct.pack("<i", value)[:3]


def write_wav(
    path: Path, frames: list[tuple[float, float]], sample_rate: int = SAMPLE_RATE
) -> None:
    validate_float_frames(path.stem, frames)
    pcm = bytearray()
    for left, right in frames:
        pcm.extend(_pack_pcm24(_quantize_pcm24(left)))
        pcm.extend(_pack_pcm24(_quantize_pcm24(right)))
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as output:
        output.setnchannels(CHANNELS)
        output.setsampwidth(PCM_BITS // 8)
        output.setframerate(sample_rate)
        output.writeframes(bytes(pcm))


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()
