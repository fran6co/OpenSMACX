"""Actual-source ButtonGroup cleanup, inline destruction, and array order.

The host model uses the real class declaration, constructor/close definitions,
and in-class destructor. Pointer width and the host array stride are not target
ABI claims; the pinned and native checks cover the original callback entry.
"""
from __future__ import annotations

import hashlib
import os
import resource
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.codeview import class_layout
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file

from test_spritelist_base_lifetime import extract_definition, repo_root


ROOT = repo_root()


def _class_declaration() -> str:
    header = (ROOT / "src/buttongroup.h").read_text()
    declaration = extract_definition(header, "class ButtonGroup")
    assert "~ButtonGroup()" in declaration
    return declaration + ";"


def _ctor() -> str:
    return extract_definition((ROOT / "src/buttongroup.cpp").read_text(),
                              "ButtonGroup::ButtonGroup()")


def _close() -> str:
    return extract_definition((ROOT / "src/buttongroup.cpp").read_text(),
                              "void ButtonGroup::close()")


def _host_source(class_declaration: str, constructor: str, close: str) -> str:
    return r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
struct BaseButton { int value; };
static void *clear_addresses[64];
static size_t clear_sizes[64];
static int clear_values[64];
static unsigned int clear_count;
static void reset_trace() { clear_count = 0; }
static void *host_memset(void *destination, int value, size_t size) {
    assert(clear_count < 64);
    clear_addresses[clear_count] = destination;
    clear_sizes[clear_count] = size;
    clear_values[clear_count] = value;
    ++clear_count;
    return ::memset(destination, value, size);
}
#define private public
''' + class_declaration + r'''
#undef private
#define memset host_memset
''' + constructor + "\n" + close + r'''
#undef memset

static void assert_empty(const ButtonGroup &group) {
    assert(group.count_ == 0);
    assert(group.field_8C_ == 0);
    assert(group.field_90_ == 0);
    for (unsigned int i = 0; i < 32; ++i) assert(group.buttons_[i] == 0);
}
static void assert_clear_event(unsigned int index, const ButtonGroup &group) {
    assert(clear_addresses[index] == static_cast<const void *>(&group.buttons_[0]));
    assert(clear_sizes[index] == sizeof(group.buttons_));
    assert(clear_values[index] == 0);
}

