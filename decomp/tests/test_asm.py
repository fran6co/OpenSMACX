"""Tests for decomp.asm.

The hermetic half runs anywhere: fact reading, span extraction, the COFF
reader against a hand-built object, and the clear refusals. The other half
needs the recovery environment (pinned exe, wine, VC6, a configured build)
and skips itself on a box without it; the end-to-end agreement check is a
smoke test, with the ratchet in tools/ keeping the strict machinery.
"""

import shutil
import struct
from pathlib import Path

import pytest

from decomp import read_file
from decomp import asm
from decomp.asm import (build_command, compare_asm, compiled_asm,
                        original_asm)

# The environment's facts live HERE, not in the package: where this checkout
# is, and where the pinned image and the build database are on this box. The
# compiler itself is the build's own business - the compile command names it.
# `reader` used to export the repository root and this file imported it, which
# said the exact opposite of the sentence above.
REPO_ROOT = Path(__file__).resolve().parents[2]
EXE = REPO_ROOT / ".opensmacx" / "game" / "terranx_original.exe"
COMPILE_COMMANDS = REPO_ROOT / "build" / "compile_commands.json"
# THE FLAG SETS BELONG TO THE CALLER, and these tests are one. `/O2` implies
# `/Oy`, which omits the frame pointer, and the shipped image is mixed - so
# which invocation reproduces a given function is a property of the function,
# and asking only the first of these answers about half the image. They are
# not a build setting either: `/Oy-` appears nowhere in this tree's CMake
# configuration, which is a Debug database of `/Od /Ob0`.
FLAGS = "/c /O2 /Gy /GR- /Oy- /GX"
FRAMELESS_FLAGS = "/c /O2 /Gy /GR- /GX"
FLAG_SETS = (FLAGS, FRAMELESS_FLAGS,
             "/c /O1 /Gy /GR- /Oy- /GX", "/c /O1 /Gy /GR- /GX")

HAVE_EXE = EXE.is_file()
HAVE_BUILD = COMPILE_COMMANDS.is_file() and shutil.which("wine") is not None

MARKED = """// ORIGINAL: 0x00401000 ?f@C@@QAEXXZ 0x00401000-0x00401008
// size      8 bytes
void C::f() {
}
"""

NO_FACTS = """// ORIGINAL: 0x00401000
void f() {
}
"""


def record_in(tmp_path, text):
    """A record anchored to a real file - asm functions read record.path."""
    path = tmp_path / "x.cpp"
    path.write_text(text)
    return read_file(path)[0]


def test_record_carries_name_and_spans(tmp_path):
    record = record_in(tmp_path, MARKED)
    assert record.name == "?f@C@@QAEXXZ"
    assert record.image_spans == ((0x00401000, 0x00401008),)


def test_span_refused_without_the_fact(tmp_path):
    with pytest.raises(ValueError, match="no spans"):
        original_asm(record_in(tmp_path, NO_FACTS),
                     Path("/nonexistent/image.exe"))


def test_original_asm_refused_without_the_exe(tmp_path):
    with pytest.raises(ValueError, match="no executable"):
        original_asm(record_in(tmp_path, MARKED),
                     Path("/nonexistent/image.exe"))


def test_compiled_asm_refused_without_the_name_fact(tmp_path):
    with pytest.raises(ValueError, match="no name"):
        compiled_asm(record_in(tmp_path, NO_FACTS), ["cl"], FLAGS)


# --------------------------------------------------------------- COFF reader


def tiny_coff(code: bytes, symbol: str) -> bytes:
    """A minimal one-section COFF object carrying one external .text symbol."""
    raw_ptr = 20 + 40
    sym_ptr = raw_ptr + len(code)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, sym_ptr, 1, 0, 0)
    section = struct.pack("<8sIIIIIIHHI", b".text\x00\x00\x00", 0, 0,
                          len(code), raw_ptr, 0, 0, 0, 0, 0x60000020)
    sym_bytes = symbol.encode() + b"\x00"
    entry = (b"\x00\x00\x00\x00" + struct.pack("<I", 4)
             + struct.pack("<IhHBB", 0, 1, 0, 2, 0))
    strtab = struct.pack("<I", 4 + len(sym_bytes)) + sym_bytes
    return header + section + code + entry + strtab


