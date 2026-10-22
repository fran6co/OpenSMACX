"""Test-owned VC6/linker control for branched EH predecessor metadata.

This exercises a bounded compiler/verifier case only. The C++ fixture below
belongs to this test; it is not extracted from or linked with OpenSMACX.
"""

from hashlib import sha256
from pathlib import Path
import shutil
import struct
import sys

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32, x86

from match_recovery.coff_replica import object_signature
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.processes import run_bounded
from match_recovery.source.asm import (
    SpanComparisonRefused,
    compare_subject,
    subject_asm,
)
from match_recovery.source.eh_regions import bind_eh_regions
from match_recovery.source.reader import read_text
from recovery.environment.compile import compile_object
from recovery.environment.wine import VC6_ROOT


ROOT = Path(__file__).resolve().parents[2]
SYMBOL = "?branch_cleanup@@YAXH@Z"
FLAGS = ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Oy-", "/Z7", "/MT")

HEADER = r"""
struct Piece {
    Piece();
    ~Piece();
    int value;
};
void use(Piece *, int);
void branch_cleanup(int);
"""

PIECE = r"""
#include "piece.h"
volatile int ActivePieces;
Piece::Piece() : value(0) { ++ActivePieces; }
Piece::~Piece() { --ActivePieces; }
void use(Piece *piece, int value) { piece->value = value; }
"""

OWNER = r"""
#include "piece.h"
void branch_cleanup(int mode) {
    Piece first;
    if (mode) {
        Piece conditional;
        use(&conditional, 1);
    }
    Piece last;
    use(&last, 2);
}
"""

ENTRY = r"""
#include "piece.h"
int main() {
    branch_cleanup(0);
    branch_cleanup(1);
    return 0;
}
"""


def _require_toolchain():
    if (not (VC6_ROOT / "BIN" / "CL.EXE").is_file()
            or not (VC6_ROOT / "BIN" / "LINK.EXE").is_file()
            or shutil.which("wine") is None):
        pytest.skip("configured VC6 compiler, linker, and Wine are unavailable")


def _map_address(map_path: Path) -> int:
    found = []
    for line in map_path.read_text(errors="replace").splitlines():
        words = line.split()
        if len(words) >= 3 and words[1] == SYMBOL:
            found.append(int(words[2], 16))
    assert len(found) == 1, f"expected one MAP entry for {SYMBOL}, got {found}"
    return found[0]


def _registration_handler(root_code: bytes, root_address: int) -> int:
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    instructions = list(engine.disasm(root_code, root_address))
    assert sum(item.size for item in instructions) == len(root_code)
    sites = []
    for first, second in zip(instructions, instructions[1:]):
        if (first.mnemonic == "push" and first.op_str == "-1"
                and second.mnemonic == "push" and second.operands
                and second.operands[0].type == x86.X86_OP_IMM):
            sites.append(second.operands[0].imm & 0xFFFFFFFF)
    assert len(sites) == 1, f"expected one registration handler push, got {sites}"
    return sites[0]


def _file_offset(image: PeImage, address: int, size: int) -> int:
    matches = [offset + address - span.start
               for span, offset, backed_size in image.sections
               if span.start <= address and address + size <= span.start + backed_size]
    assert len(matches) == 1, f"expected one file-backed PE range for {address:#x}"
    return matches[0]


def _spans_and_record(root_address: int, root_code: bytes, child_code: bytes, tmp_path: Path):
    handler = _registration_handler(root_code, root_address)
    # The package's bounded VC6 EH child ends in its 10-byte FuncInfo handler
    # stub. The actual handler address is found through the linked root
    # registration; child length comes from the associated COFF section.
    assert len(child_code) > 10
    cold_start = handler - (len(child_code) - 10)
    spans = ((root_address, root_address + len(root_code)),
             (cold_start, cold_start + len(child_code)))
    span_text = ";".join(f"0x{low:08X}-0x{high:08X}" for low, high in spans)
    mapping = (f"// ORIGINAL: 0x{root_address:08X} test-owned {span_text} FILE\n"
               f"// symbol {SYMBOL}\nvoid branch_cleanup(int) {{}}\n")
    record, = read_text(mapping, tmp_path / "synthetic_mapping.cpp")
    return spans, record


