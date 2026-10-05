"""Host controls compiling only the mapped src/veh.cpp owner."""
import os
from pathlib import Path
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import Mode, State, read_file
from match_recovery.source_inventory import image_pin


HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
OWNER = ROOT / "src/veh.cpp"
FIXTURE = HERE / "consider_designs_host_fixture.cpp"
SUPPORT_MARKER = "// consider_designs owner support"
SIGNATURE = "void __cdecl consider_designs(int faction_id)"
EXPECTED_PICKS = [
    "1,0,1", "1,0,1", "1,0,3", "1,0,2", "1,0,3", "1,0,2",
    "1,0,1", "1,1,0", "1,1,0", "1,2,0", "1,2,-2", "1,2,-1",
]


def _definition(source, signature):
    # Use the recovery test suite's established brace/string/comment-aware
    # extractor; source identity itself is checked through match_recovery.
    import sys
    sys.path.insert(0, str(ROOT / "recovery/tests"))
    from test_game_logic_seed_batch_behavior import _definition as extract
    return extract(source, signature)


def _actual_owner_block():
    rows = [row for row in read_file(OWNER) if row.address == 0x00581260]
    assert len(rows) == 1, "missing or duplicate mapped consider_designs owner"
    row, = rows
    assert row.mode == Mode.BODY and row.region
    assert row.state == State.IMPLEMENTED
    assert row.image_spans == ((0x00581260, 0x00583C8F),)
    assert row.name == "?consider_designs@@YAXH@Z", (
        "mapped body identity/decorated signature changed")
    assert SIGNATURE in row.region, "mapped owner is not the signed-int body"
    mapped_support = (
        "consider_designs_plan_proposal",
        "consider_designs_strategy_tail",
        "consider_designs_tech_selection",
        "consider_designs_plan3_selection",
        "consider_designs_plan1_ability_selection",
        "consider_designs_weapon_and_armor_choices",
        "consider_designs_plan2_fallback",
    )
    for name in mapped_support:
        assert name in row.region, "mapped source region omits support " + name
    source = OWNER.read_text(encoding="utf-8")
    assert source.count(SUPPORT_MARKER) == 1, "support boundary must be unique"
    support_start = source.index(SUPPORT_MARKER)
    function = _definition(source[support_start:], SIGNATURE)
    function_start = source.index(function, support_start)
    owner_block = source[support_start:function_start + len(function)]

    for name in mapped_support:
        assert name in owner_block, "owner support is missing " + name
    assert "BODY GOES HERE" not in owner_block
    assert "PENDING_BODY" not in owner_block
    assert ".inc" not in owner_block
    pending = (ROOT / "src/pending_bodies.cpp").read_text(encoding="utf-8")
    assert "PENDING_BODY(0x00581260" not in pending, (
        "old consider_designs forwarding body remains active")
    return owner_block


def _compile(tmp_path, owner_block=None):
    owner_block = _actual_owner_block() if owner_block is None else owner_block
    fixture = FIXTURE.read_text(encoding="utf-8")
    marker = "/* __SOURCE_EXTRACTED_FROM_MAPPED_VEH_OWNER__ */"
    assert fixture.count(marker) == 1
    cpp = tmp_path / "consider_designs_actual_owner.cpp"
    binary = tmp_path / "consider_designs_actual_owner"
    cpp.write_text(fixture.replace(marker, owner_block), encoding="utf-8")
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    command = [
        compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
        "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
        str(cpp), "-o", str(binary),
    ]
    result = subprocess.run(command, capture_output=True, text=True, timeout=45)
    assert result.returncode == 0, result.stdout + result.stderr
    return binary


def _run(binary, *args):
    env = dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
               UBSAN_OPTIONS="halt_on_error=1")
    return subprocess.run([str(binary), *args], capture_output=True,
                          text=True, timeout=15, env=env)


def test_mapped_owner_runs_complete_no_technology_chain_in_order(tmp_path):
    result = _run(_compile(tmp_path))
    assert result.returncode == 0, result.stdout + result.stderr
    lines = result.stdout.splitlines()
    assert lines[0] == "PICKS 12"
    assert lines[1:] == EXPECTED_PICKS


def test_mapped_owner_plan1_sentinel_preserves_current_armor(tmp_path):
    result = _run(_compile(tmp_path))
    assert result.returncode == 0, result.stdout + result.stderr
    # The actual extracted body begins with a direct helper control using
    # selected weapon 2, selected armor -1, and current prototype armor 5.


def test_mapped_owner_plan0_uses_scanned_weapon(tmp_path):
    result = _run(_compile(tmp_path), "plan0")
    assert result.returncode == 0, result.stdout + result.stderr


def test_owner_mutant_without_plan1_armor_guard_is_caught(tmp_path):
    owner = _actual_owner_block()
    anchor = (
        "if (state.selected_armor_id >= 0) {\n"
        "                const int reactor_limit ="
    )
    assert owner.count(anchor) == 1
    damaged = owner.replace(
        anchor,
        "if (state.selected_armor_id >= -1) {\n"
        "                const int reactor_limit =",
        1)
    result = _run(_compile(tmp_path, damaged))
    assert result.returncode != 0, "Armor[-1] mutant escaped ASan"


def test_owner_mutant_using_current_weapon_in_plan0_is_caught(tmp_path):
    owner = _actual_owner_block()
    original = (
        "propose_proto(faction_id, chassis_id, state.selected_weapon_id,\n"
        "                      prototype.armor_id, abilities, reactor_id,\n"
        "                      category, 0);"
    )
    assert owner.count(original) == 1
    damaged = owner.replace(
        original,
        "propose_proto(faction_id, chassis_id, prototype.weapon_id,\n"
        "                      prototype.armor_id, abilities, reactor_id,\n"
        "                      category, 0);",
        1)
    result = _run(_compile(tmp_path, damaged), "plan0")
    assert result.returncode != 0, "plan-0 scanned-weapon mutant escaped"


def test_owner_mutant_changing_plan3_faction_is_caught(tmp_path):
    owner = _actual_owner_block()
    original = "        faction_id, selection, 2 << state.reactor_tier,\n"
    assert owner.count(original) == 1
    damaged = owner.replace(
        original,
        "        faction_id + 1, selection, 2 << state.reactor_tier,\n",
        1)
    result = _run(_compile(tmp_path, damaged))
    assert result.returncode == 0, result.stdout + result.stderr
    assert result.stdout.splitlines()[1:] != EXPECTED_PICKS


def test_pinned_native_sentinel_and_armor_prerequisite_edges():
    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original executable is unavailable")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    sentinel_span = Span(0x00581659, 0x005816BF)
    sentinel = list(decoder.disasm(image.read(sentinel_span), sentinel_span.start))
    branch = next((insn for insn in sentinel if insn.address == 0x0058166D), None)
    assert branch is not None and branch.mnemonic == "jl"
    assert branch.op_str == "0x5816bf"
    reads = [insn for insn in sentinel
             if insn.address > branch.address and "0x94f280" in insn.op_str]
    assert reads and reads[0].address == 0x005816B1

    prereq_span = Span(0x005814D7, 0x005814F0)
    prereq = list(decoder.disasm(image.read(prereq_span), prereq_span.start))
    assert any(insn.address == 0x005814DE and insn.mnemonic == "push"
               for insn in prereq)
    assert any(insn.address == 0x005814E0 and insn.mnemonic == "call"
               and insn.op_str == "0x5b9f20" for insn in prereq)
    assert any(insn.address == 0x005814EA and insn.mnemonic == "je"
               and insn.op_str == "0x581587" for insn in prereq)
