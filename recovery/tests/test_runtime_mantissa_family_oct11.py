"""Closed pinned-library correspondence, not recovered application source."""
from dataclasses import replace
import hashlib
from pathlib import Path
import subprocess

import pytest
from match_recovery.component_source import parse_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.model import Span
from match_recovery.source import read_text
from test_crt_eight_entry_bindings_oct10 import inputs as eight_entry_inputs, verify, row, repin, raw_offset
from test_runtime_xtoa_pair_oct11 import without_xtoa_pair

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "src/vendor/vc6-runtime.recovery.h"
BASE = "ab4d03b7143c92a9278bf2816f6877a311c1aafb"
ADDITION = "a6dd9e351123e69eaf5e1ad487a9a55dedb1d013"
EXPECTED = (
    (0x64D3BF, 0x64D408, "__ZeroTail", "intrncvt", 2),
    (0x64D408, 0x64D45E, "__IncMan", "intrncvt", 4),
    (0x64D45E, 0x64D4EA, "__RoundMan", "intrncvt", 6),
    (0x64D4EA, 0x64D505, "__CopyMan", "intrncvt", 7),
    (0x64D505, 0x64D511, "__FillZeroMan", "intrncvt", 9),
    (0x64D511, 0x64D52C, "__IsZeroMan", "intrncvt", 11),
    (0x64D52C, 0x64D5B9, "__ShrMan", "intrncvt", 13),
    (0x64F383, 0x64F3A4, "___addl", "mantold", 1),
    (0x64F3A4, 0x64F402, "___add_12", "mantold", 3),
    (0x64F402, 0x64F430, "___shl_12", "mantold", 5),
    (0x64F430, 0x64F45D, "___shr_12", "mantold", 7),
    (0x64F45D, 0x64F524, "___mtold12", "mantold", 9),
)
MEMBERS = {
    "intrncvt": "4df3d60d0e40f94ac3f89fc956858b79b648470927fb1c578c4fbec3fb3239b3",
    "mantold": "bbfa4484dcc4b5336e731b38daf8d0c190cc5328b761d8dfe75c8e86ec51a0a7",
}


def baseline():
    return parse_component(subprocess.check_output(
        ["git", "show", BASE + ":" + SOURCE], cwd=ROOT, text=True))


@pytest.fixture(scope="module")
def inputs(eight_entry_inputs):
    """Use this task's immutable published checkpoint, not the later live component."""
    _, image, archive, imports = eight_entry_inputs
    checkpoint = parse_component(subprocess.check_output(
        ["git", "show", ADDITION + ":" + SOURCE], cwd=ROOT, text=True))
    return checkpoint, image, archive, imports


def test_exact_addition_preserves_all_prior_metadata_and_original_claims():
    text = subprocess.check_output(["git", "show", ADDITION + ":" + SOURCE], cwd=ROOT, text=True)
    current, old = parse_component(text), baseline()
    assert replace(current, bindings=current.bindings[:-12],
                   code_sections=old.code_sections) == old
    assert tuple((b.origin.address, b.spans[0].end, b.symbol,
                  b.unit) for b in current.bindings[-12:]) == tuple(
        (a, e, s, "../build/intel/mt_obj/" + u + ".obj") for a,e,s,u,_ in EXPECTED)
    prior = subprocess.check_output(["git", "show", BASE + ":" + SOURCE], cwd=ROOT, text=True)
    assert [replace(r, line=1, region="") for r in read_text(text, ROOT / SOURCE)] == [
        replace(r, line=1, region="") for r in read_text(prior, ROOT / SOURCE)]
    installed = parse_component((ROOT / SOURCE).read_text())
    assert installed.bindings[:len(current.bindings)] == current.bindings