def test_coff_function_extracts_and_strips_padding():
    code = b"\x55\x8B\xEC\xC3"
    obj = tiny_coff(code + b"\xCC\xCC\x90\x90", "?f@@YAXXZ")
    assert asm._coff_function_masked(obj, "?f@@YAXXZ")[0] == code


def test_coff_function_refuses_an_absent_symbol():
    obj = tiny_coff(b"\xC3", "?f@@YAXXZ")
    with pytest.raises(ValueError, match="not found"):
        asm._coff_function_masked(obj, "?g@@YAXXZ")


# ------------------------------------------------------- listing comparison

# push ebp; mov ebp, esp; mov eax, 1; ret - four instructions, nine bytes.
# Built from BYTES rather than from hand-written listing text: the text is
# what capstone renders, so asserting against text I typed asserts against
# my own transcription.
BODY = bytes.fromhex("55" "8BEC" "B801000000" "C3")


def listing(code: bytes, mask: frozenset[int] = frozenset(),
            base: int = 0x00401000) -> asm.Listing:
    return asm.Listing(code=code, base=base, mask=mask)


def test_lines_are_derived_from_the_bytes():
    """A listing renders; it does not carry a rendering."""
    lines = listing(BODY).lines
    assert len(lines) == 4
    assert lines[0].startswith("0x00401000")
    assert lines[0].endswith("push ebp")


def test_the_rendering_is_cached_and_the_bytes_are_not_writable():
    """Derived state is only safe while the thing it derives from is fixed."""
    one = listing(BODY)
    assert one.lines is one.lines            # rendered once, not per access
    with pytest.raises(Exception):           # FrozenInstanceError
        one.code = b"\x90"                   # type: ignore[misc]


def test_a_listing_is_its_bytes_not_its_text():
    """Two listings of the same body at the same address are the same
    listing - which a `list` subclass carrying rendered lines could not
    say, because it had two representations to disagree about."""
    assert listing(BODY) == listing(BODY)
    assert listing(BODY) != listing(BODY, frozenset({0}))
    assert len({listing(BODY), listing(BODY)}) == 1


def test_compare_byte_exact():
    result = compare_asm(listing(BODY), listing(BODY))
    assert result.verdict == "BYTE_EXACT"
    assert result.original_instructions == result.compiled_instructions == 4
    assert result.matching_instructions == 4
    assert result.mnemonic_similarity == 1.0
    assert result.first_divergence is None
    assert result.compared_bytes == len(BODY)
    assert result.masked_bytes == 0


def test_compare_shape_exact_names_the_wrong_constant():
    """Same instructions, same registers, one wrong value - the miss that
    names a field offset or a loop bound rather than a wrong idea."""
    other = bytes.fromhex("55" "8BEC" "B802000000" "C3")   # mov eax, 2
    result = compare_asm(listing(BODY), listing(other))
    assert result.verdict == "SHAPE_EXACT"
    assert result.matching_instructions == 3
    assert result.mnemonic_similarity == 1.0
    assert result.first_divergence == 2
    assert result.differing_constants == ((2, "mov", "0x1", "0x2"),)


def test_compare_mnemonic_only_is_the_weaker_statement():
    """Same instruction sequence, a different REGISTER. The constants are
    identical, so this is not SHAPE_EXACT - the allocation is wrong."""
    other = bytes.fromhex("55" "8BEC" "B901000000" "C3")   # mov ecx, 1
    result = compare_asm(listing(BODY), listing(other))
    assert result.verdict == "MNEMONIC_ONLY"
    assert result.mnemonic_similarity == 1.0
    assert result.differing_constants == ()