int main() {
    BaseButton live_button = { 17 };
    {
        ButtonGroup group;
        assert(clear_count == 1);
        assert_clear_event(0, group);
        assert_empty(group);
        // The constructor intentionally leaves these two fields alone; seed
        // them only after construction, then verify live close preserves them.
        group.field_84_ = 0x12345678u;
        group.field_88_ = 0x87654321u;
        group.buttons_[0] = &live_button;
        group.buttons_[31] = &live_button;
        group.count_ = 2;
        group.field_8C_ = 9;
        group.field_90_ = 13;

        reset_trace();
        group.close();
        assert(clear_count == 1);
        assert_clear_event(0, group);
        assert_empty(group);
        assert(group.field_84_ == 0x12345678u);
        assert(group.field_88_ == 0x87654321u);

        // Empty/null close is still an ordinary reset and can be repeated.
        group.close();
        assert(clear_count == 2);
        assert_clear_event(1, group);
        assert_empty(group);
        assert(group.field_84_ == 0x12345678u);
        assert(group.field_88_ == 0x87654321u);
    }

    // The inline destructor calls the same real close body exactly once.
    // The preceding explicit closes were observed separately above.
    assert(clear_count == 3);

    unsigned long expected[3];
    BaseButton array_buttons[3] = {{1}, {2}, {3}};
    reset_trace();
    {
        ButtonGroup groups[3];
        for (unsigned int i = 0; i < 3; ++i) {
            expected[i] = (unsigned long)(uintptr_t)&groups[i].buttons_[0];
            groups[i].buttons_[i] = &array_buttons[i];
            groups[i].count_ = i + 1;
            groups[i].field_8C_ = 100 + i;
            groups[i].field_90_ = 200 + i;
        }
        reset_trace();
    }
    assert(clear_count == 3);
    for (unsigned int i = 0; i < 3; ++i) {
        assert((unsigned long)(uintptr_t)clear_addresses[i] == expected[2 - i]);
        assert(clear_sizes[i] == 32 * sizeof(BaseButton *));
        assert(clear_values[i] == 0);
    }
    return 0;
}
'''


def _run(compiler: str, source_text: str, directory: Path, stem: str) -> subprocess.CompletedProcess:
    source = directory / (stem + ".cpp")
    binary = directory / stem
    source.write_text(source_text)
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(binary)], capture_output=True, text=True,
                           timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr

    def no_core_dumps():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    return subprocess.run([str(binary)], capture_output=True, text=True,
                          preexec_fn=no_core_dumps, timeout=15)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_inline_destruction_resets_live_groups_and_arrays(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable for bounded C++98 host control")
    declaration = _class_declaration()
    constructor = _ctor()
    close = _close()
    assert "memset(buttons_, 0, sizeof(buttons_));" in close
    assert "close();" in declaration
    with tempfile.TemporaryDirectory(prefix="buttongroup-lifetime-") as temp:
        directory = Path(temp)
        actual = _run(compiler, _host_source(declaration, constructor, close), directory, "actual")
        assert actual.returncode == 0, actual.stdout + actual.stderr

        # Omitting the in-class cleanup must lose both single-object and array
        # destruction observations, so the same actual-source harness rejects it.
        missing_close = declaration.replace("~ButtonGroup() { close(); }",
                                            "~ButtonGroup() {}", 1)
        assert missing_close != declaration
        damaged_dtor = _run(compiler, _host_source(missing_close, constructor, close),
                            directory, "missing_destructor_close")
        assert damaged_dtor.returncode != 0, "a destructor without close must fail the lifetime witness"

        missing_array_clear = close.replace("memset(buttons_, 0, sizeof(buttons_));",
                                            "memset(buttons_, 0, 0);", 1)
        assert missing_array_clear != close
        damaged_clear = _run(compiler, _host_source(declaration, constructor, missing_array_clear),
                             directory, "omitted_button_array_clear")
        assert damaged_clear.returncode != 0, "omitting the real pointer-array clear must fail reset checks"


PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
DTOR = "??1ButtonGroup@@QAE@XZ"
CLOSE = "?close@ButtonGroup@@QAEXXZ"
CTOR = "??0ButtonGroup@@QAE@XZ"


def _image():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original is required for native lifetime target checks")
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    return PeImage(raw, PIN)


def _record(unit, address):
    record, = (row for row in read_file(ROOT / unit) if row.address == address)
    return record


def _binding(unit, address):
    record = _record(unit, address)
    return FunctionBinding(Origin(PIN, "pe:va", address),
                           tuple(Span(*span) for span in record.image_spans),
                           record.symbol or record.name, unit)


def _decode(raw, address=0):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    operations = tuple(decoder.disasm(raw, address))
    assert operations and operations[-1].address + operations[-1].size == address + len(raw)
    return operations


def test_original_standalone_cleanup_and_real_array_callback():
    image = _image()
    destructor = _decode(image.read(Span(0x4325B0, 0x4325B5)), 0x4325B0)
    assert len(destructor) == 1 and destructor[0].mnemonic == "jmp"
    assert destructor[0].operands[0].type == X86_OP_IMM
    assert destructor[0].operands[0].imm == 0x62B7F0
    cleanup = _decode(image.read(Span(0x48D540, 0x48D54A)), 0x48D540)
    assert [op.mnemonic for op in cleanup] == ["mov", "jmp"]
    assert cleanup[0].reg_name(cleanup[0].operands[0].reg) == "ecx"
    assert cleanup[0].operands[1].type == X86_OP_IMM
    assert cleanup[0].operands[1].imm == 0x8577F0
    assert cleanup[1].operands[0].type == X86_OP_IMM
    assert cleanup[1].operands[0].imm == 0x62B7F0
    # Independently decoded bounded part of the original MainInterface ctor:
    # this is a genuine four-member array, not a manufactured destructor use.
    array = _decode(image.read(Span(0x45EF62, 0x45EF83)), 0x45EF62)
    pushes = [op for op in array if op.mnemonic == "push"]
    assert len(pushes) == 5 and all(op.operands[0].type == X86_OP_IMM for op in pushes[:4])
    assert [op.operands[0].imm for op in pushes[:4]] == [0x4325B0, 0x62B7C0, 4, 0x94]
    receiver, = (op for op in array if op.mnemonic == "lea")
    assert receiver.operands[1].type == X86_OP_MEM
    assert receiver.reg_name(receiver.operands[1].mem.base) == "esi"
    assert receiver.operands[1].mem.disp == 0x1E878
    assert pushes[-1].operands[0].type == X86_OP_REG
    assert pushes[-1].operands[0].reg == receiver.operands[0].reg
    assert array[-1].mnemonic == "call" and array[-1].operands[0].imm == 0x6457C2


def _check_native_cleanup(obj, unit, address, image):
    global_object, = (row for row in read_objects(ROOT, PIN)
                      if row.symbol == "?PrefWinButtonGroup@@3VButtonGroup@@A")
    targets = {CLOSE: _record("src/buttongroup.cpp", 0x62B7F0).address,
               global_object.symbol: global_object.origin.address}
    if address != 0x4325B0:
        # Bind the old intermediate target too. Otherwise the reverting-header
        # control is only UNKNOWN for an unbound reference, not a measured
        # wrong-target failure against the original direct close jump.
        targets[DTOR] = _record("src/maininterface.cpp", 0x4325B0).address
    row, = check_objects([_binding(unit, address)], {unit: obj}, image,
                         external_targets=targets)
    return row


@pytest.mark.parametrize("profile_index", (0, 1))
def test_native_inline_cleanup_keeps_callback_and_rejects_out_of_line_regression(profile_index):
    from recovery.environment.wine import VC6_ROOT
    if (not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None
            or not (ROOT / "build/compile_commands.json").is_file()):
        pytest.skip("prepared VC6/Wine compiler is required")
    image = _image()
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    flags = settings.flag_sets[profile_index] + settings.layout_flags
    objects = {unit: Coff(compiler(ROOT / unit, (), flags)) for unit in
               ("src/buttongroup.cpp", "src/prefwin.cpp", "src/maininterface.cpp")}
    layout = class_layout(objects["src/maininterface.cpp"], "ButtonGroup")
    assert layout["size"] == 0x94 and layout["bases"] == []
    assert layout["members"] == {"buttons_": 0, "count_": 0x80, "field_84_": 0x84,
                                  "field_88_": 0x88, "field_8C_": 0x8C, "field_90_": 0x90}
    standalone = _check_native_cleanup(objects["src/maininterface.cpp"],
                                       "src/maininterface.cpp", 0x4325B0, image)
    assert standalone["result"] == "pass", standalone
    cleanup = _check_native_cleanup(objects["src/prefwin.cpp"], "src/prefwin.cpp", 0x48D540, image)
    assert cleanup["result"] == "pass", cleanup
    with pytest.raises(ValueError):
        objects["src/buttongroup.cpp"].function(DTOR)
    main = objects["src/maininterface.cpp"]
    constructor = main.function(_record("src/maininterface.cpp", 0x45EF10).symbol
                                or _record("src/maininterface.cpp", 0x45EF10).name)
    _, references, _ = main.subject(constructor)
    assert DTOR in [main.symbols[int(ref.target)].name for ref in references]
    # The real header edit affects ordinary teardown, not its cleanup effect.
    # A compiler overlay reverting ONLY inline visibility must restore the
    # wrong direct target in PrefWin; the strict comparator must detect it.
    header = (ROOT / "src/buttongroup.h").read_bytes()
    assert header.count(b"~ButtonGroup() { close(); }") == 1
    damaged = Coff(compiler.overlay(ROOT / "src/prefwin.cpp", {
        "src/buttongroup.h": header.replace(b"~ButtonGroup() { close(); }", b"~ButtonGroup();", 1)
    }, (), flags))
    refusal = _check_native_cleanup(damaged, "src/prefwin.cpp", 0x48D540, image)
    assert refusal["result"] == "fail", refusal