def test_physical_members_reference_closure_and_precise_prior_upgrades(inputs):
    manifest, _, archive, _ = inputs
    before, after = verify(inputs, baseline()), verify(inputs)
    changed = []
    for old in before["functions"]:
        current = row(after, int(old["address"], 16))
        if old != current:
            changed.append(int(old["address"], 16))
            assert current["boundary"] == old["boundary"]
            assert current["section"] == old["section"]
            old_refs, new_refs = old["relocations"], current["relocations"]
            assert len(old_refs) == len(new_refs)
            differences = [(x,y) for x,y in zip(old_refs,new_refs) if x != y]
            assert len(differences) == (12 if int(old["address"],16) == 0x64D5B9 else 1)
            for x,y in differences:
                assert x["result"] == "unknown" and y["result"] == "pass"
                assert y["symbol"] in {s for _,_,s,_,_ in EXPECTED}
            if int(old["address"],16) == 0x64D5B9:
                assert all(current[k] == "pass" for k in ("result","bytes","references"))
            else:
                assert current["result"] == old["result"] == "unknown"
    assert changed == [0x64D5B9, 0x64F524]
    for a,e,s,u,section in EXPECTED:
        result = row(after,a)
        assert all(result[k] == "pass" for k in ("result","boundary","bytes","references"))
        member = CoffArchive(archive).member("../build/intel/mt_obj/" + u + ".obj")
        assert hashlib.sha256(member).hexdigest() == MEMBERS[u]
        obj = Coff(member)
        symbol, = [x for x in obj.symbols.values() if x.name == s]
        assert (symbol.section,symbol.value,symbol.storage) == (section,0,2)
        assert len(obj.sections[section-1].data) == e-a
        assert result["alignment_fill"] == 0
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0 and not after["ready_for_collection"]


def test_original_complete_decode_internal_exits_and_incoming_calls(inputs):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    manifest,data,_,_ = inputs
    image = PeImage(data,manifest.bindings[0].origin.image_sha256)
    decoder = Cs(CS_ARCH_X86,CS_MODE_32)
    for a,e,_,_,_ in EXPECTED:
        instructions = list(decoder.disasm(image.read(Span(a,e)),a))
        assert sum(i.size for i in instructions) == e-a
        assert instructions[-1].mnemonic == "ret" and instructions[-1].address == e-1
        for i in instructions:
            if i.mnemonic.startswith("j") and i.op_str.startswith("0x"):
                assert a <= int(i.op_str,16) < e
    calls = {0x64D4A6:0x64D3BF,0x64D4B5:0x64D408,0x64D63A:0x64D45E,
             0x64D62E:0x64D4EA,0x64D618:0x64D505,0x64D606:0x64D511,
             0x64D679:0x64D52C,0x64D433:0x64F383,0x64F49D:0x64F3A4,
             0x64F48D:0x64F402,0x64FBBD:0x64F430,0x64F8EE:0x64F45D}
    for site,target in calls.items():
        instruction, = decoder.disasm(image.read(Span(site,site+5)),site)
        assert (instruction.mnemonic,instruction.op_str) == ("call",hex(target))


@pytest.mark.parametrize("address,end,symbol,unit,section", EXPECTED)
@pytest.mark.parametrize("damage", ("body","extent","member"))
def test_attribution_damage_cannot_pass(inputs,address,end,symbol,unit,section,damage):
    manifest,image,_,_ = inputs
    if damage == "body":
        data = bytearray(image)
        data[raw_offset(image,address)] ^= 1
        image = bytes(data)
        manifest = repin(manifest,image)
    else:
        manifest = replace(manifest,bindings=tuple(
            (replace(b,spans=(Span(address,end-1),)) if damage == "extent"
             else replace(b,unit="build/intel/mt_obj/mlock.obj"))
            if b.origin.address == address else b for b in manifest.bindings))
    assert row(verify(inputs,manifest,image),address)["result"] != "pass"


@pytest.mark.parametrize("site,owner", ((0x64D433,0x64D408),(0x64D4A6,0x64D45E),
                                       (0x64F3B3,0x64F3A4),(0x64F49D,0x64F45D)))
def test_wrong_helper_target_refuses_with_fresh_pin(inputs,site,owner):
    manifest,image,_,_ = inputs
    data = bytearray(image)
    data[raw_offset(image,site+1)] ^= 1
    image = bytes(data)
    result = row(verify(inputs,repin(manifest,image),image),owner)
    assert result["references"] != "pass" and result["result"] != "pass"
