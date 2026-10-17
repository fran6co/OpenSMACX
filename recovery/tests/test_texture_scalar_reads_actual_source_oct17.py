"""Actual-source and scalar controls for borrowed-raster recovery.

The large draw_trans2 body is allowed to change as one mapped body. The other
55 mappings and all claims remain fixed. Scalar host checks cover only the
nine related reads remaining in its three sibling bodies; they do not prove
the rasterizer's behavior or renderer output.
"""

from pathlib import Path
import hashlib
import os
import re
import shutil
import subprocess
import textwrap

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text
from match_recovery.source_inventory import image_pin


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BASELINE = "f47ece84bbccd8e9ea34b440e6e0b7d4640853b2"
SPANS = (
    (0x0061C090, 0x0061C622, 1426,
     "e3fbd4bbca9d3a5f33be2b47ff39b036ca21a5d557eead791fa14089744f48ed"),
    (0x00619D80, 0x0061A23F, 1215,
     "b4831c137261abe19f3fe217ffb21cb2b4f848bca8697d228160b74fef9caba9"),
    (0x006212B0, 0x00621868, 1464,
     "82a106117db5e3d50c4464d53045b9c43b9155108c73a79db5fc23f0922a7d6c"),
    (0x00622810, 0x00622DAE, 1438,
     "869eccdfb3240259cf0e92cf4ead61cf0b8867305e696095dc335544e6717865"),
)

# The draw_trans2 body is replaced independently. The nine sibling reads are
# (old spelling, faithful expression using the declared object, occurrences).
SIBLING_REPLACEMENTS = (
    ("*reinterpret_cast<unsigned char *>(\n                                    &(TextureRasterLightThreshold))",
     "TextureRasterLightThreshold", 1),
    ("*(unsigned short *)&(TextureRasterLightPackA.low_word)",
     "TextureRasterLightPackA.low_word", 2),
    ("*(unsigned char *)&(TextureRasterLightThreshold)",
     "static_cast<unsigned char>(TextureRasterLightThreshold)", 2),
    ("*(unsigned int *)&(TextureRasterScratch528)",
     "static_cast<unsigned int>(TextureRasterScratch528)", 4),
)


def _definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/":
            state = "line"
            index += 1
        elif char == "/" and nxt == "*":
            state = "block"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return start, index + 1, text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _candidate_projection(baseline, candidate):
    signature = "int Texture::draw_trans2(Buffer *a1, Vert *a2, int a3)"
    old_start, old_end, old_body = _definition(baseline, signature)
    _, _, candidate_body = _definition(candidate, signature)
    projected = baseline[:old_start] + candidate_body + baseline[old_end:]
    assert old_body != ""
    total = 0
    for old, new, count in SIBLING_REPLACEMENTS:
        assert projected.count(old) == count, (old, projected.count(old), count)
        projected = projected.replace(old, new)
        total += count
    assert total == 9
    return projected


def _origin_projection(origin):
    if origin is None:
        return None
    return tuple((lesson.kind, lesson.key, lesson.prose,
                  tuple(line_text for _, line_text in lesson.lines))
                 for lesson in origin)


def _stable_mapping(record):
    return (
        record.address, record.mode, record.state, record.path, record.name,
        record.image_spans, record.symbol, record.body, record.context,
        record.kind, record.exclusion, record.extract_error, record.byte_exact,
        record.semantic, record.recipe, record.levers, record.ruled_out,
        record.unrecoverable, record.deferred, _origin_projection(record.origin),
        record.source_role, record.source_role_reason, record.function_fragment,
    )


def test_texture_source_allows_only_draw_trans2_body_and_nine_sibling_reads():
    baseline = subprocess.check_output(
        ["git", "show", f"{BASELINE}:src/texture.cpp"],
        cwd=ROOT, text=True, timeout=30)
    current = (ROOT / "src/texture.cpp").read_text(encoding="utf-8")
    expected = _candidate_projection(baseline, current)
    assert current == expected

    original_records = read_text(baseline, ROOT / "src/texture.cpp")
    current_records = read_text(current, ROOT / "src/texture.cpp")
    assert len(original_records) == len(current_records) == 56
    assert [_stable_mapping(row) for row in current_records] == [
        _stable_mapping(row) for row in original_records]

    header = current
    assert "static int TextureRasterLightThreshold;" in header
    assert "static int TextureRasterScratch528;" in header
    assert "static TextureRasterWordPair TextureRasterLightPackA;" in header
    assert "unsigned short low_word;" in header
    assert "unsigned short high_word;" in header
    assert "unsigned char bytes[4];" in header
    assert "static TextureRasterDwordHighWordView TextureRasterScratch4C8;" in header
    assert "static TextureRasterDwordHighWordView TextureRasterScratch4E4;" in header
    assert "static TextureRasterDwordHighWordView TextureRasterUAccumulator;" in header
    assert "static TextureRasterDwordHighWordView TextureRasterVAccumulator;" in header


