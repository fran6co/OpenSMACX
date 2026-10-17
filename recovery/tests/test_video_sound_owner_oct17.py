"""Exact source split and durable annotation continuity for VideoSoundState.

This validates the current split against the immutable pre-split source;
it does not measure
the native compiler, prove linked selection, or claim game/runtime acceptance.
"""
from __future__ import annotations

import re
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "95e6bb71f2bfb0f2d9859c2b06f8cdc9c595e9b4"
VIDEO_INCLUDE = '#include "video.h"\n'
METHOD_START = "// ORIGINAL: 0x004C8620 sub_4c8620 "
QUEUE_START = "// ORIGINAL: 0x0063E520 sub_63e520 "
DELETE_START = "// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z "
GET_TIME_OLD = "src/video.cpp owns the actual int VideoSoundState::get_time() method body"
GET_TIME_NEW = "src/video_sound.cpp owns the actual int VideoSoundState::get_time() method body"
METHODS = (
    "VideoSoundState::VideoSoundState()",
    "VideoSoundState::~VideoSoundState()",
    "int VideoSoundState::unload()",
    "int VideoSoundState::stop()",
    "int VideoSoundState::release()",
    "int VideoSoundState::get_game_hwnd()",
    "int VideoSoundState::set_attrib(unsigned long)",
    "int VideoSoundState::get_attrib()",
    "int VideoSoundState::get_time()",
    "int VideoSoundState::configure_streams(const void *audio_stream,",
    "int VideoSoundState::set_target_frame(int frame)",
    "void *VideoSoundState::acquire_audio_buffer(int bytes)",
    "int VideoSoundState::submit_audio_buffer(void *record)",
    "int VideoSoundState::current_frame()",
    "int VideoSoundState::frame_status()",
)


def _git_source(relative: str) -> str:
    return subprocess.check_output(
        ["git", "show", f"{BASE}:{relative}"], cwd=ROOT, text=True
    )


def project_video_sound_split(video_source: str, sound_source: str) -> str:
    """Reconstruct the old video.cpp, undoing only the authorized TU move.

    All moved code/annotations are taken from the new owner, not silently
    replaced with baseline text. The only normalization is the single scoped
    current-owner sentence in the get_time RECOVERY_BINDING.
    """
    old = _git_source("src/video.cpp")
    heading_end = old.index(VIDEO_INCLUDE)
    old_heading = old[:heading_end]
    method_start = old.index(METHOD_START)
    queue_start = old.index(QUEUE_START)
    delete_at = old.rindex(DELETE_START)
    delete_separator_at = old.rfind("\n\n", 0, delete_at)
    assert delete_separator_at == delete_at - 2
    delete_record = old[delete_separator_at:]

    assert video_source.startswith(VIDEO_INCLUDE), "video.cpp header moved or rewritten"
    assert sound_source.startswith(old_heading + VIDEO_INCLUDE), (
        "vtable/class-owner header or shared video.h include changed"
    )
    assert sound_source.count(METHOD_START) == 1
    assert sound_source.count(DELETE_START) == 1
    assert sound_source.endswith(delete_record), (
        "scalar deleting-destructor identity/span/claim changed or was displaced"
    )

    sound_method_start = sound_source.index(METHOD_START)
    assert sound_source[:sound_method_start] == old_heading + VIDEO_INCLUDE + "\n", (
        "unadmitted code or annotation before the moved implementation block"
    )
    sound_delete_at = sound_source.rindex(DELETE_START)
    sound_delete_separator_at = sound_source.rfind("\n\n", 0, sound_delete_at)
    assert sound_delete_separator_at == sound_delete_at - 2
    assert sound_source[sound_delete_separator_at:] == delete_record
    sound_delete_start = sound_delete_at
    moved_block = sound_source[sound_method_start:sound_delete_start]
    assert moved_block.count(GET_TIME_NEW) == 1
    moved_block = moved_block.replace(GET_TIME_NEW, GET_TIME_OLD, 1)

    # Reinsert the actual migrated method/annotation block immediately before
    # the retained first VideoFrameQueue record. Keep the old 4C9360 tail.
    assert video_source.count(QUEUE_START) == 1
    projected = old_heading + video_source
    queue_at = projected.index(QUEUE_START)
    projected = projected[:queue_at] + moved_block + projected[queue_at:]
    assert projected.endswith("\n")
    projected = projected[:-1] + delete_record
    return projected


