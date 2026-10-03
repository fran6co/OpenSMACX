"""Extract real owner bodies; host witnesses are not native recovery acceptance."""
from pathlib import Path
import shutil

import pytest
from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition
from test_popup_animation_owner_behavior import run_control

ROOT = Path(__file__).resolve().parents[2]
BODIES = (
    ("game", 0x444D90, "void __cdecl diplo("),
    ("veh", 0x50B8F0, "int __cdecl veh_next_stack_id("),
    ("general", 0x5391A0, "void __cdecl read_and_parse_text_line("),
    ("win", 0x5F83B0, "void __cdecl flip_active_bubble_text("),
)


def projected_control(damage=None):
    bodies = []
    for owner, address, signature in BODIES:
        row = next(row for row in read_file(ROOT / "src" / (owner + ".cpp"))
                   if row.address == address)
        bodies.append(_definition(row.region, signature))
    source = "\n\n".join(bodies)
    if damage is not None:
        old, new = damage
        assert old in source
        source = source.replace(old, new, 1)
    fixture = Path(__file__).with_name("short_owner_batch_control.cpp").read_text()
    assert fixture.count("// ACTUAL_SOURCE_BODIES") == 1
    return fixture.replace("// ACTUAL_SOURCE_BODIES", source)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_short_owner_bodies(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = run_control(tmp_path, compiler, projected_control())
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("old,new", [
    ("exec(faction_id, faction_id_with)", "exec(faction_id_with, faction_id)"),
    ("return veh_id;", "return -1;"),
    ("return Vehs[veh_id].next_veh_id_stack;",
     "return static_cast<unsigned short>(Vehs[veh_id].next_veh_id_stack);"),
    ("text_get();", "/* missing input advance */"),
    ("StringTemp[0] = 0;", "/* missing output reset */"),
    ("parse_string(input, StringTemp);", "parse_string(StringTemp, input);"),
    ("WinBubbleActive != 0", "WinBubbleActive > 0"),
    ("WinBubbleActive != 0", "WinBubbleActive == 0"),
])
def test_short_owner_damage_detected(tmp_path, old, new):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = run_control(tmp_path, compiler, projected_control((old, new)))
    assert result.returncode != 0, "damaged body must fail a behavioral witness"
