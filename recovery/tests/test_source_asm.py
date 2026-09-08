"""Real-image and project-build controls; never packaged binary fixtures."""
from pathlib import Path
import os
import shutil
import pytest
from match_recovery.source import asm, read_file, Tier
from recovery.environment.recipe import build_command, flag_dependent
from recovery.environment.compile import compile_object
from match_recovery.source.asm import compare_asm, compiled_asm, original_asm
from test_asm import record_in, MARKED, SELFMOD

REPO_ROOT = Path(__file__).resolve().parents[2]
EXE = Path(os.environ.get("OPENSMACX_IMAGE", str(REPO_ROOT / ".opensmacx/game/terranx_original.exe")))
COMPILE_COMMANDS = REPO_ROOT / "build/compile_commands.json"
FLAGS = "/c /O2 /Gy /GR- /Oy- /GX"
HAVE_EXE = EXE.is_file()
HAVE_BUILD = COMPILE_COMMANDS.is_file() and shutil.which("wine") is not None

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
    multi-flag machinery belongs to the package's source_measure engine;
    this is its single-flag historical comparison, not a reference/structure proof."""
    records = read_file(REPO_ROOT / "src" / "caviar.cpp")
    record = next(r for r in records if r.address == 0x00616BC0)
    assert record.byte_exact
    original = original_asm(record, EXE)
    command = build_command(COMPILE_COMMANDS, record.path)
    compiled = compiled_asm(record, command, FLAGS, compiler=compile_object)
    result = compare_asm(original, compiled)
    assert result.verdict == "BYTE_EXACT", \
        f"{result.verdict}: {result.matching_instructions}/{result.original_instructions} " \
        f"lines match, similarity {result.mnemonic_similarity:.2f}"
    assert result.matching_instructions == result.original_instructions



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
    from match_recovery.source import read
    from match_recovery.source.asm import compare_record
    units = REPO_ROOT / "src" / "recovered" / "units"
    if not units.is_dir():
        pytest.skip("no proved-unit store in this checkout")
    record = next((r for r in read(units) if r.symbol or r.name), None)
    if record is None:
        pytest.skip("no annotated unit to try")
    command = build_command(COMPILE_COMMANDS, REPO_ROOT / "src" / "buffer.cpp")
    verdict = compare_record(record, EXE, command, FLAGS, compiler=compile_object, retry_compile=flag_dependent)
    assert verdict.verdict in {str(tier) for tier in Tier}