def test_compare_mismatch_reports_the_divergence():
    # xor eax, eax where the original moves esp into ebp
    other = bytes.fromhex("55" "33C0" "B801000000" "C3")
    result = compare_asm(listing(BODY), listing(other))
    assert result.verdict == "MISMATCH"
    assert result.first_divergence == 1
    assert result.mnemonic_similarity < 1.0
    assert any("xor" in line for line in result.context[1])


def test_compare_ret_keeps_its_pop_count():
    # `ret` and `ret 4` are different instructions; the sequence must say so.
    result = compare_asm(listing(b"\xC3"), listing(bytes.fromhex("C20400")))
    assert result.verdict == "MISMATCH"


def test_compare_length_difference():
    result = compare_asm(listing(BODY), listing(BODY[:-1]))
    assert result.verdict == "MISMATCH"
    assert result.first_divergence == 3
    assert result.original_instructions == 4
    assert result.compiled_instructions == 3


# ------------------------------------------------------- the real thing


@pytest.mark.skipif(not HAVE_EXE, reason="no pinned executable")
def test_original_asm_on_a_known_record():
    records = read_file(REPO_ROOT / "src" / "buffer.cpp")
    record = next(r for r in records if r.address == 0x005D7210)
    lines = original_asm(record, EXE).lines
    assert lines
    assert lines[0].startswith("0x005D7210")


@pytest.mark.skipif(not (HAVE_EXE and HAVE_BUILD),
                    reason="no build environment")
def test_byte_exact_body_compiles_to_the_same_shape():
    """Smoke: a BYTE_EXACT claim reproduces byte for byte. The strict
    multi-flag machinery stays with the ratchet in tools/byte_match.py;
    this is the single-flag comparison the package offers."""
    records = read_file(REPO_ROOT / "src" / "caviar.cpp")
    record = next(r for r in records if r.address == 0x00616BC0)
    assert record.byte_exact
    original = original_asm(record, EXE)
    command = build_command(COMPILE_COMMANDS, record.path)
    compiled = compiled_asm(record, command, FLAGS)
    result = compare_asm(original, compiled)
    assert result.verdict == "BYTE_EXACT", \
        f"{result.verdict}: {result.matching_instructions}/{result.original_instructions} " \
        f"lines match, similarity {result.mnemonic_similarity:.2f}"
    assert result.matching_instructions == result.original_instructions


# ------------------------------------------- naming the subject in an object


def coff(functions, relocations=()):
    """A one-section COFF object carrying several external .text symbols.

    `functions` is [(symbol, code)] laid out in order; `relocations` is
    [(offset, type)] against the section. Enough to exercise subject
    selection and the relocation mask without a compiler.
    """
    blob, placed = b"", []
    for symbol, code in functions:
        placed.append((symbol, len(blob)))
        blob += code
    n_syms = len(placed)
    raw_ptr = 20 + 40
    reloc_ptr = raw_ptr + len(blob) if relocations else 0
    sym_ptr = raw_ptr + len(blob) + len(relocations) * 10
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, sym_ptr, n_syms, 0, 0)
    section = struct.pack("<8sIIIIIIHHI", b".text\x00\x00\x00", 0, 0,
                          len(blob), raw_ptr, reloc_ptr, 0,
                          len(relocations), 0, 0x60000020)
    relocs = b"".join(struct.pack("<IIH", at, 0, kind)
                      for at, kind in relocations)
    entries, strtab, offset = b"", b"", 4
    for symbol, value in placed:
        entries += (b"\x00\x00\x00\x00" + struct.pack("<I", offset)
                    + struct.pack("<IhHBB", value, 1, 0, 2, 0))
        strtab += symbol.encode() + b"\x00"
        offset += len(symbol) + 1
    return (header + section + blob + relocs + entries
            + struct.pack("<I", 4 + len(strtab)) + strtab)


