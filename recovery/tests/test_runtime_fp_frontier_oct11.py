"""Physical CRT attribution, not source-C recovery or final-link selection."""
from dataclasses import replace
import hashlib
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage
from recovery.environment.wine import VC6_ROOT

ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/"src/vendor/vc6-runtime.recovery.h"
BASE="766dcf74c"
EXPECTED={
    "__matherr":(0x64F09C,0x64F09F,"../build/intel/mt_obj/matherr.obj",2),
    "__statfp":(0x64CA1D,0x64CA2B,"../build/intel/mt_obj/fpctrl.obj",2),
    "__clrfp":(0x64CA2B,0x64CA3A,"../build/intel/mt_obj/fpctrl.obj",3),
    "__ctrlfp":(0x64CA3A,0x64CA5D,"../build/intel/mt_obj/fpctrl.obj",4),
}


def test_prior_source_bindings_and_notes_preserved():
    before=subprocess.check_output(["git","show",BASE+":src/vendor/vc6-runtime.recovery.h"],cwd=ROOT,text=True)
    after=SOURCE.read_text()
    old,new=parse_component(before),parse_component(after)
    # Later dependency discoveries may append bindings. Project this step's
    # original identities without weakening their preservation assertions.
    owned={b.symbol for b in old.bindings}|set(EXPECTED)
    new=replace(new,bindings=tuple(b for b in new.bindings if b.symbol in owned))
    assert new.bindings[:-4]==old.bindings
    assert replace(new,bindings=old.bindings)==old
    for delimiter,index in (("// RECOVERY_COMPONENT_BEGIN v1",0),("// RECOVERY_COMPONENT_END",1)):
        assert before.split(delimiter)[index]==after.split(delimiter)[index]
    assert {b.symbol:(b.origin.address,b.spans[0].end,b.unit) for b in new.bindings[-4:]}=={
        symbol:values[:3] for symbol,values in EXPECTED.items()}
    assert all("not" in b.evidence and "final-link" in b.evidence for b in new.bindings[-4:])


@pytest.fixture(scope="module")
def artifacts():
    image=ROOT/".opensmacx/game/terranx_original.exe"
    library=VC6_ROOT/"LIB/LIBCMT.LIB"
    if not image.is_file() or not library.is_file():
        pytest.skip("pinned local image and external runtime library required")
    manifest=parse_component(SOURCE.read_text())
    return manifest,image.read_bytes(),library.read_bytes()


def test_native_full_manifest_four_rows_and_existing_unknowns(artifacts):
    from match_recovery.import_source import read_imports
    manifest,image,archive=artifacts
    prior=parse_component(subprocess.check_output(["git","show",BASE+":src/vendor/vc6-runtime.recovery.h"],cwd=ROOT,text=True))
    imports=read_imports(ROOT)
    before=verify_toolchain(prior,image,archive,import_bindings=imports)
    owned={b.symbol for b in prior.bindings}|set(EXPECTED)
    manifest=replace(manifest,bindings=tuple(b for b in manifest.bindings if b.symbol in owned))
    after=verify_toolchain(manifest,image,archive,import_bindings=imports)
    assert before["result"]==after["result"]=="unknown"
    previous={r["symbol"]:r for r in before["functions"]}
    current={r["symbol"]:r for r in after["functions"]}
    assert all(current[symbol]==row for symbol,row in previous.items())
    assert set(current)-set(previous)==set(EXPECTED)
    for symbol,(start,end,unit,section) in EXPECTED.items():
        row=current[symbol]
        assert row["result"]==row["boundary"]==row["bytes"]==row["references"]=="pass",row
        assert row["span"]=={"start":start,"end":end}
        assert row["unit"]==unit and row["section"]==section
        assert row["relocations"]==[] and row["alignment_fill"]==0
    assert after["application_recoveries"]==0 and not after["ready_for_collection"]


def test_independent_decoded_extents_and_direct_callers(artifacts):
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    manifest,data,_=artifacts
    image=PeImage(data,manifest.bindings[0].origin.image_sha256)
    decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    for start,end,_,_ in EXPECTED.values():
        instructions=list(decoder.disasm(image.read(Span(start,end)),start))
        assert instructions[0].address==start
        assert sum(i.size for i in instructions)==end-start
        assert instructions[-1].mnemonic=="ret" and instructions[-1].address+instructions[-1].size==end
    for address,target in {0x64C803:0x64F09C,0x64DBF3:0x64F09C,0x64C3DD:0x64CA1D,
                           0x64C4ED:0x64CA2B,0x64C824:0x64CA3A,0x64DBDD:0x64CA3A}.items():
        instruction,=decoder.disasm(image.read(Span(address,address+5)),address)
        assert (instruction.mnemonic,instruction.op_str)==("call",hex(target))


@pytest.mark.parametrize("symbol",EXPECTED)
@pytest.mark.parametrize("damage",("none","extent","member","symbol","archive_pin","body"))
def test_candidate_damage_cannot_pass(artifacts,symbol,damage):
    manifest,image,archive=artifacts
    binding,=tuple(b for b in manifest.bindings if b.symbol==symbol)
    # A restricted candidate isolates these relocation-free rows; it is not a
    # waiver for unrelated UNKNOWNs in the real manifest.
    selected=replace(manifest,bindings=(binding,),absolute_symbols=(),data_sections=())
    if damage=="extent":
        binding=replace(binding,spans=(Span(binding.origin.address,binding.spans[0].end+1),))
    elif damage=="member":
        binding=replace(binding,unit="../build/intel/mt_obj/frnd.obj")
    elif damage=="symbol":
        binding=replace(binding,symbol="__missing_fp_entry")
    elif damage=="archive_pin":
        selected=replace(selected,archive_sha256="0"*64)
    elif damage=="body":
        parsed=PeImage(image,binding.origin.image_sha256)
        section,offset,size=next((s,o,n) for s,o,n in parsed.sections if s.start<=binding.origin.address<s.start+n)
        physical=offset+binding.origin.address-section.start
        changed=bytearray(image); changed[physical]^=1; image=bytes(changed)
        binding=replace(binding,origin=Origin(hashlib.sha256(image).hexdigest(),"pe:va",binding.origin.address))
    selected=replace(selected,bindings=(binding,))
    report=verify_toolchain(selected,image,archive)
    if damage=="archive_pin":
        assert report["result"]=="fail" and report["functions"]==[]
        return
    assert (report["functions"][0]["result"]=="pass")== (damage=="none"),report