def test_pinned_image_and_complete_rasterizer_spans():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required; set OPENSMACX_IMAGE")
    assert image_pin(ROOT) == PIN
    image_bytes = image_path.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PIN
    image = PeImage(image_bytes, image_pin(ROOT))
    for start, end, expected_size, expected_hash in SPANS:
        raw = image.read(Span(start, end))
        assert len(raw) == expected_size
        assert hashlib.sha256(raw).hexdigest() == expected_hash

    # Independent pinned disassembly anchors: the original consumes the key
    # as a byte, the packed-light member as a word, and Scratch528 as a dword.
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86_const import X86_OP_MEM

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded = {}
    for start, end, _, _ in SPANS:
        cursor = start
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        for insn in instructions:
            assert insn.address == cursor
            cursor += insn.size
            decoded[insn.address] = insn
        assert cursor == end

    def absolute_read(address, mnemonic, size, displacement):
        instruction = decoded[address]
        assert instruction.mnemonic == mnemonic
        memory_operands = [operand for operand in instruction.operands
                           if operand.type == X86_OP_MEM]
        assert any(operand.size == size and operand.mem.disp == displacement
                   for operand in memory_operands)

    for address in (0x0061C590, 0x0061A1CE, 0x006217D2, 0x00622D1B):
        absolute_read(address, "mov", 1, 0x009BB514)
    for address in (0x0061C4C2, 0x0062174C, 0x00622C95):
        absolute_read(address, "mov", 2, 0x009BB52C)
    absolute_read(0x0061C5A9, "add", 4, 0x009BB528)
    absolute_read(0x0061A1A3, "mov", 1, 0x009BB4CA)
    absolute_read(0x0061A1A9, "mov", 1, 0x009BB4E6)


def _compile_and_run(tmp_path, compiler, body, label):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    cpp_path = tmp_path / f"{label}.cpp"
    binary = tmp_path / label
    cpp_path.write_text(body, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp_path), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def _rhs(body, pattern, label):
    matches = re.findall(pattern, body, flags=re.S)
    assert len(matches) == 1, f"{label}: expected one actual-source RHS, got {len(matches)}"
    return matches[0].strip()


def _actual_scalar_expressions(cpp):
    owners = (
        ("void Texture::draw_trans(Buffer *buffer, Vert *vertices, int *zvals,",
         r"const\s+unsigned\s+char\s+key\s*=\s*(.*?);", "draw_trans key"),
        ("void Texture::sub_6212b0(Buffer *buf, Vert *verts, int *zvals, int count,",
         r"unsigned\s+char\s+threshold\s*=\s*(.*?);", "sub_6212b0 threshold"),
        ("void Texture::sub_622810(Buffer *buf, Vert *verts, int *zvals, int count,",
         r"unsigned\s+char\s+threshold\s*=\s*(.*?);", "sub_622810 threshold"),
    )
    keys = []
    for signature, pattern, label in owners:
        _, _, body = _definition(cpp, signature)
        keys.append(_rhs(body, pattern, label))

    words = []
    dwords = []
    for signature, label in (
        ("void Texture::sub_6212b0(Buffer *buf, Vert *verts, int *zvals, int count,", "6212b0"),
        ("void Texture::sub_622810(Buffer *buf, Vert *verts, int *zvals, int count,", "622810"),
    ):
        _, _, body = _definition(cpp, signature)
        words.append(_rhs(body,
            r"unsigned\s+short\s+light_step\s*=\s*(.*?);", f"{label} light_step"))
        dwords.append(_rhs(body,
            r"\bu_acc\s*=\s*u_acc\s*\+\s*(.*?);", f"{label} Scratch528 add"))
    return tuple(keys), tuple(words), tuple(dwords)