def test_a_name_the_compiler_does_not_emit_is_an_error():
    """CL writes `?set_font@Buffer@@QAEHPAVFont@@000@Z` where the catalogue
    writes the type out four times. The lookup does NOT go hunting for a
    near-miss: the annotation is supposed to record what is emitted, and
    saying so is more useful than guessing which neighbour was meant."""
    obj = coff([("?set_font@Buffer@@QAEHPAVFont@@000@Z", b"\xC3")])
    with pytest.raises(ValueError, match="no `symbol` fact"):
        asm._coff_function_masked(
            obj, "?set_font@Buffer@@QAEHPAVFont@@PAVFont@@PAVFont@@PAVFont@@@Z")


def test_the_underscore_form_still_counts_as_the_same_name():
    """`extern "C"` decorates with a leading underscore and that is the same
    symbol, not a guess about a different one."""
    obj = coff([("_f", b"\xC3")])
    code, _mask = asm._coff_function_masked(obj, "f")
    assert code == b"\xC3"


SHIM = """// ORIGINAL: 0x00401000 ?on_scrolling@BaseWin@@QAEXHH@Z 0x00401000-0x00401004
// symbol    @base_win_on_scrolling_redirect@16
void __fastcall base_win_on_scrolling_redirect(void *self, void *, int a) {
}
"""


def test_the_recorded_symbol_names_the_subject(tmp_path):
    """A redirect shim carries a symbol of its own, and the annotation says
    so - `name` stays what the IMAGE calls the piece."""
    record = record_in(tmp_path, SHIM)
    assert record.name == "?on_scrolling@BaseWin@@QAEXHH@Z"
    assert record.symbol == "@base_win_on_scrolling_redirect@16"
    obj = coff([("@base_win_on_scrolling_redirect@16", b"\xC3")])
    code, _mask = asm._coff_function_masked(obj, record.name, record.symbol)
    assert code == b"\xC3"


def test_a_stale_symbol_fact_is_an_error_not_a_guess(tmp_path):
    """Recorded and absent means the fact is wrong; the lookup must not
    quietly fall through to a name that happens to be nearby."""
    record = record_in(tmp_path, SHIM)
    obj = coff([("?on_scrolling@BaseWin@@QAEXHH@Z", b"\xC3")])
    with pytest.raises(ValueError, match="stale"):
        asm._coff_function_masked(obj, record.name, record.symbol)


def test_region_identifiers_reads_the_definition_head(tmp_path):
    """The migration aid that WRITES the fact above, kept out of the
    measurement path on purpose."""
    record = record_in(tmp_path, SHIM)
    from decomp.reader import region_identifiers
    assert "base_win_on_scrolling_redirect" in region_identifiers(record)


def test_an_unnamed_subject_says_the_fact_is_missing(tmp_path):
    record = record_in(tmp_path, """// ORIGINAL: 0x00401000 ?f@C@@QAEXXZ 0x00401000-0x00401004
void __fastcall some_other_spelling(void *self) {
}
""")
    obj = coff([("@some_other_spelling@4", b"\xC3")])
    with pytest.raises(ValueError, match="no `symbol` fact"):
        asm._coff_function_masked(obj, record.name, record.symbol)


def test_object_relocations_become_a_mask():
    # `push imm32` with a DIR32 relocation on the immediate at offset 1.
    obj = coff([("?f@@YAXXZ", b"\x68\x00\x00\x00\x00\xC3")],
               relocations=((1, 0x0006),))
    code, mask = asm._coff_function_masked(obj, "?f@@YAXXZ")
    assert code == b"\x68\x00\x00\x00\x00\xC3"
    assert mask == frozenset({1, 2, 3, 4})


# --------------------------------------------------- discounting relocations


