"""Behavior and pinned-storage controls for Caviar's shared colour source."""

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.cpp_storage_audit import storage_findings
from match_recovery.object_source import parse_objects
from test_caviar_scene_product_source import extract_definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/caviar.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ADDRESS_SPACE = "pe:va"
COLOUR_SYMBOL = "?CaviarColourSource@?%src/caviar.cpp@@3PAEA"
COLOUR_SPAN = Span(0x009B9C38, 0x009BB438)


def source_binding():
    rows = parse_objects(SOURCE.read_text(encoding="utf-8"), "src/caviar.cpp", PIN,
                         address_space=ADDRESS_SPACE)
    matches = [row for row in rows if row.symbol == COLOUR_SYMBOL]
    assert len(matches) == 1
    return matches[0]


def caviar_storage_audit(text):
    return storage_findings({"src/caviar.cpp": text}, pin=PIN,
                            address_space=ADDRESS_SPACE, symbol_format="msvc")


def extracted_source():
    text = SOURCE.read_text(encoding="utf-8")
    return (extract_definition(text, "render_mono"),
            extract_definition(text, "object_start"))


FIXTURE = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

struct Buffer {};
struct VOX_ScnRenderHnd { uint8_t *colour_table_; };
struct VOX_Vect {};
struct VOX_Matrix {};

static uint8_t CaviarColourSource[0x1800];
static uint8_t decoy_colour_source[0x1800];
static uint8_t expected_colour;
static int render_calls;

void * __cdecl caviar_memset_call(void *dst, int value, unsigned count) {
    return memset(dst, value, count);
}

struct Caviar {
    uint32_t field_4_;
    uint8_t *instance_palettes_[2];
    void render(Buffer *, int, int, int);
    void render_mono(Buffer *, int, int, int, int);
    static void __cdecl object_start(VOX_ScnRenderHnd *, long, VOX_Vect *,
                                     VOX_Matrix *, int32_t *, VOX_Vect *);
};

struct Scene {
    Caviar *renderer_;
    uint8_t *render_argument_;
};
static Scene CaviarScene;

void Caviar::render(Buffer *, int, int, int) {
    assert((field_4_ & 8) != 0);
    assert((field_4_ & 0x10) == 0);
    for (unsigned i = 0; i != 0x1800; ++i)
        assert(CaviarColourSource[i] == expected_colour);
    ++render_calls;
}

ACTUAL_RENDER_MONO

ACTUAL_OBJECT_START

static void require_table(const uint8_t *table, uint8_t expected) {
    for (unsigned i = 0; i != 0x1800; ++i)
        assert(table[i] == expected);
}

