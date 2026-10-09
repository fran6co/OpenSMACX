"""Real common-chain device switching; external driver remains an ABI double."""

from pathlib import Path

import pytest

from match_recovery.pe_coff import Coff
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.object_source import object_targets
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, run_recovered_harness
from test_sound_chain_storage import CHAIN_DECLARATIONS, CHAIN_HANDLERS, HEAD, TAIL, chain
from test_sound_virtual_surface import family, observations


def test_typed_switch_matches_complete_original_body_and_chain_target(chain):
    compiler, settings, trials, image, storage, _ = chain
    path = ROOT / "src/wave_device.cpp"
    row, = (r for r in read_file(path) if r.address == 0x4C5030)
    binding = FunctionBinding(Origin(image_pin(ROOT), "pe:va", row.address),
                              tuple(Span(a, b) for a, b in row.image_spans),
                              row.symbol or row.name, "src/wave_device.cpp")
    passed = []
    reports = []
    for flags, owners in zip(settings.flag_sets, trials):
        objects = {**owners, "src/wave_device.cpp": Coff(compiler(path, (), flags + settings.layout_flags))}
        targets, _ = object_targets(storage, objects)
        assert targets == {HEAD: 0x90DB20, TAIL: 0x90DB1C}
        result, = check_objects([binding], objects, image, external_targets=targets)
        reports.append(result)
        passed.append(all(result.get(dimension) == "pass"
                          for dimension in ("result", "boundary", "bytes", "references")))
        if passed[-1]:
            damaged, = check_objects([binding], objects, image,
                                     external_targets={HEAD: targets[TAIL], TAIL: targets[HEAD]})
            assert damaged["references"] == "fail", damaged
    assert any(passed), reports


def test_wave_attribute_override_occupies_the_original_sound_slot(family):
    _, _, trials, image = family
    assert int.from_bytes(image.read(Span(0x66E44C + 28 * 4, 0x66E44C + 29 * 4)), "little") == 0x4C6F80
    for objects in trials:
        rows = observations(objects, image)
        for row in rows:
            if row["symbol"] == "??_7Sound@@6B@":
                continue  # The base is pure in both source and image; it has no owned body.
            entry = row["diagnostic"]["entries"][28]
            assert entry["original_absolute_relocation"]
            assert entry["observation"] == "target_word_agrees", (row["symbol"], entry)


def test_reverting_attribute_query_to_nonvirtual_breaks_table_contract(family):
    compiler, settings, trials, image = family
    header = (ROOT / "src/sound.h").read_bytes()
    assert header.count(b"virtual int get_attrib() = 0;") == 1
    for flags, objects in zip(settings.flag_sets, trials):
        changed = Coff(compiler.overlay(ROOT / "src/wave.cpp", {
            "src/sound.h": header.replace(b"virtual int get_attrib() = 0;", b"int get_attrib();")
        }, (), flags + settings.layout_flags))
        rows = observations({**objects, "src/wave.cpp": changed}, image)
        wave, = (row for row in rows if row["symbol"] == "??_7Wave@@6B@")
        entries = wave["diagnostic"]["entries"]
        assert entries[28]["observation"] != "target_word_agrees"


def run_switch(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "wave_device_sound_chain_harness.cpp",
        [*CHAIN_HANDLERS,
         ("// LIVE_GROUP_CONSTRUCTOR", "src/wave_device.cpp", 0x4C5490),
         ("// LIVE_GROUP_DESTRUCTOR", "src/wave_device.cpp", 0x4C5B80),
         ("// LIVE_SELECT", "src/wave_device.cpp", 0x4C5030)], declarations=[
            *CHAIN_DECLARATIONS,
            ("// LIVE_DEVICE_BASE_DECL", "src/sounddevice.h", "class SoundDeviceBase {", "static_assert(sizeof(SoundDeviceBase)"),
            ("// LIVE_SOUND_DOUBLES", "recovery/tests/sound_chain_storage_harness.cpp", "// SOUND_LINK_DOUBLES_BEGIN", "// SOUND_LINK_DOUBLES_END"),
            ("// LIVE_GROUP_TYPES", "src/wave_device.h", "struct WaveGroupNode {", " /*\n  * Wave_Device class"),
            ("// LIVE_DEVICE_DECLARATION", "src/wave_device.h", "class Wave_Device : public SoundDeviceBase {", "// THE OBJECT, at 0x0090D978"),
            ("// LIVE_SLOT_DISPATCH", "src/soundx_wave_device_abi.h", "class WaveDeviceProviderCallView {", "};"),
        ])


def test_actual_switch_rethreads_mixed_sound_types_and_observes_callbacks(tmp_path, monkeypatch):
    run_switch(tmp_path, monkeypatch)


@pytest.mark.parametrize("before,after", [
    ("(sound->get_attrib() & 1) && !(sound->get_attrib() & 4)", "(sound->get_attrib() & 5) == 1"),
    ("Sound *const next = sound->get_next();\n            sound->unload();",
     "sound->unload();\n            Sound *const next = sound->get_next();"),
    ("replay->chain_prev_ = nullptr;", "/* missing link clear */"),
    ("resume_tail = resume_tail->get_prev();\n        const char *const fname = static_cast<const char *>(replay->fname_);",
     "const char *const fname = static_cast<const char *>(replay->fname_);\n        resume_tail = resume_tail->get_prev();"),
    ("replay->load(fname);", "(void)fname;"),
    ("Sound *resume_tail = nullptr;", "void *stale_device = device_14_; Sound *resume_tail = nullptr;"),
])
def test_actual_switch_rejects_wrong_call_order_or_resume_links(tmp_path, monkeypatch, before, after):
    original = Path.read_text
    path = ROOT / "src/wave_device.cpp"
    assert before in original(path)
    def damaged(p, *args, **kwargs):
        text = original(p, *args, **kwargs)
        if p != path:
            return text
        text = text.replace(before, after)
        if "stale_device" in after:
            text = text.replace(
                "static_cast<WaveDeviceProviderCallView *>(device_14_)->select(a1);",
                "static_cast<WaveDeviceProviderCallView *>(stale_device)->select(a1);")
        return text
    monkeypatch.setattr(Path, "read_text", damaged)
    with pytest.raises(AssertionError):
        run_switch(tmp_path, monkeypatch)