def test_relocated_bytes_do_not_count_as_a_divergence():
    """The image's linked address against the object's zeros is not a
    difference in the source, and this is the pair that used to read
    MNEMONIC_ONLY."""
    image = b"\x68\x74\x81\x9B\x00\xC3"          # push 0x009B8174
    obj = b"\x68\x00\x00\x00\x00\xC3"            # push 0 + a relocation
    mask = frozenset({1, 2, 3, 4})
    left = listing(image)
    right = listing(obj, mask)
    # Without the mask the same pair is only SHAPE_EXACT: the four relocated
    # bytes read as a wrong CONSTANT, which is not a difference in the source.
    assert compare_asm(left, listing(obj)).verdict == "SHAPE_EXACT"
    result = compare_asm(left, right)
    assert result.verdict == "BYTE_EXACT"
    assert result.masked_bytes == 4
    assert result.compared_bytes == 2


def test_a_real_difference_survives_the_mask():
    """Masking discounts relocated bytes, not the instruction around them."""
    image = b"\x68\x74\x81\x9B\x00\xC3"
    obj = b"\xB8\x00\x00\x00\x00\xC3"            # mov eax, imm32, not push
    mask = frozenset({1, 2, 3, 4})
    assert compare_asm(listing(image), listing(obj, mask)).verdict == "MISMATCH"


def test_padding_is_stripped_from_both_sides():
    """A catalogued span may include COMDAT padding; the object's is already
    stripped, so the image's must be too or the lengths never agree."""
    body = b"\x55\x8B\xEC\xC3"
    obj = coff([("?f@@YAXXZ", body + b"\xCC\xCC")])
    code, _mask = asm._coff_function_masked(obj, "?f@@YAXXZ")
    assert code == body


def test_a_mask_never_reshapes_the_body_it_describes():
    """`0x004E0F80` is seven instructions in the image, and an early version
    reported nine: it zeroed the image at the OBJECT's relocation offsets
    and re-disassembled, splitting instructions that were never there.
    Nothing is re-disassembled now - `_agree` compares an instruction's own
    bytes - so the count a reader is given is always the body's own.
    """
    image = bytes.fromhex("E89BAF0C00" "E856490E00" "E8A14A0E00"
                          "6A01" "E8FAA1F8FF" "59" "C3")
    obj = bytes.fromhex("56" "8BF1" "E800000000" "E800000000")
    left = listing(image, base=0x004E0F80)
    right = listing(obj, frozenset({4, 5, 6, 7, 9, 10, 11, 12}), 0x004E0F80)
    assert len(left.lines) == 7
    result = compare_asm(left, right)
    assert result.verdict == "MISMATCH"
    assert result.original_instructions == 7, \
        "the mask reshaped the image's own listing"


def test_byte_equality_decides_not_the_rendered_text():
    """A pair that agrees on every unmasked byte is BYTE_EXACT even though
    the rendered lines differ - the verdict reads the bytes."""
    image = bytes.fromhex("A1 74819B00 C3".replace(" ", ""))
    obj = bytes.fromhex("A1 00000000 C3".replace(" ", ""))
    mask = frozenset({1, 2, 3, 4})
    left, right = listing(image), listing(obj, mask)
    assert left.lines != right.lines          # the text differs
    assert compare_asm(left, right).verdict == "BYTE_EXACT"


def test_differing_lengths_are_never_byte_exact():
    a = b"\x90\xC3"
    b = b"\x90\x90\xC3"
    assert compare_asm(listing(a), listing(b)).verdict == "MISMATCH"


# ------------------------------------------------- results, not exceptions


def test_a_body_that_will_not_build_is_a_tier(tmp_path, monkeypatch):
    """NO_COMPILE is what a recovery pass most wants to act on, so it comes
    back as a verdict carrying the compiler's own words - not a traceback."""
    record = record_in(tmp_path, MARKED)
    def refuse(*args, **kwargs):
        raise asm.CompileFailed("x.cpp(3) : error C2065: 'g_x' : undeclared")
    monkeypatch.setattr(asm, "compiled_asm", refuse)
    monkeypatch.setattr(asm, "original_asm",
                        lambda *a, **k: listing(b"\xC3"))
    monkeypatch.setattr(asm, "span_refusal", lambda *a, **k: None)
    result = asm.compare_record(record, "exe", ["cl"], ("/c /O2",))
    assert result.verdict == "NO_COMPILE"
    assert "C2065" in result.diagnostic


