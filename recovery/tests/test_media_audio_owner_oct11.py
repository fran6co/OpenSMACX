"""Separate descriptor buffer ownership from runtime parsed audio frequency."""
from pathlib import Path
import subprocess

import pytest

from recovery.tests.test_movie_media_behavior import build_and_run, definition

ROOT = Path(__file__).resolve().parents[2]
BASE = "c30c6facc4dbb2873f47ccf9acc468c32b86774f"
HEADER_BEFORE = """  // Native readers assign incompatible meanings to this same DWORD: setup
  // passes it as an optional source-buffer interface, while create-buffer
  // shifts it left twice for a sample/byte rate. Preserve raw storage and
  // provenance; callers' initialization has not yet explained the overlap.
  DWORD unresolved_source_or_rate_58;"""
HEADER_AFTER = """  // Setup5CE340 queries this optional buffer; teardown5CE450 releases it.
  // The parsed playback rate is MediaRuntime+58, a different object's field.
  IDirectSoundBuffer *source_buffer_58;          // +58; optional caller buffer"""


def without_audio_owner_functions(text):
    """Project only this separately bounded migration out of older checks."""
    for name in ("setup_sound_device", "create_playback_buffer", "release_session_interfaces"):
        signature = "int MediaRuntime::" + name + "()"
        text = text.replace(definition(text, signature), "AUDIO_OWNER " + name)
    from test_media_output_buffers_oct11 import without_renderer_body
    return without_renderer_body(text)


def restore_audio_header(text):
    from test_media_output_buffers_oct11 import restore_output_header
    text = restore_output_header(text)
    assert text.count(HEADER_AFTER) == 1
    return text.replace(HEADER_AFTER, HEADER_BEFORE)


def expect_movie_descriptor_reread_changes(text):
    """Apply exact reviewed bodies/loop to the prior source projection."""
    replacements = {
        "int MediaRuntime::release_session_interfaces()": '''int MediaRuntime::release_session_interfaces() {
  const DWORD descriptor_flags = descriptor->flags;
  if ((descriptor_flags & 0x00800000) != 0) {
    if (release_optional_buffer != 0) {
      MediaDescriptor *source_owner = descriptor;
      if (source_owner->source_buffer_58 != 0) {
        IDirectSoundBuffer *source_buffer = source_owner->source_buffer_58;
        source_buffer->Release();
        // Native 5CE475 reloads runtime->descriptor after Release.
        descriptor->source_buffer_58 = 0;
      }
    }
    if (release_sound_device != 0) {
      // This load follows the optional-buffer Release and therefore observes
      // any descriptor replacement performed by that call.
      MediaDescriptor *device_owner = descriptor;
      if (device_owner->sound_device != 0) {
        IDirectSound *sound_device = device_owner->sound_device;
        sound_device->Release();
        device_owner->sound_device = 0;
      }
    }
  }
  return 0;
}''',
        "int MediaRuntime::release_playback_buffer()": '''int MediaRuntime::release_playback_buffer() {
  MediaDescriptor *playback_owner = descriptor;
  if (playback_owner->play_buffer != 0) {
    IDirectSoundBuffer *playback_buffer = playback_owner->play_buffer;
    playback_buffer->Release();
    // Native 5CE632 reloads runtime->descriptor before the 5CE634 store.
    descriptor->play_buffer = 0;
  }
  return 0;
}''',
        "int MediaRuntime::sub_5cae00()": '''int MediaRuntime::sub_5cae00() {
  MediaDescriptor *stream_owner = descriptor;
  if ((stream_owner->flags & 0x00080000) == 0 && stream_owner->stream != 0)
    mmioClose(stream_owner->stream, 0);
  if (input_ring_base != 0) free(input_ring_base);
  if (decoded_ring_base != 0) free(decoded_ring_base);
  for (int i = 0; i != 24; ++i) read_records[i].payload = 0;
  // Native 5CAE58 reloads runtime->descriptor after close/free and record
  // clearing; the gate is read from that current descriptor.
  MediaDescriptor *output_owner = descriptor;
  if (output_owner->stream_gate_7c != 0)
    for (int j = 0; j != 24; ++j) output_records[j].payload = 0;
  return 0;
}''',
        'extern "C" char *__fastcall sub_5cf120(': '''extern "C" char *__fastcall sub_5cf120(int code) {
  for (unsigned int i = 0;
       i < sizeof(media_hresult_texts) / sizeof(media_hresult_texts[0]);
       ++i) {
    if (media_hresult_texts[i].result == code)
      return (char *)media_hresult_texts[i].text;
  }
  return (char *)media_hresult_unknown;
}''',
    }
    for signature, replacement in replacements.items():
        old_body = definition(text, signature)
        assert old_body != replacement
        text = text.replace(old_body, replacement, 1)
    return text


