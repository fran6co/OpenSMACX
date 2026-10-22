"""Test-owned VC6 virtual-base cleanup control, not game/runtime acceptance."""

from hashlib import sha256
from pathlib import Path
import struct
import sys

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, x86

from match_recovery.coff_replica import object_signature
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.processes import run_bounded
from match_recovery.source.asm import SpanComparisonRefused, compare_subject, subject_asm
from match_recovery.source.eh_regions import bind_eh_regions
from match_recovery.source.reader import read_text
from recovery.environment.compile import compile_object
from recovery.tests.test_branched_eh_compiler import (
    FLAGS, _file_offset, _registration_handler, _require_toolchain,
)


ROOT = Path(__file__).resolve().parents[2]
SYMBOL = "??1ExtendedList@@UAE@XZ"
HEADER = r"""
void observed(void *);
struct AllocationRoot {
    AllocationRoot();
    virtual ~AllocationRoot();
    unsigned owner;
};
struct ListBase : virtual AllocationRoot {
    ListBase();
    ~ListBase();
    virtual void hook();
    unsigned head, current, count, position, allocator;
};
struct ExtendedList : ListBase {
    ~ExtendedList();
    virtual void hook();
    unsigned extension[EXTENSION_WORDS];
};
"""
BASE = r"""
#include "lifetime.h"
volatile unsigned visits;
AllocationRoot::AllocationRoot() : owner(0) {}
AllocationRoot::~AllocationRoot() { visits += owner; }
ListBase::ListBase() : head(0), current(0), count(0), position(0), allocator(0) {}
ListBase::~ListBase() { observed(this); }
void ListBase::hook() { observed(this); }
void observed(void *) { ++visits; }
"""
OWNER = r"""
#include "lifetime.h"
ExtendedList::~ExtendedList() { observed(this); }
void ExtendedList::hook() { observed(this); }
"""
ENTRY = r"""
#include "lifetime.h"
int main() {
    ExtendedList extended;
    ListBase plain;
    extended.hook();
    plain.hook();
    return 0;
}
"""


def compile_fixture(tmp_path, *, extension_words=3, virtual_base=True):
    _require_toolchain()
    header = HEADER.replace("EXTENSION_WORDS", str(extension_words))
    if not virtual_base:
        header = header.replace(": virtual AllocationRoot", ": AllocationRoot")
    for name, contents in (("lifetime.h", header), ("base.cpp", BASE),
                           ("owner.cpp", OWNER), ("entry.cpp", ENTRY)):
        (tmp_path / name).write_text(contents)
    command = (sys.executable, str(ROOT / "recovery/environment/wine.py"),
               "--tool=cl", "/nologo")
    objects = {}
    for name in ("owner", "base", "entry"):
        objects[name] = compile_object(tmp_path / f"{name}.cpp", command, FLAGS)
        (tmp_path / f"{name}.obj").write_bytes(objects[name])
    replica = compile_object(tmp_path / "owner.cpp", command, FLAGS)
    assert object_signature(Coff(replica)) == object_signature(Coff(objects["owner"]))
    image_path, map_path = tmp_path / "sub-control.exe", tmp_path / "sub-control.map"
    linked = run_bounded(
        [sys.executable, str(ROOT / "recovery/environment/wine.py"), "--tool=link", "/nologo",
         "/nodefaultlib", "/subsystem:console", "/incremental:no", "/fixed:no",
         "/opt:noref", "/opt:noicf", "/map:" + str(map_path), "/out:" + str(image_path),
         *(str(tmp_path / f"{name}.obj") for name in ("owner", "base", "entry")),
         "libcmt.lib", "oldnames.lib", "kernel32.lib"], cwd=tmp_path, timeout=120)
    assert linked.returncode == 0, linked.stdout + linked.stderr
    addresses = [int(words[2], 16) for line in map_path.read_text(errors="replace").splitlines()
                 if len(words := line.split()) >= 3 and words[1] == SYMBOL]
    assert len(addresses) == 1
    address, = addresses
    owner = Coff(objects["owner"])
    root = owner.function(SYMBOL)
    children = owner.code_sections(root)
    assert len(children) == 2 and children[0] == root.section
    child, = children[1:]
    assert owner.associations[child] == root.section
    code = owner.sections[root.section - 1].data
    cold = owner.sections[child - 1].data
    raw = image_path.read_bytes()
    image = PeImage(raw, sha256(raw).hexdigest())
    linked_root = image.read(Span(address, address + len(code)))
    handler = _registration_handler(linked_root, address)
    cold_start = handler - len(cold) + 10
    spans = ((address, address + len(code)), (cold_start, cold_start + len(cold)))
    text = ";".join(f"0x{low:08X}-0x{high:08X}" for low, high in spans)
    record, = read_text(f"// ORIGINAL: 0x{address:08X} test-owned {text} FILE\n"
                        f"// symbol {SYMBOL}\nExtendedList::~ExtendedList() {{}}\n",
                        tmp_path / "test_mapping.cpp")
    return objects["owner"], owner, root, child, image_path, image, spans, record