def test_a_lying_annotation_is_still_an_error(tmp_path, monkeypatch):
    """A stale `symbol` fact is a defect in the annotation, not a fact about
    the body; reporting it as NO_COMPILE would blame the wrong thing."""
    record = record_in(tmp_path, MARKED)
    def stale(*args, **kwargs):
        raise ValueError("?f@C@@QAEXXZ: ... the fact is stale")
    monkeypatch.setattr(asm, "compiled_asm", stale)
    monkeypatch.setattr(asm, "original_asm",
                        lambda *a, **k: listing(b"\xC3"))
    monkeypatch.setattr(asm, "span_refusal", lambda *a, **k: None)
    with pytest.raises(ValueError, match="stale"):
        asm.compare_record(record, "exe", ["cl"], ("/c /O2",))


def test_compare_source_compiles_the_candidate_not_the_tree(tmp_path,
                                                            monkeypatch):
    """The candidate is the parameter, and that is what makes an
    edit-measure loop possible: the question is posed without editing the
    tree, so a failed experiment is not a dirty checkout."""
    record = record_in(tmp_path, MARKED)
    candidate = tmp_path / "candidate.cpp"
    candidate.write_text("void f() {}\n")
    compiled = []
    monkeypatch.setattr(asm, "compile_unit",
                        lambda source, *a, **k: compiled.append(source) or b"")
    monkeypatch.setattr(asm, "subject_asm",
                        lambda *a, **k: listing(b"\xC3"))
    monkeypatch.setattr(asm, "original_asm",
                        lambda *a, **k: listing(b"\xC3"))
    monkeypatch.setattr(asm, "span_refusal", lambda *a, **k: None)

    asm.compare_source(record, "exe", candidate, ["cl"], ("/c /O2",))
    asm.compare_record(record, "exe", ["cl"], ("/c /O2",))
    assert compiled == [candidate, record.path]


def test_the_diagnostic_prefers_what_cl_called_an_error():
    assert "C2065" in asm._diagnostic("note\nx.cpp : error C2065: bad\n", "")
    assert asm._diagnostic("", "") == "the compile produced no object"
    assert asm._diagnostic("only this\n", "") == "only this"


SELFMOD = """// ORIGINAL: 0x00666F68 ?f@@YAXXZ 0x00666F68-0x00666F70
void f() {
}
"""


@pytest.mark.skipif(not HAVE_EXE, reason="no pinned executable")
def test_the_image_says_which_of_its_code_can_rewrite_itself():
    """This was `SELFMOD_RANGE = (0x00664000, 0x00669000)` in the source -
    the bounds of one section of one image, transcribed where nothing could
    keep them true. The image states the PROPERTY in its own section
    characteristics: exactly one section here is executable and writable."""
    assert asm.rewrites_itself(EXE, 0x00666F68)
    assert not asm.rewrites_itself(EXE, 0x005DAC70)     # .text: exec, not write
    assert not asm.rewrites_itself(EXE, 0x00682100)     # .data: write, not exec
    assert not asm.rewrites_itself(EXE, 0x00000010)     # in no section at all
    writable_code = [n for n, _lo, _hi, c in asm._sections(EXE)
                     if c & asm.IMAGE_SCN_MEM_EXECUTE
                     and c & asm.IMAGE_SCN_MEM_WRITE]
    assert writable_code == ["_SELFMOD"], writable_code
    assert asm.section_of(EXE, 0x005DAC70) == ".text"


@pytest.mark.skipif(not HAVE_EXE, reason="no pinned executable")
def test_a_self_modifying_span_is_refused_before_a_compiler(tmp_path):
    record = record_in(tmp_path, SELFMOD)
    assert asm.span_refusal(record, EXE) == "REFUSED"