def _scalar_harness(key_exprs, word_exprs, dword_exprs):
    assert len(key_exprs) == 3 and len(word_exprs) == len(dword_exprs) == 2
    key_functions = "\n".join(
        f"static unsigned int key_value_{i}(int TextureRasterLightThreshold) "
        f"{{ return ({expr}); }}"
        for i, expr in enumerate(key_exprs))
    word_functions = "\n".join(
        f"static unsigned short word_value_{i}(unsigned short low_word) "
        f"{{ const LightPack TextureRasterLightPackA = {{low_word, 0}}; "
        f"return ({expr}); }}"
        for i, expr in enumerate(word_exprs))
    dword_functions = "\n".join(
        f"static unsigned int dword_value_{i}(int TextureRasterScratch528) "
        f"{{ return ({expr}); }}"
        for i, expr in enumerate(dword_exprs))
    return textwrap.dedent(r"""\
        #include <cassert>
        #include <cstdint>
        #include <climits>
        struct LightPack { unsigned short low_word, high_word; };
        @KEY_FUNCTIONS@
        @WORD_FUNCTIONS@
        @DWORD_FUNCTIONS@
        int main() {
            static_assert(sizeof(unsigned int) == 4, "32-bit unsigned int required");

            const int keys[] = {INT_MIN, -65537, -256, -1, 0, 1, 255, 256,
                                0x12345678, INT_MAX};
            for (int value : keys)
                assert(key_value_0(value) == (static_cast<uint32_t>(value) & 0xffu));
            for (int value : keys)
                assert(key_value_1(value) == (static_cast<uint32_t>(value) & 0xffu));
            for (int value : keys)
                assert(key_value_2(value) == (static_cast<uint32_t>(value) & 0xffu));

            for (unsigned int value = 0; value <= 0xffffu; ++value)
                assert(word_value_0(static_cast<unsigned short>(value)) == value);
            for (unsigned int value = 0; value <= 0xffffu; ++value)
                assert(word_value_1(static_cast<unsigned short>(value)) == value);

            const int dword_inputs[] = {INT_MIN, -1, 0, 1, INT_MAX};
            for (int value : dword_inputs)
                assert(dword_value_0(value) == static_cast<uint32_t>(value));
            for (int value : dword_inputs)
                assert(dword_value_1(value) == static_cast<uint32_t>(value));
            return 0;
        }
    """).replace("@KEY_FUNCTIONS@", key_functions).replace(
        "@WORD_FUNCTIONS@", word_functions).replace(
        "@DWORD_FUNCTIONS@", dword_functions)


EXPECTED_KEY_EXPR = "static_cast<unsigned char>(TextureRasterLightThreshold)"
EXPECTED_WORD_EXPR = "TextureRasterLightPackA.low_word"
EXPECTED_DWORD_EXPR = "static_cast<unsigned int>(TextureRasterScratch528)"


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_typed_scalar_reads_preserve_declared_value_domains(tmp_path, compiler):
    cpp = (ROOT / "src/texture.cpp").read_text(encoding="utf-8")
    key_exprs, word_exprs, dword_exprs = _actual_scalar_expressions(cpp)
    normalize = lambda expression: re.sub(r"\s+", "", expression)
    assert tuple(map(normalize, key_exprs)) == (normalize(EXPECTED_KEY_EXPR),) * 3
    assert tuple(map(normalize, word_exprs)) == (normalize(EXPECTED_WORD_EXPR),) * 2
    assert tuple(map(normalize, dword_exprs)) == (normalize(EXPECTED_DWORD_EXPR),) * 2
    result = _compile_and_run(tmp_path, compiler,
        _scalar_harness(key_exprs, word_exprs, dword_exprs),
        "scalar_reads")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", (
    "wrong_key_byte", "wrong_key_signedness", "word_truncation",
    "dword_truncation"))
def test_scalar_read_damage_controls_fail_at_runtime(tmp_path, compiler, mutation):
    cpp = (ROOT / "src/texture.cpp").read_text(encoding="utf-8")
    keys, words, dwords = _actual_scalar_expressions(cpp)
    keys, words, dwords = list(keys), list(words), list(dwords)
    if mutation == "wrong_key_byte":
        keys = [expr.replace("TextureRasterLightThreshold",
            "(static_cast<uint32_t>(TextureRasterLightThreshold) >> 8)", 1)
            for expr in keys]
    elif mutation == "wrong_key_signedness":
        keys = [expr.replace("static_cast<unsigned char>",
            "static_cast<int8_t>", 1) for expr in keys]
    elif mutation == "word_truncation":
        words = [f"static_cast<unsigned char>({expr})" for expr in words]
    else:
        dwords = [f"static_cast<unsigned short>({expr})" for expr in dwords]
    result = _compile_and_run(tmp_path, compiler,
        _scalar_harness(keys, words, dwords), f"scalar_mutant_{mutation}")
    assert result.returncode != 0, f"{mutation} mutant unexpectedly passed"