def _assert_full_split(video_source: str, sound_source: str) -> str:
    old = _git_source("src/video.cpp")
    projected = project_video_sound_split(video_source, sound_source)
    assert projected == old, "split does not reconstruct the immutable pre-split TU"

    # Exercise the package parser on both new owners and the reconstructed
    # source so malformed/ambiguous annotations cannot hide behind text checks.
    old_rows = read_text(old, ROOT / "src/video.cpp")
    restored_rows = read_text(projected, ROOT / "src/video.cpp")
    sound_rows = read_text(sound_source, ROOT / "src/video_sound.cpp")
    assert len(old_rows) == len(restored_rows) > 0
    assert len(sound_rows) > 0

    assert sound_source.count("// RECOVERY_GENERATED: v1 vtable 0x66e790 0x66e82c ") == 1
    assert "??_7VideoSoundState@@6B@ 0x004c8620" in sound_source
    assert "??_7VideoSoundState@@6B@" not in video_source
    assert sound_source.count("// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z") == 1
    assert "??_GVideoSoundState@@UAEPAXI@Z" in sound_source
    assert "??_GVideoSoundState@@UAEPAXI@Z" not in video_source
    assert sound_source.count("RECOVERY_SHARED:") == 4

    for signature in METHODS:
        assert len(re.findall(r"(?m)^" + re.escape(signature), sound_source)) == 1, signature
        assert signature not in video_source, signature
    for retained in (
        "VideoFrameQueue::VideoFrameQueue()",
        "VideoFrameQueue::allocate(",
        "Video::advance_frame()",
        "VideoRegistry[10]",
        "VideoViewportCodecParameters VideoViewportCodec;",
    ):
        assert retained in video_source, retained
    return projected


def test_split_is_complete_unique_and_reconstructs_pre_split_source():
    video_source = (ROOT / "src/video.cpp").read_text()
    sound_source = (ROOT / "src/video_sound.cpp").read_text()
    _assert_full_split(video_source, sound_source)


@pytest.mark.parametrize("damage", (
    "drop_shared_unload",
    "change_original_span",
    "erase_old_lesson",
    "duplicate_vtable_record",
    "damage_deleting_mapping",
    "move_video_queue_record",
    "unadmitted_prologue",
))
def test_split_projection_rejects_moved_record_damage(damage):
    video_source = (ROOT / "src/video.cpp").read_text()
    sound_source = (ROOT / "src/video_sound.cpp").read_text()
    if damage == "drop_shared_unload":
        marker = "// RECOVERY_SHARED: v1 0x406b30 0x406b33 ?unload@VideoSoundState@@UAEHXZ 0x4c8620\n"
        assert sound_source.count(marker) == 1
        sound_source = sound_source.replace(marker, "", 1)
    elif damage == "change_original_span":
        old = "// ORIGINAL: 0x004C8890 sub_4c8890 0x004C8890-0x004C88AC FILE BYTE_EXACT"
        assert sound_source.count(old) == 1
        sound_source = sound_source.replace(old, old.replace("004C88AC", "004C88AD"), 1)
    elif damage == "erase_old_lesson":
        old = "Not a lever - it names why the remainder cannot close from here."
        assert sound_source.count(old) == 1
        sound_source = sound_source.replace(old, "Altered lesson history.", 1)
    elif damage == "duplicate_vtable_record":
        marker = "// RECOVERY_GENERATED: v1 vtable 0x66e790 0x66e82c ??_7VideoSoundState@@6B@ 0x004c8620\n"
        assert sound_source.count(marker) == 1
        sound_source = sound_source.replace(marker, marker + marker, 1)
    elif damage == "damage_deleting_mapping":
        old = "// symbol    ??_GVideoSoundState@@UAEPAXI@Z"
        assert sound_source.count(old) == 1
        sound_source = sound_source.replace(old, "// symbol    ??_GVideo@@UAEPAXI@Z", 1)
    elif damage == "move_video_queue_record":
        marker = "// ORIGINAL: 0x0063E520 sub_63e520 0x0063E520-0x0063E534 BYTE_EXACT"
        assert video_source.count(marker) == 1
        video_source = video_source.replace(marker, "// damaged queue identity", 1)
    elif damage == "unadmitted_prologue":
        assert sound_source.count(VIDEO_INCLUDE) == 1
        sound_source = sound_source.replace(
            VIDEO_INCLUDE, VIDEO_INCLUDE + "int unintended_product_storage;\n", 1)
    else:
        raise AssertionError(damage)

    with pytest.raises(AssertionError):
        _assert_full_split(video_source, sound_source)