int main() {
    Caviar renderer = {};
    CaviarScene.renderer_ = &renderer;
    Buffer buffer;
    expected_colour = 0xB9;
    memset(CaviarColourSource, 0xA5, sizeof(CaviarColourSource));
    renderer.field_4_ = 0x24;
    renderer.render_mono(&buffer, 2, 3, 0x123456B9, 0x40);
    assert(render_calls == 1);
    assert(renderer.field_4_ == 0x24);
    require_table(CaviarColourSource, expected_colour);

    uint8_t output[0x1800];
    uint8_t scene_argument[0x1800];
    uint8_t palette_zero[0x1800];
    uint8_t palette_one[0x1800];
    memset(CaviarColourSource, 0x31, sizeof(CaviarColourSource));
    memset(scene_argument, 0x52, sizeof(scene_argument));
    memset(palette_zero, 0x73, sizeof(palette_zero));
    memset(palette_one, 0x94, sizeof(palette_one));
    renderer.instance_palettes_[0] = palette_zero;
    renderer.instance_palettes_[1] = palette_one;
    CaviarScene.render_argument_ = scene_argument;
    VOX_ScnRenderHnd handle = { output };
    VOX_Vect position = {};
    VOX_Matrix orientation = {};

    renderer.field_4_ = 0x18; // bit 3 wins even when bit 4 is also set
    Caviar::object_start(&handle, 1, &position, &orientation, 0, &position);
    require_table(output, 0x31);
    renderer.field_4_ = 0x10;
    Caviar::object_start(&handle, 1, &position, &orientation, 0, &position);
    require_table(output, 0x52);
    renderer.field_4_ = 0;
    Caviar::object_start(&handle, 1, &position, &orientation, 0, &position);
    require_table(output, 0x94);
    return 0;
}
'''


def host_compilers():
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("host GCC or Clang is required for extracted-source behavior control")
    return compilers


def host_run(tmp_path, render_body, object_body, compiler):
    program = FIXTURE.replace("ACTUAL_RENDER_MONO", render_body)
    program = program.replace("ACTUAL_OBJECT_START", object_body)
    source_file = tmp_path / "caviar_colour_source.cpp"
    executable = tmp_path / ("caviar_colour_source_" + Path(compiler).name.replace("+", "p"))
    source_file.write_text(program, encoding="utf-8")
    return subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                           str(source_file), "-o", str(executable)],
                          capture_output=True, text=True, timeout=60), executable


def test_current_binding_has_source_declared_bounded_identity():
    binding = source_binding()
    assert binding.unit == "src/caviar.cpp"
    assert binding.origin.image_sha256 == PIN
    assert binding.origin.address_space == ADDRESS_SPACE
    assert binding.origin.address == COLOUR_SPAN.start
    assert binding.span == COLOUR_SPAN
    assert binding.evidence is not None
    assert binding.evidence.image_sha256 == PIN
    assert binding.evidence.address_space == ADDRESS_SPACE
    assert binding.evidence.address == 0x00617620
    assert binding.emitted_symbol == COLOUR_SYMBOL


def test_actual_source_has_no_duplicate_global_identity_hint():
    counts, per_file, findings = caviar_storage_audit(SOURCE.read_text(encoding="utf-8"))
    assert counts["duplicate global identities"] == 0
    assert per_file.get("src/caviar.cpp", {}).get("duplicate global identities", 0) == 0
    assert not [finding for finding in findings
                if finding["rule"] == "duplicate global identities"]


def test_reintroduced_colour_address_hint_is_detected():
    text = SOURCE.read_text(encoding="utf-8")
    declaration = "uint8_t CaviarColourSource[0x1800];"
    assert text.count(declaration) == 1
    damaged = text.replace(declaration, declaration + "  // 0x009B9C38", 1)
    counts, per_file, findings = caviar_storage_audit(damaged)
    assert counts["duplicate global identities"] == 1
    assert per_file["src/caviar.cpp"]["duplicate global identities"] == 2
    related = [finding for finding in findings
               if finding["rule"] == "duplicate global identities"
               and finding.get("address") == COLOUR_SPAN.start]
    assert len(related) == 2
    sites = related[0]["related"]
    assert {site["kind"] for site in sites} == {"address_comment", "source_object"}
    assert {site["symbol"] for site in sites} == {
        "CaviarColourSource", "?%src/caviar.cpp::CaviarColourSource"}
    assert all(set(finding["symbols"]) == {
        "CaviarColourSource", "?%src/caviar.cpp::CaviarColourSource"} for finding in related)


def test_extracted_current_producer_and_consumer_execute_expected_paths(tmp_path):
    render_body, object_body = extracted_source()
    for compiler in host_compilers():
        compilation, executable = host_run(tmp_path, render_body, object_body, compiler)
        assert compilation.returncode == 0, compilation.stderr
        run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
        assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("damage", [
    "truncate_fill",
    "wrong_fill_source",
    "wrong_render_flag",
    "omitted_flag_clear",
    "wrong_consumer_source",
    "wrong_consumer_flag",
])
def test_executable_control_rejects_semantic_damage(tmp_path, damage):
    render_body, object_body = extracted_source()
    if damage == "truncate_fill":
        assert "colour & 0xff, 0x1800" in render_body
        render_body = render_body.replace("colour & 0xff, 0x1800",
                                          "colour & 0xff, 0x17ff", 1)
    elif damage == "wrong_fill_source":
        assert "caviar_memset_call(CaviarColourSource," in render_body
        render_body = render_body.replace("caviar_memset_call(CaviarColourSource,",
                                          "caviar_memset_call(decoy_colour_source,", 1)
    elif damage == "wrong_render_flag":
        assert "field_4_ |= 8;" in render_body
        render_body = render_body.replace("field_4_ |= 8;", "field_4_ |= 0x10;", 1)
    elif damage == "omitted_flag_clear":
        assert "field_4_ &= ~0x08;" in render_body
        render_body = render_body.replace("    field_4_ &= ~0x08;", "", 1)
    elif damage == "wrong_consumer_source":
        assert "memcpy(handle->colour_table_, CaviarColourSource, 0x1800);" in object_body
        object_body = object_body.replace(
            "memcpy(handle->colour_table_, CaviarColourSource, 0x1800);",
            "memcpy(handle->colour_table_, CaviarScene.render_argument_, 0x1800);", 1)
    else:
        assert "if (self.field_4_ & 8)" in object_body
        object_body = object_body.replace("if (self.field_4_ & 8)",
                                          "if (self.field_4_ & 4)", 1)

    for compiler in host_compilers():
        compilation, executable = host_run(tmp_path, render_body, object_body, compiler)
        assert compilation.returncode == 0, compilation.stderr
        run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
        assert run.returncode != 0, f"semantic damage {damage} unexpectedly passed with {compiler}"


def test_pinned_storage_initial_bytes_and_absolute_producer_consumer_operands():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable for Caviar storage control")
    capstone = pytest.importorskip("capstone")
    from match_recovery.pe_coff import PeImage

    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    original = PeImage(raw, PIN)
    assert original.read_initial(COLOUR_SPAN) == bytes(0x1800)
    assert original.absolute_sites(COLOUR_SPAN) == frozenset()

    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    decoder.detail = True
    producer = list(decoder.disasm(original.read(Span(0x00617620, 0x0061766F)),
                                   0x00617620))
    consumer = list(decoder.disasm(original.read(Span(0x00618DA0, 0x00618E06)),
                                   0x00618DA0))

    def operands_at(instructions, address):
        instruction, = (item for item in instructions if item.address == address)
        return instruction.mnemonic, [operand.imm & 0xffffffff
                                      for operand in instruction.operands
                                      if operand.type == X86_OP_IMM]

    from capstone.x86_const import X86_OP_IMM

    def immediate_values(instructions):
        return {operand.imm & 0xffffffff for instruction in instructions
                for operand in instruction.operands if operand.type == X86_OP_IMM}

    # The writer passes the full table and byte source to memset; the consumer
    # passes this same original address to memcpy when the renderer bit is set.
    assert operands_at(producer, 0x00617625) == ("and", [0xFF])
    assert operands_at(producer, 0x0061762A) == ("push", [0x1800])
    assert operands_at(producer, 0x00617632) == ("push", [0x009B9C38])
    assert operands_at(producer, 0x00617637) == ("call", [0x006465F0])
    assert operands_at(consumer, 0x00618DB1) == ("push", [0x1800])
    assert operands_at(consumer, 0x00618DB6) == ("push", [0x009B9C38])
    assert operands_at(consumer, 0x00618DBF) == ("call", [0x00645930])
    assert 0x009B9C38 in immediate_values(producer)
    assert 0x009B9C38 in immediate_values(consumer)