def test_linked_branched_eh_predecessor_topology_and_damage(tmp_path):
    _require_toolchain()
    for name, contents in (("piece.h", HEADER), ("piece.cpp", PIECE),
                           ("owner.cpp", OWNER), ("entry.cpp", ENTRY)):
        (tmp_path / name).write_text(contents)

    command = (sys.executable, str(ROOT / "recovery" / "environment" / "wine.py"),
               "--tool=cl", "/nologo")
    objects = {}
    for name in ("owner", "piece", "entry"):
        objects[name] = compile_object(tmp_path / f"{name}.cpp", command, FLAGS)
        (tmp_path / f"{name}.obj").write_bytes(objects[name])

    owner_replica = compile_object(tmp_path / "owner.cpp", command, FLAGS)
    assert object_signature(Coff(owner_replica)) == object_signature(Coff(objects["owner"]))

    image_path = tmp_path / "branched-control.exe"
    map_path = tmp_path / "branched-control.map"
    link_command = (sys.executable, str(ROOT / "recovery" / "environment" / "wine.py"),
                    "--tool=link", "/nologo")
    linked = run_bounded(
        [*link_command, "/nodefaultlib", "/subsystem:console", "/incremental:no",
         "/fixed:no", "/opt:noref", "/opt:noicf", "/map:" + str(map_path),
         "/out:" + str(image_path), *(str(tmp_path / f"{name}.obj")
                                        for name in ("owner", "piece", "entry")),
         "libcmt.lib", "oldnames.lib", "kernel32.lib"],
        cwd=tmp_path, timeout=120)
    assert linked.returncode == 0, linked.stdout + linked.stderr
    assert image_path.is_file() and map_path.is_file()

    owner = Coff(objects["owner"])
    root = owner.function(SYMBOL)
    code_sections = owner.code_sections(root)
    assert len(code_sections) == 2 and code_sections[0] == root.section
    child_section, = code_sections[1:]
    assert owner.associations.get(child_section) == root.section
    root_code, _, _ = owner.section_subject(root.section)
    child_code, _, _ = owner.section_subject(child_section)

    root_address = _map_address(map_path)
    image_bytes = image_path.read_bytes()
    image = PeImage(image_bytes, sha256(image_bytes).hexdigest())
    linked_root_code = image.read(Span(root_address, root_address + len(root_code)))
    spans, record = _spans_and_record(root_address, linked_root_code, child_code, tmp_path)
    regions = bind_eh_regions(objects["owner"], SYMBOL, image, spans)
    assert len(regions) == 2
    assert regions[0].ownership.original_predecessors == (0xFFFFFFFF, 0, 0)
    assert regions[0].ownership.compiled_predecessors == (0xFFFFFFFF, 0, 0)

    compiled = subject_asm(objects["owner"], record, "test-owned-vc6-link")
    baseline = compare_subject(record, image_path, compiled)
    assert baseline.verdict.value == "BYTE_EXACT", baseline

    # Change only one valid predecessor word in FuncInfo metadata. Root and
    # cold code bytes remain identical, while the verifier must reject the
    # otherwise valid but noncanonical branch predecessor graph.
    info_section = regions[0].ownership.func_info_section
    info_data = owner.sections[info_section - 1].data
    magic = struct.pack("<I", 0x19930520)
    info_offsets = [at for at in range(len(info_data)) if info_data.startswith(magic, at)]
    assert len(info_offsets) == 1
    info_offset = info_offsets[0]
    state_count, = struct.unpack_from("<I", info_data, info_offset + 4)
    assert state_count == 3
    predecessor_offset = info_offset + 32 + 2 * 8
    assert struct.unpack_from("<I", info_data, predecessor_offset)[0] == 0
    section_header = 20 + (info_section - 1) * 40
    raw_pointer, = struct.unpack_from("<I", objects["owner"], section_header + 20)
    damaged = bytearray(objects["owner"])
    struct.pack_into("<I", damaged, raw_pointer + predecessor_offset, 1)
    damaged_obj = Coff(bytes(damaged))
    assert damaged_obj.section_subject(root.section) == owner.section_subject(root.section)
    assert damaged_obj.section_subject(child_section) == owner.section_subject(child_section)
    changed = compare_subject(record, image_path,
                              subject_asm(bytes(damaged), record, "metadata-only-damage"))
    assert changed.verdict.value == "MISMATCH"
    assert "predecessor topology" in changed.diagnostic

    invalid = bytearray(objects["owner"])
    struct.pack_into("<I", invalid, raw_pointer + predecessor_offset, 2)
    with pytest.raises(ValueError, match="predecessor"):
        bind_eh_regions(bytes(invalid), SYMBOL, image, spans)

    # Independently damage the linked image's original unwind table. A unique
    # alternate path prevents any path-keyed image cache from reusing baseline
    # bytes; the code layout and source record stay unchanged.
    info_va = regions[0].ownership.original_func_info
    unwind_map, = struct.unpack("<I", image.read(Span(info_va + 8, info_va + 12)))
    original_map = image.read(Span(unwind_map, unwind_map + 3 * 8))
    assert struct.unpack_from("<I", original_map, 2 * 8)[0] == 0
    changed_image = bytearray(image_bytes)
    map_file_offset = _file_offset(image, unwind_map, 3 * 8)
    struct.pack_into("<I", changed_image, map_file_offset + 2 * 8, 2)
    altered_image_path = tmp_path / "branched-control-original-topology-damaged.exe"
    altered_image_path.write_bytes(changed_image)
    with pytest.raises(SpanComparisonRefused, match="predecessor"):
        compare_subject(record, altered_image_path, compiled)
