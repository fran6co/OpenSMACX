"""Transferred callback controls, reconciled against current faction.cpp."""
from pathlib import Path
import os
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src/faction.cpp"
HARNESS = ROOT / "recovery/tests/council_vote_purchase_harness.cpp"
SIGNATURE = "void __cdecl buy_council_vote("
MARKER = "// PRODUCT_BODY_INSERTION_POINT"


def actual_body():
    source = PRODUCT.read_text()
    start = source.find(SIGNATURE)
    assert start >= 0
    opening = source.find("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                body = source[start : pos + 1]
                # This fixture predates two current typed globals; use scalar
                # test doubles with the same current owner values.
                return (body.replace("GameDataStateStorage.fields_.governor_faction_", "GovernorFaction")
                            .replace("GameDataStateStorage.fields_.turn_current_num_", "TurnCurrentNum"))
    raise AssertionError("unterminated buy_council_vote body")


def run_with_callbacks(extra_main):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    fixture = HARNESS.read_text()
    assert fixture.count(MARKER) == 1
    fixture = fixture.replace(
        "struct Popup {",
        "extern Data PlayersData[8]; extern int IsMultiplayerNet;\n"
        "int mutate_exec_energy=-9999, mutate_exec_network=-1;\n"
        "int parse_calls=0, mutate_energy_parse=-1, parse_energy_value=-9999;\n"
        "int mutate_tech_parse=-1, parse_tech_value=-1;\n"
        "struct Popup {",
    )
    fixture = fixture.replace(
        "int exec(int, int (*)(void)) { ++exec_calls; last_rows=d.rows; return result; }",
        "int exec(int, int (*)(void)) { ++exec_calls; last_rows=d.rows; "
        "if(mutate_exec_energy!=-9999) PlayersData[1].energy_reserves=mutate_exec_energy; "
        "if(mutate_exec_network>=0) IsMultiplayerNet=mutate_exec_network; return result; }",
    )
    fixture = fixture.replace(
        "void parse_string(const char *s, char *out) { if (s != out) std::strncpy(out, s, 1020); out[1020]=0; }",
        "void parse_string(const char *s, char *out) { ++parse_calls; "
        "if(parse_calls==mutate_energy_parse && parse_energy_value!=-9999) PlayersData[1].energy_reserves=parse_energy_value; "
        "if(parse_calls==mutate_tech_parse && parse_tech_value>=0) { DiploScratchFAA8=parse_tech_value; DiploScratchFA18=DiploScratchFA1C=DiploScratchFA28=-1; } "
        "if (s != out) std::strncpy(out, s, 1020); out[1020]=0; }",
    )
    fixture = fixture.replace(
        "force_text_open=0; mutate_treaty_on_xpop=0;",
        "force_text_open=0; mutate_treaty_on_xpop=0; mutate_exec_energy=-9999; "
        "mutate_exec_network=-1; parse_calls=0; mutate_energy_parse=-1; "
        "parse_energy_value=-9999; mutate_tech_parse=-1; parse_tech_value=-1;",
    )
    fixture = fixture.replace(
        "return 0;\n}",
        extra_main + "\n    return 0;\n}",
    )
    with tempfile.TemporaryDirectory(prefix="retained-council-callback-") as td:
        cpp = Path(td) / "callback.cpp"
        exe = Path(td) / "callback"
        cpp.write_text(fixture.replace(MARKER, actual_body()))
        built = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
             str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30,
        )
        assert built.returncode == 0, built.stderr
        result = subprocess.run([str(exe)], capture_output=True, text=True,
                                env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
                                timeout=10)
        assert result.returncode == 0, result.stdout + result.stderr


def test_popup_callbacks_update_network_mode_after_exec():
    run_with_callbacks(r"""
    reset(); Popup::result=1; PlayersData[1].energy_reserves=100;
    mutate_exec_energy=77; mutate_exec_network=1;
    buy_council_vote(1,2,2,3);
    assert(PlayersData[1].energy_reserves==77);
    assert(find("message").a[0]==0x2447 && count("sync")==1);
    """)


def test_menu_parser_mutations_change_live_energy_and_tech_eligibility():
    run_with_callbacks(r"""
    reset(); Popup::result=2; PlayersData[1].energy_reserves=10000;
    DiploScratchFAA8=DiploScratchFA18=DiploScratchFA1C=DiploScratchFA28=-1;
    mutate_energy_parse=2; parse_energy_value=0;
    mutate_tech_parse=3; parse_tech_value=21;
    buy_council_vote(1,2,1,0);
    assert((row_ids()==std::vector<int>{0,2}));
    assert(count("tech_local")==1 && find("tech_local").a[1]==21);
    """)
