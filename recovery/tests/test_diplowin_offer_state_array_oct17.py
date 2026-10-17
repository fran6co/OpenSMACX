"""Host controls for DiploWin's evidence-backed two-lane offer state."""
from pathlib import Path
import json
import shutil
import subprocess

import pytest

from test_diplowin_exec_behavior import extract_function


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("diplowin_exec_host_control.cpp")
BASE_REVISION = "4c45ca091e3d68724a0b8cedb4f2059c2a331649"


def _live_exec_functions(source: str, general: str) -> str:
    methods = "\n\n".join(
        extract_function(source, signature)
        for signature in (
            "static RECT inset_rect(RECT value, int left, int top, int right, int bottom)",
            "void __cdecl DiploWin::string_callback(int faction_id, int unused)",
            "void DiploWin::exec(int faction_id, int faction_id_with)",
        )
    )
    swap = extract_function(general, "void __cdecl swap(int *var1, int *var2)")
    transcript = extract_function(
        source,
        'extern "C" void __cdecl sub_441100(int faction_id, int faction_id_with)',
    )
    return swap + "\n\n" + methods + "\n\n" + transcript


def _build_and_run(tmp_path: Path, source: str, damage=None) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    general = (ROOT / "src/general.cpp").read_text()
    functions = _live_exec_functions(source, general)
    if damage is not None:
        old, new = damage
        assert functions.count(old) == 1, f"mutation anchor changed: {old}"
        functions = functions.replace(old, new, 1)
    generated = FIXTURE.read_text()
    marker = "/* DIPLOWIN_EXEC_FUNCTIONS */"
    transcript_marker = "/* DIPLOWIN_TRANSCRIPT_FUNCTION */"
    assert generated.count(marker) == generated.count(transcript_marker) == 1
    # The transcript body is a separate marker in the fixture so it remains
    # easy to retain independently while the methods are assembled above.
    transcript = extract_function(
        source,
        'extern "C" void __cdecl sub_441100(int faction_id, int faction_id_with)',
    )
    methods = functions.replace(
        transcript, "", 1
    ).replace(
        extract_function(general, "void __cdecl swap(int *var1, int *var2)"), "", 1
    )
    swap = extract_function(general, "void __cdecl swap(int *var1, int *var2)")
    generated = generated.replace(marker, swap + "\n\n" + methods, 1)
    generated = generated.replace(transcript_marker, transcript, 1)
    cpp, exe = tmp_path / "offer-state.cpp", tmp_path / "offer-state"
    cpp.write_text(generated)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-I", str(FIXTURE.parent),
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20)


def test_offer_state_two_lanes_roundtrip_hostile_signed_pbem_words(tmp_path):
    source = (ROOT / "src/diplowin.cpp").read_text()
    result = _build_and_run(tmp_path, source)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("damage", [
    ("offer_state_[side] = cell.state_[side];",
     "offer_state_[1 - side] = cell.state_[side];"),
    ("cell.state_[side] = offer_state_[side];",
     "cell.state_[side] = offer_state_[1 - side];"),
])
def test_offer_state_lane_order_mutants_fail_pbem_roundtrip(tmp_path, damage):
    source = (ROOT / "src/diplowin.cpp").read_text()
    result = _build_and_run(tmp_path, source, damage)
    assert result.returncode != 0, "PBEM lane-order mutation survived the host assertions"


def test_offer_state_migration_retains_pinned_exec_identity_and_history():
    source_path = ROOT / "src/diplowin.cpp"
    source = source_path.read_text()
    header = (ROOT / "src/diplowin.h").read_text()
    task = json.loads((ROOT / "recovery/tasks/diplowin-offer-state-array-oct17.json").read_text())
    assert task["base_revision"] == BASE_REVISION
    original = "// ORIGINAL: 0x00443F10 ?exec@DiploWin@@QAEXHH@Z 0x00443F10-0x00444D8B"
    assert original in source
    assert "// TRIED: the ~500-instruction layout-RECT tree" in source
    assert "// RECOVERY_MEMBER: v1 DiploWin offer_state_ 0xA24 0x00443F10" in header
    assert "int32_t offer_state_[2];" in header
    # The migration changes the representation, not the source-owned identity,
    # historical experiment, or the existing measurement span.
    from match_recovery.source import Mode, read_file
    record, = (row for row in read_file(source_path) if row.address == 0x00443F10)
    assert record.name == "?exec@DiploWin@@QAEXHH@Z"
    assert record.image_spans == ((0x00443F10, 0x00444D8B),)
    assert record.mode is Mode.BODY
