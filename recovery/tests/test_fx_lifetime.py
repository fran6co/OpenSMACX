"""Real FX member lifetime and bounded calls, not whole sound-stack acceptance."""

from dataclasses import replace
import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.application_layout import check_member
from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.layout_source import read_members
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, references, run_recovered_harness
import test_application_link_recovery as harness_support


UNIT = "src/fx.cpp"
CTOR = "??0FX@@QAE@XZ"
DTOR = "??1FX@@QAE@XZ"
ELEMENT_CTOR = "??0Effect@@QAE@XZ"
ELEMENT_DTOR = "??1Effect@@UAE@XZ"
VECTOR_CTOR = "??_L@YGXPAXIHP6EX0@Z1@Z"
VECTOR_DTOR = "??_M@YGXPAXIHP6EX0@Z@Z"


@pytest.fixture(scope="module")
def family():
    from recovery.environment.wine import VC6_ROOT
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not path.is_file():
        pytest.skip("VC6, Wine32 and pinned image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [Coff(compiler(ROOT / UNIT, (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
    return objects, PeImage(path.read_bytes(), image_pin(ROOT))


def test_original_array_count_stride_and_element_callbacks(family):
    _, image = family
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for span, pushes, callee in (
            (Span(0x4482A0, 0x4482BB), [0x4482C0, 0x4482D0, 97, 108], 0x6457C2),
            (Span(0x448290, 0x4482A0), [0x4482C0, 97, 108], 0x6456E4)):
        instructions = list(decoder.disasm(image.read(span), span.start))
        assert [op.operands[0].imm for op in instructions if op.mnemonic == "push"
                and op.operands[0].type == X86_OP_IMM] == pushes
        assert [op.operands[0].imm for op in instructions if op.mnemonic == "call"] == [callee]


def test_bank_has_typed_effect_array_and_compiler_owned_callbacks(family):
    objects, _ = family
    requirement, = (row for row in read_members(ROOT, image_pin(ROOT))
                     if row.class_name == "FX" and row.member == "effects_")
    for obj in objects:
        layout = class_layout(obj, "FX")
        assert check_member(requirement, layout)["result"] == "pass"
        assert layout["size"] == 0x28F0
        assert layout["members"] == {"effects_": 0, "field_28EC_": 0x28EC}
        assert layout["member_types"]["effects_"] == {
            "kind": "array", "target": {"kind": "class_value", "class": "Effect"},
            "size": 97 * 108, "element_size": 108, "count": 97}
        assert references(obj, CTOR) == [ELEMENT_DTOR, ELEMENT_CTOR, VECTOR_CTOR]
        assert references(obj, DTOR) == [ELEMENT_DTOR, VECTOR_DTOR]
        obj.function(ELEMENT_CTOR)
        assert not any("EffectElementCtor" in symbol.name or "EffectElementTeardown" in symbol.name
                       for symbol in obj.symbols.values())


def test_five_bank_bodies_preserve_bytes_and_independent_call_targets(family):
    objects, image = family
    pin = image_pin(ROOT)
    addresses = {0x4482A0, 0x448290, 0x446A90, 0x446AB0, 0x446AD0}
    bindings = [FunctionBinding(Origin(pin, "pe:va", row.address),
                                tuple(Span(a, b) for a, b in row.image_spans),
                                row.symbol or row.name, UNIT)
                for row in read_file(ROOT / UNIT) if row.address in addresses]
    assert {row.origin.address for row in bindings} == addresses
    # Original callback identities and CRT destinations above. This caller
    # comparison does not certify the callees' bodies, EH runtime or linking.
    targets = {ELEMENT_CTOR: 0x4482D0, ELEMENT_DTOR: 0x4482C0,
               VECTOR_CTOR: 0x6457C2, VECTOR_DTOR: 0x6456E4}
    trials = [check_objects(bindings, {UNIT: obj}, image, external_targets=targets)
              for obj in objects]
    # The three indexed methods retain a frame in the original; the two
    # fixed project trials are alternatives, not two original compilations.
    for index in range(len(bindings)):
        rows = [trial[index] for trial in trials]
        assert any(row["boundary"] == row["bytes"] == row["references"] == "pass"
                   for row in rows), rows


def test_live_bank_constructs_destroys_and_unwinds_all_elements(tmp_path, monkeypatch):
    run_recovered_harness(tmp_path, monkeypatch, "fx_lifetime_harness.cpp", [
        ("// LIVE_FX_CONSTRUCTOR", UNIT, 0x4482A0),
        ("// LIVE_FX_DESTRUCTOR", UNIT, 0x448290),
        ("// LIVE_FX_STOP", UNIT, 0x446A90),
        ("// LIVE_FX_RELEASE", UNIT, 0x446AB0),
        ("// LIVE_FX_FADE", UNIT, 0x446AD0)], declarations=[
        ("// LIVE_FX_DECLARATION", "src/fx.h", "class FX {", "static_assert(sizeof(FX)")])


@pytest.mark.parametrize("address,before,after", [
    (0x448290, "FX::~FX() {", "FX::~FX() { effects_[0].~Effect();"),
    (0x446A90, "effects_[a1].stop();", "effects_[0].stop();"),
    (0x446AB0, "effects_[a1].release();", "effects_[96].release();"),
    (0x446AD0, "effects_[a1].fade();", "effects_[0].fade();"),
])
def test_live_bank_rejects_double_destruction_and_wrong_element(
        tmp_path, monkeypatch, address, before, after):
    reader = harness_support.read_file

    def damaged(path):
        records = []
        for row in reader(path):
            if row.address == address:
                assert row.region.count(before) == 1
                row = replace(row, region=row.region.replace(before, after))
            records.append(row)
        return records

    monkeypatch.setattr(harness_support, "read_file", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_bank_constructs_destroys_and_unwinds_all_elements(tmp_path, monkeypatch)
