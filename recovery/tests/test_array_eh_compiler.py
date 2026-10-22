"""Real VC6/linker control for complete array cleanup callback/helper ownership.

The fixture belongs to this test; it contains no executable-derived game code.
This is a physical EH binding control, not a runtime or strict target-identity proof.
"""

from hashlib import sha256
from pathlib import Path
import struct
import sys

import pytest

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
SYMBOL = "??1ArrayOwner@@QAE@XZ"
CALLBACK = "??1Element@@QAE@XZ"
VECTOR_HELPER = "??_M@YGXPAXIHP6EX0@Z@Z"

HEADER = r"""
void destroyed(int);
struct Element {
    Element();
    DESTRUCTOR
    int value;
};
struct ArrayOwner {
    char prefix[320];
    Element elements[3];
    ArrayOwner();
    ~ArrayOwner();
};
void use(Element *, int);
"""

PIECE = r"""
#include "element.h"
volatile int ActiveElements;
Element::Element() : value(0) { ++ActiveElements; }
void destroyed(int value) { ActiveElements -= value + 1; }
DESTRUCTOR
void use(Element *element, int value) { element->value = value; }
"""

OWNER = r"""
#include "element.h"
ArrayOwner::ArrayOwner() {}
ArrayOwner::~ArrayOwner() { use(&elements[0], 7); }
"""

ENTRY = r"""
#include "element.h"
int main() { ArrayOwner owner; return 0; }
"""


def _compile_array_fixture(tmp_path, local_callback, omit_frame=False):
    _require_toolchain()
    header = HEADER.replace("DESTRUCTOR", "~Element() { destroyed(value); }"
                            if local_callback else "~Element();")
    piece = PIECE.replace("DESTRUCTOR", "" if local_callback
                          else "Element::~Element() { destroyed(value); }")
    for name, contents in (("element.h", header), ("element.cpp", piece),
                           ("owner.cpp", OWNER), ("entry.cpp", ENTRY)):
        (tmp_path / name).write_text(contents)
    command = (sys.executable, str(ROOT / "recovery/environment/wine.py"),
               "--tool=cl", "/nologo")
    # This is a test-owned metadata control, not a production recipe change.
    flags = tuple("/Oy" if omit_frame and flag == "/Oy-" else flag for flag in FLAGS)
    objects = {}
    for name in ("owner", "element", "entry"):
        objects[name] = compile_object(tmp_path / f"{name}.cpp", command, flags)
        (tmp_path / f"{name}.obj").write_bytes(objects[name])
    replica = compile_object(tmp_path / "owner.cpp", command, flags)
    assert object_signature(Coff(replica)) == object_signature(Coff(objects["owner"]))
    image_path, map_path = tmp_path / "array-control.exe", tmp_path / "array-control.map"
    link_command = (sys.executable, str(ROOT / "recovery/environment/wine.py"),
                    "--tool=link", "/nologo")
    linked = run_bounded(
        [*link_command, "/nodefaultlib", "/subsystem:console", "/incremental:no",
         "/fixed:no", "/opt:noref", "/opt:noicf", "/map:" + str(map_path),
         "/out:" + str(image_path), *(str(tmp_path / f"{name}.obj")
                                      for name in ("owner", "element", "entry")),
         "libcmt.lib", "oldnames.lib", "kernel32.lib"], cwd=tmp_path, timeout=120)
    assert linked.returncode == 0, linked.stdout + linked.stderr
    addresses = [int(words[2], 16) for line in map_path.read_text(errors="replace").splitlines()
                 if len(words := line.split()) >= 3 and words[1] == SYMBOL]
    assert len(addresses) == 1
    root_address, = addresses
    owner = Coff(objects["owner"])
    root = owner.function(SYMBOL)
    code_sections = owner.code_sections(root)
    assert len(code_sections) == 2 and code_sections[0] == root.section
    child_section, = code_sections[1:]
    assert owner.associations.get(child_section) == root.section
    root_code, _, _ = owner.section_subject(root.section)
    child_code, _, _ = owner.section_subject(child_section)
    image_bytes = image_path.read_bytes()
    image = PeImage(image_bytes, sha256(image_bytes).hexdigest())
    linked_root = image.read(Span(root_address, root_address + len(root_code)))
    handler = _registration_handler(linked_root, root_address)
    cold_start = handler - (len(child_code) - 10)
    spans = ((root_address, root_address + len(root_code)),
             (cold_start, cold_start + len(child_code)))
    span_text = ";".join(f"0x{low:08X}-0x{high:08X}" for low, high in spans)
    record, = read_text(f"// ORIGINAL: 0x{root_address:08X} test-owned {span_text} FILE\n"
                        f"// symbol {SYMBOL}\nArrayOwner::~ArrayOwner() {{}}\n",
                        tmp_path / "test_mapping.cpp")
    return objects["owner"], owner, root, child_section, image_path, image, spans, record