def test_a_folded_tail_belongs_to_no_single_body(tmp_path):
    a = record_in(tmp_path, MARKED)
    shared = asm.shared_spans([a, a.__class__(**{**a.__dict__,
                                                "address": 0x00402000})])
    assert a.image_spans[0] in shared
    assert asm.span_refusal(a, EXE, shared) == "SHARED_TAIL"


def test_an_ordinary_span_is_not_refused(tmp_path):
    assert asm.span_refusal(record_in(tmp_path, MARKED), EXE) is None


# --------------------------------------------- a command, not a permission


@pytest.mark.skipif(not HAVE_BUILD, reason="no build database")
def test_build_command_reads_the_builds_own_invocation():
    command = build_command(COMPILE_COMMANDS, REPO_ROOT / "src" / "buffer.cpp")
    assert command[0].endswith("vc6-cl")
    assert any(w.startswith("/I") for w in command)
    # NOT the optimisation flags: those are a property of the function being
    # matched, and belong to whoever is asking.
    assert not any(w.startswith(("/O", "/Gy", "/Oy")) for w in command)


@pytest.mark.skipif(not HAVE_BUILD, reason="no build database")
def test_a_file_the_build_does_not_name_is_refused_not_guessed():
    """3,315 records live in files CMake has no reason to build, and most
    are FILE-mode - already a translation unit, wanting only an invocation.
    Which invocation is a judgement about include paths that the caller
    holds, so this refuses and says what to pass instead of picking one."""
    with pytest.raises(ValueError, match="pass the command of a file that is"):
        build_command(COMPILE_COMMANDS,
                      REPO_ROOT / "src" / "recovered" / "units" / "x.cpp")


@pytest.mark.skipif(not HAVE_BUILD, reason="no build database")
def test_a_borrowed_command_compiles_a_file_the_build_never_names():
    """The borrow is the caller's to make, and it works: the same command
    that builds `buffer.cpp` builds a unit CMake never sees."""
    from decomp import read
    from decomp.asm import compare_record
    units = REPO_ROOT / "src" / "recovered" / "units"
    if not units.is_dir():
        pytest.skip("no proved-unit store in this checkout")
    record = next((r for r in read(units) if r.symbol or r.name), None)
    if record is None:
        pytest.skip("no annotated unit to try")
    command = build_command(COMPILE_COMMANDS, REPO_ROOT / "src" / "buffer.cpp")
    verdict = compare_record(record, EXE, command, FLAGS)
    assert verdict.verdict in tuple(str(t) for t in __import__(
        "decomp").Tier)


def test_one_instruction_of_another_size_does_not_disable_the_mask():
    """The bug this replaced. Discounting used to need both bodies to be
    the same LENGTH, so a single instruction differing by a byte withdrew
    the mask from everything - measured on `_WinMain@16`, 459 bytes against
    458, reporting 35 of 141 instructions agreeing where 138 do.

    Here: `push <global>` twice, then a `mov` that is 3 bytes on one side
    and 2 on the other. The pushes are relocated on both sides and must
    still agree; only the `mov` may differ.
    """
    image = bytes.fromhex("68 74819B00  68 78819B00  8B4510"
                          .replace(" ", ""))
    obj = bytes.fromhex("68 00000000  68 00000000  8BC7"
                        .replace(" ", ""))
    relocated = frozenset({1, 2, 3, 4, 6, 7, 8, 9})
    left = asm.Listing(code=image, base=0x401000, mask=relocated)
    right = asm.Listing(code=obj, base=0x401000, mask=relocated)
    assert len(left.code) != len(right.code)          # 13 against 12
    result = compare_asm(left, right)
    assert result.original_instructions == 3
    assert result.matching_instructions == 2, \
        "the two relocated pushes agree; only the mov differs"
    assert result.first_divergence == 2
    # Same instruction sequence, one operand a register where the other is
    # memory - which is what `_WinMain@16` scores, for the same reason.
    assert result.verdict == "MNEMONIC_ONLY"
