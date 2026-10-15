"""Bounded host witnesses of installed config_game, not native/game proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import State, read_file
from match_recovery.object_source import parse_objects

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/newgame.cpp").is_file())
SOURCE = ROOT / "src/newgame.cpp"
FIXTURE = Path(__file__).with_name("fixtures") / "config_game_api.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
TOKENS = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S)


def definition(text, signature):
    assert text.count(signature) == 1, signature
    start = text.index(signature)
    opened = False
    depth = 0
    for token in TOKENS.finditer(text, start):
        if token.group() == "{":
            opened = True
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return text[start:token.end()]
    raise AssertionError("unterminated definition: " + signature)


def actual_bundle():
    text = SOURCE.read_text()
    records = {row.address: row for row in read_file(SOURCE)}
    primary = records[0x589D30]
    callback = records[0x5898A0]
    assert primary.state == callback.state == State.IMPLEMENTED
    assert primary.image_spans == ((0x589D30, 0x58C29D), (0x66028A, 0x6605A5))
    assert callback.image_spans == ((0x5898A0, 0x589A24),)
    declarations = ["Popup ConfigFactionPreview;", "int ConfigFactionDetailMode;",
                    "char ConfigRandomLeaderInitial[4];"]
    for declaration in declarations:
        assert text.splitlines().count(declaration) == 1
    functions = [definition(callback.region, "void __cdecl thumb_routine(int faction)"),
                 definition(text, "static void config_game_seed_checks()"),
                 definition(text, "static void config_game_apply_checks()"),
                 definition(text, "static void config_game_play_voice(int faction, bool flagged)"),
                 definition(text, "static unsigned char config_game_original_rule_byte(int selected)"),
                 definition(primary.region, "int __cdecl config_game(int mode)")]
    bundle = "\n\n".join(declarations + functions)
    assert "BODY GOES HERE" not in bundle
    assert "__asm" not in bundle
    assert "reinterpret_cast<Popup" not in bundle
    assert not re.search(r"reinterpret_cast<[^>]+>[(]0x[0-9A-Fa-f]+", bundle)
    return bundle


def projected(damage=None):
    body = actual_bundle()
    if damage is not None:
        old, new = damage
        expected_count = 2 if old.startswith("mode == 2 && !") else 1
        assert body.count(old) == expected_count, old
        body = body.replace(old, new)
    fixture = FIXTURE.read_text()
    assert fixture.count("// @@ACTUAL_CONFIG_GAME@@") == 1
    return fixture.replace("// @@ACTUAL_CONFIG_GAME@@", body)


def run_control(tmp_path, compiler, source):
    path = tmp_path / "config_game.cpp"
    path.write_text(source)
    binary = tmp_path / "config_game"
    compile_result = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", "-Werror",
         str(path), "-o", str(binary)], capture_output=True, timeout=60)
    assert compile_result.returncode == 0, compile_result.stderr.decode(errors="replace")
    return subprocess.run([str(binary)], capture_output=True, timeout=15)


@pytest.mark.parametrize("name", ["g++", "clang++"])
def test_actual_installed_config_game(tmp_path, name):
    compiler = shutil.which(name)
    if compiler is None:
        pytest.skip(name + " unavailable")
    result = run_control(tmp_path, compiler, projected())
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("old,new", [
    ("    return 0;\n}", "    return 1;\n}"),
    ("switch (residue / 7)", "switch (residue / 21)"),
    ("strcpy(Players[picked].noun_faction, ParseStrBuffer[2].str);",
     "strcpy(Players[picked].noun_faction, ParseStrBuffer[4].str);"),
    ("selected = DefaultPrefs.faction_id;", "selected = 1;"),
    ("if (picked < 0) return 1;", "if (picked < 0) return 0;"),
    ("mode == 2 && !(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << i))",
     "mode == 2 && (GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1u << i))"),
    ("GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =\n        static_cast<unsigned char>(human_mask);",
     "GameDataStateStorage.fields_.lock_masks_.enable_mask_ =\n        static_cast<unsigned char>(human_mask);"),
    ("GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ =\n        static_cast<unsigned char>(human_mask);",
     "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ =\n        static_cast<unsigned char>(human_mask);"),
])
def test_actual_config_game_damage_fails(tmp_path, old, new):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    result = run_control(tmp_path, compiler, projected((old, new)))
    assert result.returncode != 0, "mutated actual product body passed witness"


def test_random_initial_storage_does_not_claim_unknown_extent():
    # Native qualification may add a start-only binding. The preparation overlay
    # deliberately leaves tokens unbound until physical compiler observation.
    rows = parse_objects(SOURCE.read_text(), "src/newgame.cpp", PIN, address_space="pe:va")
    observed = [row for row in rows if row.origin.address == 0x945834]
    assert len(observed) <= 1
    if observed:
        assert observed[0].span is None


def test_picker_parser_loop_variables_have_explicit_block_scopes():
    # VC6 for-init variables live through their containing block. These three
    # loops must not place initialized i in function scope across edit_names
    # or parse_factions, nor redeclare it later in that same scope.
    body = definition(SOURCE.read_text(), "int __cdecl config_game(int mode)")
    depth = 0
    offsets = []
    for token in TOKENS.finditer(body):
        if token.group() == "{":
            depth += 1
        elif token.group() == "}":
            depth -= 1
        offsets.append((token.end(), depth))
    loops = list(re.finditer(r"for [(]int i =", body))
    assert len(loops) == 7
    for loop in loops:
        enclosing = [level for end, level in offsets if end <= loop.start()][-1]
        assert enclosing >= 2, "loop variable leaks to function scope"
    assert body.count("    {\n        for (int i = 1; i < 8; ++i)") == 3