@pytest.mark.parametrize("local_callback", [False, True])
@pytest.mark.parametrize("omit_frame", [False, True])
def test_linked_array_cleanup_callback_helper_and_damage(tmp_path, local_callback, omit_frame):
    raw, owner, root, child_number, image_path, image, spans, record = _compile_array_fixture(
        tmp_path, local_callback, omit_frame)
    child = owner.sections[child_number - 1]
    targets = [(offset, owner.symbols[index], kind)
               for offset, index, kind in child.relocations]
    callback_sites = [offset for offset, symbol, kind in targets
                      if symbol.name == CALLBACK and kind == 6]
    helper_sites = [offset for offset, symbol, kind in targets
                    if symbol.name == VECTOR_HELPER and kind == 20]
    # These assertions establish the compiler actually emitted this control's
    # callback/helper pair; a mere successful compile is not this evidence.
    assert len(callback_sites) == len(helper_sites) == 1
    callback_site, = callback_sites
    helper_site, = helper_sites
    callback_symbol, = [symbol for offset, symbol, _ in targets if offset == callback_site]
    assert bool(callback_symbol.section) == local_callback
    if local_callback and omit_frame:
        debug_children = [owner.sections[number - 1]
                          for number, parent in owner.associations.items()
                          if parent == callback_symbol.section
                          and owner.sections[number - 1].name == ".debug$F"]
        assert len(debug_children) == 1
        assert len(debug_children[0].data) == 16
        assert debug_children[0].relocations == ((0, callback_symbol.index, 7),)
    assert len(child.data) > 10 and child.data[helper_site - 1] == 0xE8
    assert child.data[helper_site + 4] == 0xC3
    assert child.data[callback_site - 1] == 0x68
    regions = bind_eh_regions(raw, SYMBOL, image, spans)
    assert len(regions) == 2
    assert regions[0].ownership.original_predecessors == (0xFFFFFFFF,)
    assert regions[0].ownership.compiled_predecessors == (0xFFFFFFFF,)
    compiled = subject_asm(raw, record, "test-owned-array-control")
    assert compare_subject(record, image_path, compiled).verdict.value == "BYTE_EXACT"

    # Alter a still-valid array count, not a relocated operand. Physical
    # ownership remains valid, but complete code comparison must observe it.
    assert child.data[callback_site + 4:callback_site + 6] == b"\x6a\x03"
    raw_pointer, = struct.unpack_from("<I", raw, 20 + (child_number - 1) * 40 + 20)
    changed = bytearray(raw)
    changed[raw_pointer + callback_site + 5] = 4
    assert compare_subject(record, image_path,
                           subject_asm(bytes(changed), record, "count-damage")).verdict.value == "MISMATCH"

    # Changing the terminal action RET into NOP leaves the linked root alone;
    # a primary-only binder would incorrectly continue to accept this input.
    malformed = bytearray(raw)
    malformed[raw_pointer + helper_site + 4] = 0x90
    with pytest.raises(ValueError):
        bind_eh_regions(bytes(malformed), SYMBOL, image, spans)

    # The linked original's callback must itself be executable and outside
    # both owned regions; preserving its HIGHLOW site does not waive that.
    original = image_path.read_bytes()
    original_changed = bytearray(original)
    site = _file_offset(image, spans[1][0] + callback_site, 4)
    struct.pack_into("<I", original_changed, site, spans[0][0])
    invalid_image = tmp_path / "array-control-original-callback-damaged.exe"
    invalid_image.write_bytes(original_changed)
    with pytest.raises(SpanComparisonRefused):
        compare_subject(record, invalid_image, compiled)

    # The callback's object addend is not a wildcard address or partial body.
    added = bytearray(raw)
    struct.pack_into("<I", added, raw_pointer + callback_site, 1)
    with pytest.raises(ValueError):
        bind_eh_regions(bytes(added), SYMBOL, image, spans)
