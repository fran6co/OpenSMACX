"""Actual-source callback/lifetime controls, not native ABI or game proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).parent / "gameplay_picker_supply"


def actual_body(name):
    source = (ROOT / "src/console.cpp").read_text()
    address = {"veh_pick": 0x48D0D0, "supply_options": 0x593A00}[name]
    records = [r for r in read_file(ROOT / "src/console.cpp")
               if r.address == address]
    assert len(records) == 1 and records[0].region
    body = _definition(records[0].region, "int __cdecl " + name + "(")
    if name == "supply_options":
        body = "\n".join(_definition(source, "static int supply_wrap_" + op + "(")
                         for op in ("add", "sub", "mul")) + "\n" + body
    return body


def run_fixture(tmp_path, compiler, name, body):
    fixture = (FIXTURES / (name + ".cpp")).read_text()
    assert fixture.count("// ACTUAL_SOURCE_BODY") == 1
    path = tmp_path / (name + ".cpp")
    path.write_text(fixture.replace("// ACTUAL_SOURCE_BODY", body))
    executable = tmp_path / name
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                    str(path), "-o", str(executable)], check=True, capture_output=True)
    return subprocess.run([str(executable)], capture_output=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("name", ["veh_pick", "supply_options"])
def test_actual_owner_callback_and_cleanup_order(tmp_path, compiler_name, name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    result = run_fixture(tmp_path, compiler, name, actual_body(name))
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("damage", ["action_flags", "early_close", "stack_link", "modal_zero", "text_pointer", "picture_binding"])
def test_controls_detect_source_damage(tmp_path, damage):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is unavailable")
    name = "veh_pick" if damage == "stack_link" else "supply_options"
    body = actual_body(name)
    if damage == "action_flags":
        old = "(action_flags & 1u) != 0 && (action_flags & 2u) == 0"
        new = "(action_flags & 1u) == 0 || (action_flags & 2u) != 0"
    elif damage == "early_close":
        old = 'if (X_text_open(ClimatePopupTitle[0], "SUPPLYMENU") != 0) {\n        return 0;'
        new = 'if (X_text_open(ClimatePopupTitle[0], "SUPPLYMENU") != 0) {\n        popup.close();\n        return 0;'
    elif damage == "modal_zero":
        old = "if (selected_action == 0) {\n            return 0;"
        new = "if (selected_action == 0) {\n            return 1;"
    elif damage == "text_pointer":
        old = 'parse_say(0, reinterpret_cast<int>(Facility[facility_id].name), -1, -1);\n        parse_string(TextBufferGetPtr, StringTemp);'
        new = 'parse_say(0, reinterpret_cast<int>(Facility[facility_id].name), -1, -1);\n        parse_string(localized, StringTemp);'
    elif damage == "picture_binding":
        old = 'popup.set_caller_picture(&popup.owned_picture());'
        new = '(void)popup.owned_picture();'
    else:
        old = "const int next_vehicle = Vehs[listed_vehicle].next_veh_id_stack;"
        new = "const int next_vehicle = listed_vehicle == 0 ? 1 : Vehs[listed_vehicle].next_veh_id_stack;"
    assert body.count(old) == 1
    result = run_fixture(tmp_path, compiler, name, body.replace(old, new))
    assert result.returncode != 0, "damaged owner escaped the behavioral control"
