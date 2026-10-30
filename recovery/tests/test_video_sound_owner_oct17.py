"""VideoSoundState-owned transfer and durable annotation continuity.

This validates the moved VideoSoundState block against its immutable source;
it does not freeze unrelated Video methods or measure native selection.
It does not claim game/runtime acceptance.
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
GET_TIME_NEW = "src/sound.cpp owns the actual int VideoSoundState::get_time() method body"
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


def _base_video_sound_block() -> tuple[str, str, str]:
    """Return the immutable migrated implementation and its old file framing."""
    old = _git_source("src/video.cpp")
    heading_end = old.index(VIDEO_INCLUDE)
    old_heading = old[:heading_end]
    method_start = old.index(METHOD_START)
    queue_start = old.index(QUEUE_START)
    delete_at = old.rindex(DELETE_START)
    delete_separator_at = old.rfind("\n\n", 0, delete_at)
    assert delete_separator_at == delete_at - 2
    delete_record = old[delete_separator_at:]
    assert method_start < queue_start < delete_at
    block = old[method_start:queue_start]
    return old_heading, block, delete_record


def project_video_sound_split(video_source: str, sound_source: str) -> str:
    """Validate the byte-preserving move from video.cpp into the audio TU."""
    old_heading, expected_block, delete_record = _base_video_sound_block()

    assert video_source.startswith(VIDEO_INCLUDE), "video.cpp header moved or rewritten"
    assert sound_source.count(old_heading) == 1, "moved VideoSound section heading changed or duplicated"
    section_start = sound_source.index(old_heading)
    voice_start = sound_source.find(
        "// RECOVERY_GENERATED: v1 vtable 0x66e82c 0x66e8c4 ??_7VoiceRx@@6B@",
        section_start,
    )
    assert voice_start >= 0, "merged video section is not followed by the retained voice section"
    moved_section = sound_source[section_start:voice_start]
    expected_section = old_heading + VIDEO_INCLUDE + "\n" + expected_block + delete_record

    assert moved_section.count(METHOD_START) == 1
    assert moved_section.count(DELETE_START) == 1
    assert moved_section.count(GET_TIME_NEW) == 1
    normalized_section = moved_section.replace(GET_TIME_NEW, GET_TIME_OLD, 1)
    assert normalized_section == expected_section, (
        "VideoSound section prologue, implementation, deletion mapping or footer changed"
    )
    moved_block = moved_section[len(old_heading + VIDEO_INCLUDE + "\n"):]
    moved_block = moved_block[:moved_block.index(DELETE_START)].rstrip()
    moved_block = moved_block.replace(GET_TIME_NEW, GET_TIME_OLD, 1)
    assert moved_block == expected_block, "VideoSound implementation/annotation transfer changed"
    return moved_block


def _assert_full_split(video_source: str, sound_source: str) -> str:
    projected = project_video_sound_split(video_source, sound_source)

    # Exercise the package parser on the owner and the migrated block so
    # malformed/ambiguous annotations cannot hide behind text comparisons.
    old_heading, _, _ = _base_video_sound_block()
    old = _git_source("src/video.cpp")
    old_rows = read_text(old, ROOT / "src/video.cpp")
    migrated_rows = read_text(old_heading + VIDEO_INCLUDE + "\n" + projected,
                              ROOT / "src/sound.cpp")
    sound_rows = read_text(sound_source, ROOT / "src/sound.cpp")
    old_sound_rows = [row for row in old_rows if row.address in {
        record.address for record in migrated_rows
    }]
    assert [row.address for row in old_sound_rows] == [row.address for row in migrated_rows]
    assert [row.address for row in sound_rows if row.address in {
        record.address for record in migrated_rows
    }] == [row.address for row in migrated_rows]
    assert len(sound_rows) > 0

    assert sound_source.count("// RECOVERY_GENERATED: v1 vtable 0x66e790 0x66e82c ") == 1
    assert "??_7VideoSoundState@@6B@ 0x004c8620" in sound_source
    assert "??_7VideoSoundState@@6B@" not in video_source
    assert sound_source.count("// ORIGINAL: 0x004C9360 ??_GVideo@@UAEPAXI@Z") == 1
    assert "??_GVideoSoundState@@UAEPAXI@Z" in sound_source
    assert "??_GVideoSoundState@@UAEPAXI@Z" not in video_source
    assert projected.count("RECOVERY_SHARED:") == 4

    for signature in METHODS:
        assert len(re.findall(r"(?m)^" + re.escape(signature), sound_source)) == 1, signature
        assert signature not in video_source, signature
    for retained in (
        "Video::advance_frame()",
        "VideoRegistry[10]",
        "VideoViewportCodecParameters VideoViewportCodec;",
    ):
        assert retained in video_source, retained
    return projected


def test_video_sound_owner_is_complete_unique_and_matches_migrated_block():
    video_source = (ROOT / "src/video.cpp").read_text()
    sound_source = (ROOT / "src/sound.cpp").read_text()
    _assert_full_split(video_source, sound_source)


@pytest.mark.parametrize("damage", (
    "drop_shared_unload",
    "change_original_span",
    "erase_old_lesson",
    "duplicate_vtable_record",
    "damage_deleting_mapping",
    "damage_video_sound_method",
    "unadmitted_prologue",
))
def test_split_projection_rejects_moved_record_damage(damage):
    video_source = (ROOT / "src/video.cpp").read_text()
    sound_source = (ROOT / "src/sound.cpp").read_text()
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
    elif damage == "damage_video_sound_method":
        marker = "int VideoSoundState::get_time() {"
        assert sound_source.count(marker) == 1
        sound_source = sound_source.replace(marker, "int VideoSoundState::changed_time()", 1)
    elif damage == "unadmitted_prologue":
        assert sound_source.count(METHOD_START) == 1
        sound_source = sound_source.replace(
            METHOD_START, "int unintended_product_storage;\n" + METHOD_START, 1)
    else:
        raise AssertionError(damage)

    with pytest.raises(AssertionError):
        _assert_full_split(video_source, sound_source)
