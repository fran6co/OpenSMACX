"""Common Sound storage, real list operations and independent damage controls."""

from pathlib import Path

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import read_objects, object_targets
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, run_recovered_harness
from test_sound_virtual_surface import family
from test_sound_lifetime import decode


HEAD = "?SoundChainHead@@3PAVSound@@A"
TAIL = "?SoundChainTail@@3PAVSound@@A"
ADDRESSES = (0x4C6370, 0x4C63D0)


@pytest.fixture(scope="module")
def chain(family):
    compiler, settings, trials, image = family
    objects = [b for b in read_objects(ROOT, image_pin(ROOT)) if b.symbol in (HEAD, TAIL)]
    assert {b.symbol: b.span for b in objects} == {
        HEAD: Span(0x90DB20, 0x90DB24), TAIL: Span(0x90DB1C, 0x90DB20)}
    functions = [FunctionBinding(Origin(image_pin(ROOT), "pe:va", r.address),
                                tuple(Span(a, b) for a, b in r.image_spans),
                                r.symbol or r.name, "src/sound.cpp")
                 for r in read_file(ROOT / "src/sound.cpp") if r.address in ADDRESSES]
    assert len(functions) == 2
    return compiler, settings, trials, image, objects, functions


def test_original_chain_operations_identify_both_independent_zero_initialized_ends(chain):
    _, _, _, image, objects, _ = chain
    ops = {op.address: op for op in decode(image.read(Span(0x4C6370, 0x4C63C1)), 0x4C6370)}
    assert ops[0x4C6387].operands[0].mem.disp == 0x90DB20
    assert ops[0x4C638D].operands[0].mem.disp == 0x90DB1C
    for binding in objects:
        assert binding.evidence.address == 0x4C6370
        assert image.read_initial(binding.span) == bytes(4)


def test_actual_storage_and_complete_attach_detach_match_with_exact_targets(chain):
    _, _, trials, image, bindings, functions = chain
    for objects in trials:
        targets, _ = object_targets(bindings, objects)
        assert targets == {HEAD: 0x90DB20, TAIL: 0x90DB1C}
        for row in check_initial_data(bindings, objects, image, targets):
            assert row["definition"] == row["content"] == "pass", row
            assert row["compiler_storage"]["size"] == 4
        for row in check_objects(functions, objects, image, external_targets=targets):
            assert row["boundary"] == row["bytes"] == row["references"] == "pass", row


def test_strict_references_reject_swapped_chain_ends(chain):
    _, _, trials, image, _, functions = chain
    for objects in trials:
        rows = check_objects(functions, objects, image,
                             external_targets={HEAD: 0x90DB1C, TAIL: 0x90DB20})
        assert all(row["references"] == "fail" for row in rows), rows


CHAIN_HANDLERS = [
    ("// LIVE_ATTACH", "src/sound.cpp", 0x4C6370),
    ("// LIVE_DETACH", "src/sound.cpp", 0x4C63D0),
    ("// LIVE_DESTRUCTOR", "src/sound.cpp", 0x4C6120),
    ("// LIVE_NEXT", "src/sound.cpp", 0x448310),
    ("// LIVE_PREV", "src/sound.cpp", 0x448320),
]
CHAIN_DECLARATIONS = [
    ("// LIVE_FADE_DECLARATION", "src/sound.h", "class SoundFadeState {", "// RECOVERY_MEMBER: v1 SoundFadeState volume_"),
    ("// LIVE_SOUND_DECLARATION", "src/sound.h", "class Sound : public SoundFadeState {", "// Two independent sources agree on 0x54"),
    ("// LIVE_CHAIN_STORAGE", "src/sound.cpp", "Sound *SoundChainHead;", "/*\nPurpose: Unknown; the legacy implementation is a constant return that returns 11."),
]


def run_chain(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "sound_chain_storage_harness.cpp",
                          CHAIN_HANDLERS, declarations=CHAIN_DECLARATIONS)


def test_real_chain_supports_sound_and_non_wave_subclasses_and_destruction(tmp_path, monkeypatch):
    run_chain(tmp_path, monkeypatch)


@pytest.mark.parametrize("before,after", [
    ("SoundChainHead = this;", "SoundChainTail = this;"),
    ("chain_prev_->chain_next_ = this;", "/* missing forward link */"),
    ("SoundChainTail = chain_prev_;", "SoundChainTail = chain_next_;"),
    ("flags_40_ &= ~2u;", "flags_40_ |= 2u;"),
    ("chain_next_ = nullptr;", "/* missing cleared link */"),
])
def test_real_chain_contract_rejects_broken_links_ends_or_flags(tmp_path, monkeypatch, before, after):
    original = Path.read_text
    path = ROOT / "src/sound.cpp"
    assert before in original(path)
    def damaged(p, *args, **kwargs):
        text = original(p, *args, **kwargs)
        return text.replace(before, after) if p == path else text
    monkeypatch.setattr(Path, "read_text", damaged)
    with pytest.raises(AssertionError):
        run_chain(tmp_path, monkeypatch)
