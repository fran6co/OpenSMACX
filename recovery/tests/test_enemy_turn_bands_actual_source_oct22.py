"""Behavior controls for the actual mapped enemy_turn definition.

The harness supplies bounded game-state doubles. It checks the recovered
source body against pinned branch/effect witnesses, not the complete game or
the VC6 ABI. The decrement mutant is expected to time out because a failed
priority-8 guard must advance to the next vehicle rather than repeat forever.
"""
from __future__ import annotations

import shutil
import subprocess
import os
from pathlib import Path

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.local_variants import _bound_function_body_bytes, _marker_identity
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source.mangled import qualified_name
from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ADDRESS = 0x00579510
OWNER = "src/general.cpp"
SIGNATURE = "void __cdecl enemy_turn(int faction_id)"
SYMBOL = "?enemy_turn@@YAHH@Z"
EMITTED_SYMBOL = "?enemy_turn@@YAXH@Z"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BODY_MARKER = "/* ACTUAL_ENEMY_TURN_BODY */"
FIXTURE = Path(__file__).parent / "fixtures" / "enemy_turn_bands_actual_source_oct22.cpp"


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").is_file() and (parent / OWNER).is_file():
            return parent
    raise RuntimeError("cannot locate OpenSMACX worktree")


def _actual_body() -> str:
    root = _root()
    path = root / OWNER
    source = path.read_text(encoding="utf-8")
    records = [record for record in read_file(path) if record.address == ADDRESS]
    assert len(records) == 1
    record = records[0]
    assert record.name == SYMBOL
    assert (record.symbol or record.name) == EMITTED_SYMBOL
    assert record.image_spans == ((0x00579510, 0x00579769),)
    assert Path(record.path).resolve() == path.resolve()
    assert source.count("// ORIGINAL: 0x00579510") == 1
    assert record.region.count("// RECOVERY_BINDING: v1 ") == 1
    body = extract_definition(source, SIGNATURE)
    markers = tuple(_marker_identity(row) for row in read_file(path))
    start, end = _bound_function_body_bytes(source, qualified_name(EMITTED_SYMBOL), path, record, markers)
    assert body.encode().endswith(source.encode()[start:end])
    return body


def test_pinned_original_dispatch_tail_and_effect_anchors():
    root = _root()
    paths = ([Path(os.environ["OPENSMACX_IMAGE"])] if os.environ.get("OPENSMACX_IMAGE")
             else [parent / ".opensmacx/game/terranx_original.exe" for parent in (root, *root.parents)])
    path = next((candidate for candidate in paths if candidate.is_file()), None)
    if path is None:
        pytest.skip("pinned proprietary image unavailable")
    image = PeImage(path.read_bytes(), PIN)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(ADDRESS, 0x00579769)), ADDRESS))
    cursor = ADDRESS
    for instruction in instructions:
        assert instruction.address == cursor
        cursor += instruction.size
    assert cursor == 0x00579769 and len(instructions) == 184
    by_address = {instruction.address: instruction for instruction in instructions}
    anchors = {
        0x579643: ("cmp", "edx, 7"), 0x579646: ("jge", "0x579687"),
        0x579687: ("cmp", "edx, 8"), 0x57968A: ("jne", "0x5796ab"),
        0x57969D: ("jne", "0x5795a7"), 0x5796A5: ("je", "0x5795a7"),
        0x5795A7: ("dec", "ebx"), 0x5795A8: ("sub", "esi, 0x34"),
        0x5796D3: ("je", "0x5796fc"), 0x57974F: ("cmp", "eax, 0x20"),
        0x579755: ("jle", "0x5796bc"),
        0x579559: ("call", "0x5c0db0"), 0x579594: ("call", "0x5b9580"),
        0x57964E: ("call", "0x5bf1f0"), 0x5796D8: ("call", "0x5c1540"),
        0x579703: ("call", "0x579240"), 0x57975C: ("call", "0x5c1d20"),
    }
    for address, expected in anchors.items():
        actual = by_address[address]
        assert (actual.mnemonic, actual.op_str) == expected


def _changed(body: str, old: str, new: str) -> str:
    assert body.count(old) == 1, (old, body.count(old))
    return body.replace(old, new, 1)


def _mutant(body: str, name: str) -> str:
    if name == "omit_late_bands":
        return _changed(body, "} else {\n                    matched = true;\n                }",
                        "} else if (priority < 7) {\n                    matched = true;\n                }")
    if name == "apply_shared_filters_to_late_bands":
        return _changed(body, "priority >= 7\n                        ||",
                        "false\n                        ||")
    if name == "continue_skips_vehicle_decrement":
        mutated = _changed(body, "for (; veh_id >= 0; --veh_id, --veh) {", "do {")
        return _changed(
            mutated,
            "            }\n        }\n    }\n\n    PlayersData[faction_id].flags",
            "            }\n            --veh_id;\n            --veh;\n        } while (veh_id >= 0);\n    }\n\n    PlayersData[faction_id].flags",
        )
    if name == "remove_priority_eight_speed_exemption":
        return _changed(body, "priority != 8 || turns_spent != 0",
                        "true")
    if name == "wrong_priority_eight_guard":
        return _changed(body, "faction_id == 0", "faction_id != 0")
    if name == "wrong_flag_mask":
        return _changed(body, "flags &= ~PFLAG_UNK_10000;",
                        "flags &= PFLAG_UNK_10000;")
    raise AssertionError(f"unknown mutant: {name}")


def _compile_run(tmp_path: Path, compiler: str, body: str, label: str):
    harness = FIXTURE.read_text(encoding="utf-8")
    assert harness.count(BODY_MARKER) == 1
    source_text = harness.replace(BODY_MARKER, body)
    source = tmp_path / f"{label}.cpp"
    binary = tmp_path / label
    source.write_text(source_text, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-O0", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=45, check=False,
    )
    if built.returncode != 0:
        return built, None
    try:
        ran = subprocess.run(
            [str(binary)], capture_output=True, text=True, timeout=8, check=False,
        )
    except subprocess.TimeoutExpired:
        return built, "timeout"
    return built, ran


@pytest.fixture(params=["g++", "clang++"])
def compiler(request):
    resolved = shutil.which(request.param)
    if resolved is None:
        pytest.skip(f"{request.param} is not installed")
    return resolved


def test_actual_enemy_turn_band_and_effect_controls(tmp_path, compiler):
    built, ran = _compile_run(tmp_path, compiler, _actual_body(), "enemy_turn_actual")
    assert built.returncode == 0, built.stderr
    assert ran != "timeout", "faithful source unexpectedly hung"
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("name", [
    "omit_late_bands",
    "apply_shared_filters_to_late_bands",
    "continue_skips_vehicle_decrement",
    "remove_priority_eight_speed_exemption",
    "wrong_priority_eight_guard",
    "wrong_flag_mask",
])
def test_compiling_damage_mutants_are_detected(tmp_path, compiler, name):
    mutant = _mutant(_actual_body(), name)
    built, ran = _compile_run(tmp_path, compiler, mutant, f"enemy_turn_{name}")
    assert built.returncode == 0, (name, built.stderr)
    assert ran == "timeout" or ran.returncode != 0, name
