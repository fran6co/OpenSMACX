"""Execute surface setup at the original descriptor capture boundaries."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from recovery.tests.test_movie_media_behavior import generated_source, definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/movie_media.cpp"


def _body(source_text, variant):
    body = definition(source_text, "void MediaRuntime::sub_5cbe30()")
    if variant == "actual":
        return body
    if variant == "cached_owner_damage":
        damaged, count = re.subn(r"(?m)^ *d = descriptor;\n", "", body)
        assert count >= 5
        return damaged
    if variant == "extra_refresh_damage":
        anchor = "d->surface_14->GetPixelFormat(&pixel_format);\n\n"
        assert body.count(anchor) == 1
        return body.replace(anchor, anchor + "      d = descriptor;\n", 1)
    raise AssertionError(variant)


def projected_program(variant="actual"):
    source_text = SOURCE.read_text()
    body = _body(source_text, variant)
    text = generated_source(source_text)

    text = text.replace(
        "typedef void *HWND;\n",
        "typedef void *HWND;\ntypedef void *LPPALETTEENTRY;\n",
        1,
    )
    text = text.replace(
        "static const GUID IID_IDirectDrawSurface2 = {{0}};\n"
        "struct DDCAPS { DWORD unused[8]; };",
        "static const GUID IID_IDirectDrawSurface2 = {{0}};\n"
        "static const GUID IID_IDirectDraw2 = {{1}};\n"
        "struct DDCAPS { DWORD dwSize, dwCaps; BYTE rest[0x164]; };",
        1,
    )
    text = text.replace(
        "struct IDirectDrawSurface2 : IDirectDrawSurface {\n"
        "  virtual HRESULT SetColorKey(DWORD, DDCOLORKEY *) = 0;\n"
        "};\n"
        "struct IDirectDraw2 {\n"
        "  virtual HRESULT CreateSurface(DDSURFACEDESC *, IDirectDrawSurface **,\n"
        "                                void *) = 0;\n"
        "};",
        "struct IDirectDrawPalette {};\n"
        "struct IDirectDrawSurface2 : IDirectDrawSurface {\n"
        "  virtual HRESULT SetColorKey(DWORD, DDCOLORKEY *) = 0;\n"
        "  virtual HRESULT GetSurfaceDesc(DDSURFACEDESC *) { return DD_OK; }\n"
        "  virtual HRESULT GetPixelFormat(DDPIXELFORMAT *) { return DD_OK; }\n"
        "  virtual HRESULT SetPalette(IDirectDrawPalette *) { return DD_OK; }\n"
        "};\n"
        "struct IDirectDraw2 {\n"
        "  virtual HRESULT GetCaps(DDCAPS *, DDCAPS *) { return DD_OK; }\n"
        "  virtual HRESULT CreatePalette(DWORD, LPPALETTEENTRY, IDirectDrawPalette **, void *) { return DD_OK; }\n"
        "  virtual HRESULT CreateSurface(DDSURFACEDESC *, IDirectDrawSurface **,\n"
        "                                void *) = 0;\n"
        "};\n"
        "struct IDirectDrawQuery { virtual HRESULT QueryInterface(const GUID &, void **) = 0; };",
        1,
    )
    table_old = (
        "struct MediaConversionTableStorage {\n"
        "  DWORD block_scale_entries[64];\n"
        "  BYTE unresolved[12];\n"
        "  DWORD packed_component_words[384];\n"
        "};"
    )
    table_new = (
        "struct MediaConversionTableStorage {\n"
        "  DWORD block_scale_entries[64];\n"
        "  BYTE unresolved[12];\n"
        "  DWORD packed_component_words[384];\n"
        "  struct { DWORD words[1024]; } remap_or_lookup_storage;\n"
        "};"
    )
    assert text.count(table_old) == 1
    text = text.replace(table_old, table_new, 1)

    descriptor_old = (
        "struct MediaDescriptor {\n"
        "  DWORD flags;\n"
        "  HMMIO stream;\n"
        "  DWORD stream_gate_7c;\n"
        "  HWND window;\n"
        "  IDirectSound *sound_device;\n"
        "  IDirectSoundBuffer *source_buffer_58;\n"
        "  IDirectSoundBuffer *play_buffer;\n"
        "  LONG volume_argument;\n"
        "};"
    )
    descriptor_new = (
        "struct MediaPixelFormat {\n"
        "  DWORD red_bits, green_bits, blue_bits, red_shift, green_shift, blue_shift;\n"
        "  DWORD combined_component_mask, total_component_bits;\n"
        "  void sub_5cc450() {}\n"
        "};\n"
        "struct MediaDescriptor {\n"
        "  DWORD flags, capability_options_08;\n"
        "  DWORD stream_gate_80;\n"
        "  IDirectDrawQuery *draw_query_interface_0c;\n"
        "  IDirectDraw2 *draw_device_10;\n"
        "  IDirectDrawSurface2 *surface_14, *surface_1c, *surface_20;\n"
        "  IDirectDrawPalette *palette_24;\n"
        "  BYTE *palette_entries_28;\n"
        "  MediaPixelFormat pixel_format;\n"
        "  DWORD setup_value_78;\n"
        "  HMMIO stream;\n"
        "  DWORD stream_gate_7c;\n"
        "  HWND window;\n"
        "  IDirectSound *sound_device;\n"
        "  IDirectSoundBuffer *source_buffer_58;\n"
        "  IDirectSoundBuffer *play_buffer;\n"
        "  LONG volume_argument;\n"
        "};"
    )
    assert text.count(descriptor_old) == 1
    text = text.replace(descriptor_old, descriptor_new, 1)
    runtime_anchor = "  int sub_5cae00();\n};"
    runtime_new = (
        "  int sub_5cae00();\n"
        "  void sub_5cbe30();\n"
        "  DWORD release_interface_10, queue_record_kind;\n"
        "  DWORD surface_width_004, surface_height_008;\n"
        "  DWORD teardown_gate_3c;\n"
        "};"
    )
    assert text.count(runtime_anchor) == 1
    text = text.replace(runtime_anchor, runtime_new, 1)

    # Add observable surface callbacks, preserving all previous fixture defaults.
    surface_fields = "  DDCOLORKEY last_key;\n  MockSurface2()"
    surface_fields_new = (
        "  DDCOLORKEY last_key;\n"
        "  MediaRuntime *callback_runtime; MediaDescriptor *after_desc, *after_pixel;\n"
        "  int desc_calls, pixel_calls;\n"
        "  MockSurface2()"
    )
    assert text.count(surface_fields) == 1
    text = text.replace(surface_fields, surface_fields_new, 1)
    surface_init = "query_result_pointer(this), last_color_key_flags(0), last_key() {}"
    surface_init_new = (
        "query_result_pointer(this), last_color_key_flags(0), last_key(), "
        "callback_runtime(0), after_desc(0), after_pixel(0), desc_calls(0), pixel_calls(0) {}"
    )
    assert text.count(surface_init) == 1
    text = text.replace(surface_init, surface_init_new, 1)
    surface_method_anchor = "    return color_key_result;\n  }\n};\nstruct MockSurface1"
    surface_methods = (
        "    return color_key_result;\n  }\n"
        "  HRESULT GetSurfaceDesc(DDSURFACEDESC *desc) {\n"
        "    ++desc_calls; desc->dwWidth = 640; desc->dwHeight = 480;\n"
        "    if (callback_runtime && after_desc) callback_runtime->descriptor = after_desc;\n"
        "    return DD_OK;\n"
        "  }\n"
        "  HRESULT GetPixelFormat(DDPIXELFORMAT *format) {\n"
        "    ++pixel_calls; format->dwRGBBitCount = 16; format->dwRBitMask = 0xF800;\n"
        "    format->dwGBitMask = 0x07E0; format->dwBBitMask = 0x001F;\n"
        "    if (callback_runtime && after_pixel) callback_runtime->descriptor = after_pixel;\n"
        "    return DD_OK;\n"
        "  }\n"
        "};\nstruct MockSurface1"
    )
    assert text.count(surface_method_anchor) == 1
    text = text.replace(surface_method_anchor, surface_methods, 1)

    draw_anchor = "struct MockDraw2 : IDirectDraw2 {\n  HRESULT create_result;"
    draw_new = (
        "struct MockDraw2 : IDirectDraw2 {\n"
        "  MediaRuntime *callback_runtime; MediaDescriptor *after_caps;\n"
        "  DWORD reported_caps; int caps_calls;\n"
        "  HRESULT create_result;"
    )
    assert text.count(draw_anchor) == 1
    text = text.replace(draw_anchor, draw_new, 1)
    draw_ctor = "MockDraw2() : create_result(DD_OK), created(0), calls(0), observed() {}"
    draw_ctor_new = (
        "MockDraw2() : callback_runtime(0), after_caps(0), reported_caps(0), caps_calls(0), "
        "create_result(DD_OK), created(0), calls(0), observed() {}"
    )
    assert text.count(draw_ctor) == 1
    text = text.replace(draw_ctor, draw_ctor_new, 1)
    draw_method_anchor = "  HRESULT CreateSurface(DDSURFACEDESC *description,"
    draw_methods = (
        "  HRESULT GetCaps(DDCAPS *a, DDCAPS *) {\n"
        "    ++caps_calls; a->dwCaps = reported_caps;\n"
        "    if (callback_runtime && after_caps) callback_runtime->descriptor = after_caps;\n"
        "    return DD_OK;\n"
        "  }\n"
        "  HRESULT CreateSurface(DDSURFACEDESC *description,"
    )
    assert text.count(draw_method_anchor) == 1
    text = text.replace(draw_method_anchor, draw_methods, 1)

    helper = definition(source_text, "int __fastcall media_component_mask_bits_31(")
    stubs = helper + "\n" + (
        "static int sub_5cc150(int, int, int, int, int, int, int) { return 0; }\n"
        "static void sub_5cc2b0(int) {}\n"
    )
    marker = "struct MockSurface2 : IDirectDrawSurface2 {"
    assert text.count(marker) == 1
    text = text.replace(marker, stubs + body + "\n" + marker, 1)

    test_body = r'''static void test_5cbe30_descriptor_capture_windows() {
  MockDraw2 draw;
  MockSurface2 surface_a, surface_b, surface_c, surface_d;
  MediaDescriptor entry = {}, after_caps = {}, after_surface_desc = {}, after_pixel = {};
  MediaRuntime runtime = {};
  runtime.descriptor = &entry;
  entry.flags = 0x00000200;
  entry.capability_options_08 = 0;
  entry.draw_device_10 = &draw;
  entry.surface_1c = &surface_a;
  entry.surface_20 = &surface_c;

  after_caps.capability_options_08 = 2;
  after_caps.surface_1c = &surface_c;
  after_caps.surface_20 = &surface_a;
  after_caps.surface_14 = &surface_b;
  after_surface_desc.pixel_format.total_component_bits = 77;
  after_surface_desc.surface_14 = &surface_c;
  after_surface_desc.surface_1c = &surface_a;
  after_surface_desc.surface_20 = &surface_b;
  after_pixel.flags = 0x04100800; // skip palette allocation and setup-value write
  after_pixel.pixel_format.total_component_bits = 99;
  after_pixel.surface_14 = &surface_d;
  after_pixel.surface_1c = &surface_b;
  after_pixel.surface_20 = &surface_c;

  draw.reported_caps = 0x6000;
  draw.callback_runtime = &runtime;
  draw.after_caps = &after_caps;
  surface_b.callback_runtime = &runtime;
  surface_b.after_desc = &after_surface_desc;
  surface_c.callback_runtime = &runtime;
  surface_c.after_pixel = &after_pixel;

  runtime.sub_5cbe30();
  assert(runtime.descriptor == &after_pixel);
  assert(runtime.queue_record_kind == 2); // uses capability_options_08 after GetCaps
  assert(entry.surface_1c == &surface_a && entry.surface_20 == &surface_c);
  assert(after_caps.surface_1c == 0 && after_caps.surface_20 == 0);
  assert(after_surface_desc.surface_1c == &surface_a &&
         after_surface_desc.surface_20 == &surface_b);
  assert(after_pixel.surface_1c == &surface_b && after_pixel.surface_20 == &surface_c);
  assert(surface_b.desc_calls == 1 && surface_b.pixel_calls == 0);
  assert(surface_c.desc_calls == 0 && surface_c.pixel_calls == 1);
  assert(surface_d.desc_calls == 0 && surface_d.pixel_calls == 0);
  assert(after_surface_desc.pixel_format.total_component_bits == 16);
  assert(after_surface_desc.pixel_format.red_bits == 5 &&
         after_surface_desc.pixel_format.green_bits == 6 &&
         after_surface_desc.pixel_format.blue_bits == 5);
  assert(after_pixel.pixel_format.total_component_bits == 99);
  assert(runtime.surface_width_004 == 640 && runtime.surface_height_008 == 480);
  assert(runtime.teardown_gate_3c == 1);
}

'''
    text = text.replace("int main() {\n", test_body + "int main() {\n", 1)
    text = text.replace(
        "  test_optional_source_buffer_is_not_playback_rate();\n",
        "  test_optional_source_buffer_is_not_playback_rate();\n"
        "  test_5cbe30_descriptor_capture_windows();\n",
        1,
    )
    return text


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("variant,expected_success", (
    ("actual", True), ("cached_owner_damage", False), ("extra_refresh_damage", False)))
def test_5cbe30_live_descriptor_owner(tmp_path, compiler_name, variant, expected_success):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    output = tmp_path / "media_5cbe30_host.cpp"
    binary = tmp_path / "media_5cbe30_host"
    output.write_text(projected_program(variant))
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", str(output), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (ran.returncode == 0) == expected_success, ran.stdout + ran.stderr
