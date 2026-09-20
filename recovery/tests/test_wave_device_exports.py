"""Real export-array consumers, independent original targets and damage controls."""

from pathlib import Path

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.object_source import object_targets
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, run_recovered_harness
from test_sound_export_storage import export_storage, EXPORTS
from test_sound_virtual_surface import family


ADDRESSES = (0x4C4ED0, 0x4C4F10, 0x4C4F80)


@pytest.fixture(scope="module")
def device_exports(export_storage):
    compiler, settings, trials, image, exports = export_storage
    objects = [{**trial, "src/wave_device.cpp": Coff(compiler(
        ROOT / "src/wave_device.cpp", (), flags + settings.layout_flags))}
        for trial, flags in zip(trials, settings.flag_sets)]
    from match_recovery.model import Span
    bindings = [FunctionBinding(Origin(image_pin(ROOT), "pe:va", r.address),
                               tuple(Span(a, b) for a, b in r.image_spans),
                               r.symbol or r.name, "src/wave_device.cpp")
                for r in read_file(ROOT / "src/wave_device.cpp") if r.address in ADDRESSES]
    assert len(bindings) == len(ADDRESSES)
    return compiler, settings, objects, image, exports, bindings


def test_lifecycle_bodies_match_with_loader_owned_targets(device_exports):
    _, _, trials, image, exports, bindings = device_exports
    passes = {b.origin.address: False for b in bindings}
    for objects in trials:
        targets, _ = object_targets([exports], objects)
        assert targets == {EXPORTS: 0x90DB24}
        rows = check_objects(bindings, objects, image, external_targets=targets)
        for binding, row in zip(bindings, rows):
            if row["boundary"] == row["bytes"] == row["references"] == "pass":
                passes[binding.origin.address] = True
        # No standalone pointer definitions may shadow the loader's slots.
        assert not any(s.name.startswith(("?WaveDeviceFactoryHook@@", "?WaveDeviceDestroyHook@@"))
                       for s in objects["src/wave_device.cpp"].symbols.values())
    assert all(passes.values()), passes


@pytest.mark.parametrize("before,after,address", [
    ("SoundDllProcs[4]", "SoundDllProcs[3]", 0x4C4ED0),
    ("SoundDllProcs[5]", "SoundDllProcs[6]", 0x4C4F10),
    ("SoundDllProcs[5]", "SoundDllProcs[6]", 0x4C4F80),
])
def test_native_comparison_rejects_a_wrong_export_entry(device_exports, before, after, address):
    compiler, settings, trials, image, exports, bindings = device_exports
    source = (ROOT / "src/wave_device.cpp").read_bytes()
    for flags, objects in zip(settings.flag_sets, trials):
        changed = Coff(compiler.overlay(ROOT / "src/wave_device.cpp", {
            "src/wave_device.cpp": source.replace(before.encode(), after.encode())
        }, (), flags + settings.layout_flags))
        targets, _ = object_targets([exports], objects)
        binding, = (b for b in bindings if b.origin.address == address)
        row, = check_objects([binding], {**objects, "src/wave_device.cpp": changed}, image, external_targets=targets)
        assert row["references"] == "fail", row


def run_device(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "wave_device_exports_harness.cpp", [
        ("// LIVE_CREATE", "src/wave_device.cpp", 0x4C4ED0),
        ("// LIVE_DELETE", "src/wave_device.cpp", 0x4C4F10),
        ("// LIVE_RELEASE", "src/wave_device.cpp", 0x4C4F80)], declarations=[
        ("// LIVE_GROUP_TYPES", "src/wave_device.h", "struct WaveGroupNode {", " /*\n  * Wave_Device class"),
        ("// LIVE_DEVICE_DECLARATION", "src/wave_device.h", "class Wave_Device {", "// THE OBJECT, at 0x0090D978"),
        ("// LIVE_ROLE_TYPES", "src/wave_device.h", "typedef int(__cdecl func_wave_device_factory)", "// The per-group construct/teardown pair"),
        ("// LIVE_SLOT_DISPATCH", "src/original_seam.h", "template <class Fn>\nFn &vtable_slot", "/*\n * The same slot as a POINTER-TO-MEMBER REFERENCE"),
        ("// LIVE_EXPORT_TYPE", "src/sound.h", "typedef void *SoundDllProcsTable[11];", "extern SoundDllProcsTable SoundDllProcs;"),
        ("// LIVE_EXPORT_STORAGE", "src/sound_loader.cpp", "SoundDllProcsTable SoundDllProcs;", "int __cdecl load_sound_dll()"),
        ("// LIVE_LOADER", "src/sound_loader.cpp", "int __cdecl load_sound_dll()", "/*\nPurpose: The sound-DLL version complaint box.")])


def test_actual_lifecycle_observes_loading_rebinding_and_callback_changes(tmp_path, monkeypatch):
    run_device(tmp_path, monkeypatch)


@pytest.mark.parametrize("before,after", [
    ("SoundDllProcs[4]", "SoundDllProcs[3]"),
    ("SoundDllProcs[5]", "SoundDllProcs[6]"),
    ("return factory(device_slot, a1);", "return 0x14;"),
    ("if (device_14_) {\n            func_wave_device_destroy", "if (true) {\n            func_wave_device_destroy"),
])
def test_runtime_contract_rejects_bad_targets_or_lifecycle_guards(tmp_path, monkeypatch, before, after):
    original_read = Path.read_text
    target = ROOT / "src/wave_device.cpp"
    assert before in original_read(target)
    def damaged_read(path, *args, **kwargs):
        text = original_read(path, *args, **kwargs)
        return text.replace(before, after) if path == target else text
    monkeypatch.setattr(Path, "read_text", damaged_read)
    with pytest.raises(AssertionError):
        run_device(tmp_path, monkeypatch)
