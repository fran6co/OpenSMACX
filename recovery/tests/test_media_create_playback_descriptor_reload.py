"""Execute playback-buffer owner reloads across controlled COM callbacks."""
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_movie_media_behavior import generated_source


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/movie_media.cpp"
SIGNATURE = "int MediaRuntime::create_playback_buffer()"


def projected_program(edits=()):
    """Use the existing test's balanced extractor and complete host fixture."""
    text = generated_source(SOURCE.read_text(), edits)

    # Add opt-in callback controls to MockSound; existing fixture cases leave
    # these null and keep their original behavior.
    field_anchor = "  MockBuffer *buffer;\n  MediaRuntime *frequency_owner;"
    field_replacement = (
        "  MockBuffer *buffer;\n"
        "  MediaRuntime *playback_runtime;\n"
        "  MediaDescriptor *descriptor_after_create_buffer;\n"
        "  IDirectSoundBuffer **expected_playback_slot;\n"
        "  MediaRuntime *frequency_owner;"
    )
    assert text.count(field_anchor) == 1
    text = text.replace(field_anchor, field_replacement, 1)

    buffer_field_anchor = "  int releases, caps_calls;"
    buffer_field_replacement = (
        "  int releases, caps_calls, play_calls;\n"
        "  MediaRuntime *descriptor_runtime;\n"
        "  MediaDescriptor *after_frequency, *after_volume, *after_caps, "
        "*after_play, *after_release;"
    )
    assert text.count(buffer_field_anchor) == 1
    text = text.replace(buffer_field_anchor, buffer_field_replacement, 1)
    buffer_init_anchor = "releases(0), caps_calls(0) {}"
    buffer_init_replacement = (
        "releases(0), caps_calls(0), play_calls(0), descriptor_runtime(0), "
        "after_frequency(0), after_volume(0), after_caps(0), after_play(0), "
        "after_release(0) {}"
    )
    assert text.count(buffer_init_anchor) == 1
    text = text.replace(buffer_init_anchor, buffer_init_replacement, 1)
    frequency_anchor = "sound_event('f'); frequency = value; return frequency_result;"
    frequency_replacement = (
        "sound_event('f'); frequency = value; "
        "if (descriptor_runtime && after_frequency) descriptor_runtime->descriptor = after_frequency; "
        "return frequency_result;"
    )
    assert text.count(frequency_anchor) == 1
    text = text.replace(frequency_anchor, frequency_replacement, 1)
    volume_anchor = "sound_event('v'); volume = value; return volume_result;"
    volume_replacement = (
        "sound_event('v'); volume = value; "
        "if (descriptor_runtime && after_volume) descriptor_runtime->descriptor = after_volume; "
        "return volume_result;"
    )
    assert text.count(volume_anchor) == 1
    text = text.replace(volume_anchor, volume_replacement, 1)
    caps_anchor = "caps->dwBufferBytes = actual_bytes;\n    return caps_result;"
    caps_replacement = (
        "caps->dwBufferBytes = actual_bytes;\n"
        "    if (descriptor_runtime && after_caps) descriptor_runtime->descriptor = after_caps;\n"
        "    return caps_result;"
    )
    assert text.count(caps_anchor) == 1
    text = text.replace(caps_anchor, caps_replacement, 1)
    play_anchor = "HRESULT Play(DWORD, DWORD, DWORD) { sound_event('p'); return play_result; }"
    play_replacement = (
        "HRESULT Play(DWORD, DWORD, DWORD) { sound_event('p'); ++play_calls; "
        "if (descriptor_runtime && after_play) descriptor_runtime->descriptor = after_play; "
        "return play_result; }"
    )
    assert text.count(play_anchor) == 1
    text = text.replace(play_anchor, play_replacement, 1)
    release_anchor = "unsigned long Release() { sound_event('r'); ++releases; return releases; }"
    release_replacement = (
        "unsigned long Release() { sound_event('r'); ++releases; "
        "if (descriptor_runtime && after_release) descriptor_runtime->descriptor = after_release; "
        "return releases; }"
    )
    assert text.count(release_anchor) == 1
    text = text.replace(release_anchor, release_replacement, 1)

    init_anchor = "buffer(0), frequency_owner(0),"
    init_replacement = (
        "buffer(0), playback_runtime(0), descriptor_after_create_buffer(0), "
        "expected_playback_slot(0), frequency_owner(0),"
    )
    assert text.count(init_anchor) == 1
    text = text.replace(init_anchor, init_replacement, 1)

    call_anchor = "    if (create_result == DS_OK) *out = buffer;\n    return create_result;"
    call_replacement = (
        "    if (playback_runtime != 0 && expected_playback_slot != 0)\n"
        "      assert(out == expected_playback_slot);\n"
        "    if (create_result == DS_OK) *out = buffer;\n"
        "    if (playback_runtime != 0 && descriptor_after_create_buffer != 0)\n"
        "      playback_runtime->descriptor = descriptor_after_create_buffer;\n"
        "    return create_result;"
    )
    assert text.count(call_anchor) == 1
    text = text.replace(call_anchor, call_replacement, 1)

    test_body = r'''static void test_create_playback_buffer_reloads_after_each_success_callback() {
  MockSound entry_sound;
  MockBuffer created_into_entry_slot, after_frequency_buffer,
             after_volume_buffer, after_caps_buffer, after_play_buffer;
  MediaDescriptor entry = {}, after_create = {}, after_frequency = {},
                  after_volume = {}, after_caps = {}, after_play = {};
  entry.sound_device = &entry_sound;
  entry.flags = 0x00002000;
  entry.volume_argument = -111;
  after_create.flags = 0x00002000;
  after_create.play_buffer = &after_frequency_buffer;
  after_frequency.flags = 0x00002000;
  after_frequency.play_buffer = &after_volume_buffer;
  after_frequency.volume_argument = -321;
  after_volume.play_buffer = &after_caps_buffer;
  after_caps.play_buffer = &after_play_buffer;

  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.playback_frequency = 22050;
  runtime.source_buffer_bytes = 0x6000;
  entry_sound.buffer = &created_into_entry_slot;
  entry_sound.playback_runtime = &runtime;
  entry_sound.descriptor_after_create_buffer = &after_create;
  entry_sound.expected_playback_slot = &entry.play_buffer;
  after_frequency_buffer.descriptor_runtime = &runtime;
  after_frequency_buffer.after_frequency = &after_frequency;
  after_volume_buffer.descriptor_runtime = &runtime;
  after_volume_buffer.after_volume = &after_volume;
  after_caps_buffer.descriptor_runtime = &runtime;
  after_caps_buffer.after_caps = &after_caps;
  after_play_buffer.descriptor_runtime = &runtime;
  after_play_buffer.after_play = &after_play;

  reset_sound_events();
  assert(runtime.create_playback_buffer() == 0);
  // CreateSoundBuffer writes through the entry descriptor's output slot.
  assert(entry.play_buffer == &created_into_entry_slot);
  // Each next operation uses the descriptor selected by the previous callback.
  assert(after_frequency_buffer.frequency == 22050);
  assert(after_volume_buffer.volume == -321);
  assert(after_caps_buffer.caps_calls == 1);
  assert(after_play_buffer.play_calls == 1);
  assert(created_into_entry_slot.frequency == 0 && created_into_entry_slot.caps_calls == 0);
  assert(runtime.descriptor == &after_play);
  assert(runtime.playback_buffer_bytes == after_caps_buffer.actual_bytes);
  assert(runtime.buffer_setup_finished == 1);
  assert(sound_event_count == 5);
  assert(sound_events[0] == 'C' && sound_events[1] == 'f' &&
         sound_events[2] == 'v' && sound_events[3] == 'b' &&
         sound_events[4] == 'p');
}

static void test_create_playback_buffer_failure_clears_post_release_owner() {
  MockSound entry_sound;
  MockBuffer created_into_entry_slot, before_play, released_buffer, cleared_owner_buffer;
  MediaDescriptor entry = {}, after_create = {}, after_play = {},
                  after_release = {};
  entry.sound_device = &entry_sound;
  after_create.flags = 0; // SetVolume is skipped on the live post-frequency descriptor.
  after_create.play_buffer = &before_play;
  after_play.play_buffer = &released_buffer;
  after_release.play_buffer = &cleared_owner_buffer;
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  runtime.playback_frequency = 11025;
  entry_sound.buffer = &created_into_entry_slot;
  entry_sound.playback_runtime = &runtime;
  entry_sound.descriptor_after_create_buffer = &after_create;
  entry_sound.expected_playback_slot = &entry.play_buffer;
  before_play.descriptor_runtime = &runtime;
  before_play.after_caps = &after_play;
  released_buffer.play_result = -1;
  released_buffer.descriptor_runtime = &runtime;
  released_buffer.after_play = &after_play;
  released_buffer.after_release = &after_release;

  reset_sound_events();
  assert(runtime.create_playback_buffer() == 0);
  assert(runtime.descriptor == &after_release);
  assert(entry.play_buffer == &created_into_entry_slot);
  assert(released_buffer.play_calls == 1 && released_buffer.releases == 1);
  assert(after_play.play_buffer == &released_buffer);
  // Release replaces the descriptor; the following clear belongs to that new owner.
  assert(after_release.play_buffer == 0);
  assert(cleared_owner_buffer.frequency == 0 && cleared_owner_buffer.releases == 0);
  assert(runtime.synthetic_cursor_mode == 1 && runtime.synthetic_playback_cursor == 0);
  assert(runtime.playback_buffer_bytes == 0x8000 && runtime.buffer_setup_finished == 1);
  assert(sound_event_count == 5);
  assert(sound_events[0] == 'C' && sound_events[1] == 'f' &&
         sound_events[2] == 'b' && sound_events[3] == 'p' &&
         sound_events[4] == 'r');
}

'''
    main_anchor = "int main() {\n"
    assert text.count(main_anchor) == 1
    text = text.replace(main_anchor, test_body + main_anchor, 1)
    call_anchor = "  test_optional_source_buffer_is_not_playback_rate();\n"
    assert text.count(call_anchor) == 1
    text = text.replace(
        call_anchor,
        call_anchor + "  test_create_playback_buffer_reloads_after_each_success_callback();\n"
        "  test_create_playback_buffer_failure_clears_post_release_owner();\n",
        1,
    )
    return text


def compile_and_run(tmp_path, compiler_name, edits=()):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = tmp_path / "media_create_playback.cpp"
    executable = tmp_path / "media_create_playback"
    source.write_text(projected_program(edits))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_create_playback_buffer_uses_descriptor_current_after_create_callback(tmp_path, compiler_name):
    result = compile_and_run(tmp_path, compiler_name)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_cached_entry_descriptor_damage_is_detected(tmp_path, compiler_name):
    edits = (
        (
            "int MediaRuntime::create_playback_buffer() {",
            "int MediaRuntime::create_playback_buffer() {\n"
            "  MediaDescriptor *entry_owner = descriptor;",
        ),
        (
            "descriptor->play_buffer->SetFrequency(playback_frequency);",
            "entry_owner->play_buffer->SetFrequency(playback_frequency);",
        ),
    )
    result = compile_and_run(tmp_path, compiler_name, edits)
    assert result.returncode != 0, result.stdout + result.stderr