def test_exact_owner_migration_and_original_identity_continuity():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    for name in ("movie_media.cpp", "movie_media.h", "uv2player.cpp"):
        path = ROOT / "src" / name
        before = subprocess.check_output(["git", "show", BASE + ":src/" + name], cwd=ROOT, text=True)
        after = path.read_text()
        if name == "movie_media.h":
            assert before.count(HEADER_BEFORE) == 1
            expected = before.replace(HEADER_BEFORE, HEADER_AFTER)
            from test_media_output_buffers_oct11 import restore_output_header
            after = restore_output_header(after)
        elif name == "uv2player.cpp":
            expected = before.replace("media_.unresolved_source_or_rate_58 = 0;", "media_.source_buffer_58 = 0;")
            expected = expected.replace(
                "// DirectDrawSurface2 extends DirectDrawSurface in the SDK; Buffer's\n"
                "        // borrowed surface API consumes the shared interface prefix.",
                "// The SDK declares Surface2 separately, with the same Surface method\n"
                "        // prefix, not C++ inheritance. The original passes this borrowed\n"
                "        // Surface2 pointer unchanged to Buffer (4BF11D..4BF147); its borrowed\n"
                "        // path uses only shared slots. A QueryInterface here would change\n"
                "        // reference-count and failure behavior.")
            # The separately bounded Wave interface migration now carries
            # this pointer through its producer instead of an integer cast.
            after = after.replace("media_.sound_device = WaveDeviceGlobal.get_ds();",
                                  "media_.sound_device =\n        reinterpret_cast<IDirectSound *>(WaveDeviceGlobal.get_ds());")
        else:
            expected = before.replace("d.unresolved_source_or_rate_58", "d.source_buffer_58")
            for indent in ("  ", "      "):
                old = indent + "IDirectSoundBuffer *source_buffer =\n" + indent + "    reinterpret_cast<IDirectSoundBuffer *>(\n" + indent + "        d.source_buffer_58);"
                assert expected.count(old) == 1
                expected = expected.replace(old, indent + "IDirectSoundBuffer *source_buffer = d.source_buffer_58;")
            expected = expected.replace("format.nSamplesPerSec = d.source_buffer_58;", "format.nSamplesPerSec = playback_frequency;")
            expected = expected.replace("format.nAvgBytesPerSec = d.source_buffer_58 << 2;", "format.nAvgBytesPerSec = playback_frequency << 2;")
            expected = expected.replace("d.play_buffer->SetFrequency(d.source_buffer_58);", "d.play_buffer->SetFrequency(playback_frequency);")
        if name == "movie_media.cpp":
            from test_media_output_buffers_oct11 import without_renderer_body
            assert expect_movie_descriptor_reread_changes(
                without_renderer_body(expected)) == without_renderer_body(after)
        else:
            assert expected == after
        assert [(_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(before, path)] == [
            (_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(after, path)]


def test_native_distinct_receivers_and_frequency_producer():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    data = ROOT / ".opensmacx/game/terranx_original.exe"
    if not data.is_file():
        pytest.skip("pinned original required")
    image = PeImage(data.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for start, end in ((0x5CE4B0, 0x5CE61A), (0x5CE340, 0x5CE44B),
                       (0x5CE450, 0x5CE4A3), (0x5CA6C0, 0x5CA95B)):
        instructions.update({i.address: (i.mnemonic, i.op_str)
                             for i in decoder.disasm(image.read(Span(start, end)), start)})
    for address, expected in {
        0x5CE4B6: ("mov", "esi, ecx"),
        0x5CE4BF: ("mov", "eax, dword ptr [esi]"),
        0x5CE508: ("mov", "ecx, dword ptr [esi + 0x58]"),
        0x5CE520: ("shl", "ecx, 2"),
        0x5CE561: ("mov", "ecx, dword ptr [esi + 0x58]"),
        0x5CE568: ("call", "dword ptr [edx + 0x44]"),
        0x5CE3E8: ("mov", "eax, dword ptr [eax + 0x58]"),
        0x5CE415: ("call", "dword ptr [edx + 0xc]"),
        0x5CE468: ("mov", "eax, dword ptr [eax + 0x58]"),
        0x5CE472: ("call", "dword ptr [ecx + 8]"),
        0x5CE477: ("mov", "dword ptr [edx + 0x58], 0"),
        0x5CA71A: ("mov", "dword ptr [edi + 0x58], 0x5622"),
    }.items():
        assert instructions[address] == expected


@pytest.mark.parametrize("old,new", (
    ("format.nSamplesPerSec = playback_frequency;", "format.nSamplesPerSec = 0;"),
    ("format.nAvgBytesPerSec = playback_frequency << 2;", "format.nAvgBytesPerSec = playback_frequency;"),
    ("d.play_buffer->SetFrequency(playback_frequency);", "d.play_buffer->SetFrequency(format.nSamplesPerSec);"),
    ("source_buffer->GetCaps(&source_caps);", "d.play_buffer->GetCaps(&source_caps);"),
    ("source_buffer->Release();", "source_owner->play_buffer->Release();"),
))
def test_actual_source_rejects_owner_and_reload_regressions(old, new):
    result = build_and_run(((old, new),))
    assert result.returncode != 0, result.stdout + result.stderr


def test_descriptor_declares_sdk_pointer_and_no_conflicting_raw_rate():
    header = (ROOT / "src/movie_media.h").read_text()
    assert "IDirectSoundBuffer *source_buffer_58;" in header
    assert "DWORD playback_frequency;" in header
    for name in ("movie_media.h", "movie_media.cpp", "uv2player.cpp"):
        assert "unresolved_source_or_rate_58" not in (ROOT / "src" / name).read_text()