@pytest.mark.parametrize("virtual_base,extension_words", [(True, 3), (True, 40), (False, 3)])
def test_linked_virtual_base_frame_transfer_and_damage(tmp_path, virtual_base, extension_words):
    raw, owner, root, child_number, image_path, image, spans, record = compile_fixture(
        tmp_path, extension_words=extension_words, virtual_base=virtual_base)
    child = owner.sections[child_number - 1]
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(child.data, 0))
    assert sum(i.size for i in instructions) == len(child.data)
    # Qualify actual compiler emission, not merely successful source compilation.
    load = instructions[0]
    assert load.bytes[:2] == b"\x8b\x4d" and load.size == 3
    assert load.operands[0].reg == x86.X86_REG_ECX
    assert load.operands[1].type == x86.X86_OP_MEM
    assert load.operands[1].mem.base == x86.X86_REG_EBP
    if virtual_base:
        adjustment, jump = instructions[1:3]
        assert adjustment.mnemonic == "sub" and adjustment.operands[0].reg == x86.X86_REG_ECX
        assert adjustment.operands[1].type == x86.X86_OP_IMM
        assert adjustment.operands[1].imm == extension_words * 4
        assert adjustment.bytes[:2] == (b"\x83\xe9" if extension_words == 3 else b"\x81\xe9")
        assert adjustment.encoding.imm_size == (1 if extension_words == 3 else 4)
    else:
        jump = instructions[1]
        assert jump.mnemonic == "jmp"  # No virtual-base extension adjustment.
    assert jump.bytes[:1] == b"\xe9" and jump.size == 5
    transfer_site = jump.address + jump.encoding.imm_offset
    cleanup, = [owner.symbols[index] for offset, index, kind in child.relocations
                if offset == transfer_site and kind == 20]
    assert cleanup.name == "??1ListBase@@UAE@XZ" and cleanup.section == 0
    regions = bind_eh_regions(raw, SYMBOL, image, spans)
    assert len(regions) == 2
    assert regions[0].ownership.original_predecessors == (0xFFFFFFFF,)
    assert regions[0].ownership.compiled_predecessors == (0xFFFFFFFF,)
    compiled = subject_asm(raw, record, "test-owned-virtual-base-control")
    assert compare_subject(record, image_path, compiled).verdict.value == "BYTE_EXACT"

    section_pointer, = struct.unpack_from("<I", raw, 20 + (child_number - 1) * 40 + 20)
    if virtual_base:
        # Still-valid numeric adjustment and frame-slot changes are real
        # differences, not relocated bytes or a new physical-ownership waiver.
        changed = bytearray(raw)
        changed[section_pointer + adjustment.address + adjustment.encoding.imm_offset] += 1
        assert compare_subject(record, image_path, subject_asm(
            bytes(changed), record, "valid-adjustment-damage")).verdict.value == "MISMATCH"
        changed = bytearray(raw)
        changed[section_pointer + load.encoding.disp_offset] -= 4
        assert compare_subject(record, image_path, subject_asm(
            bytes(changed), record, "valid-frame-damage")).verdict.value == "MISMATCH"

        # Same-sized ADD is already a supported complete action. It must not
        # be normalized into equality with the subtracting original sequence.
        changed = bytearray(raw)
        changed[section_pointer + adjustment.address + 1] = 0xC1
        assert compare_subject(record, image_path, subject_asm(
            bytes(changed), record, "valid-add-damage")).verdict.value == "MISMATCH"

    # The complete terminal transfer and original target remain obligations.
    malformed = bytearray(raw)
    malformed[section_pointer + jump.address] = 0xE8
    with pytest.raises(ValueError):
        bind_eh_regions(bytes(malformed), SYMBOL, image, spans)
    addend = bytearray(raw)
    struct.pack_into("<I", addend, section_pointer + transfer_site, 1)
    with pytest.raises(ValueError):
        bind_eh_regions(bytes(addend), SYMBOL, image, spans)
    original = bytearray(image_path.read_bytes())
    original_site = _file_offset(image, spans[1][0] + transfer_site, 4)
    struct.pack_into("<i", original, original_site, spans[0][0] - (spans[1][0] + jump.address + jump.size))
    changed_image = tmp_path / "invalid-cleanup-target.exe"
    changed_image.write_bytes(original)
    with pytest.raises(SpanComparisonRefused):
        compare_subject(record, changed_image, compiled)
