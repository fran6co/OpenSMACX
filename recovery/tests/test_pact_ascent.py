"""Actual-source C++98 callback and damage controls; not native/game proof."""
import re
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/game.cpp").is_file())
FIXTURE = Path(__file__).with_name("pact_ascent_control.cpp").read_text()


def _closing_brace(text, opening):
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if ch == "\\":
                i += 1
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif ch == "/" and nxt == "/":
            state = "line"
            i += 1
        elif ch == "/" and nxt == "*":
            state = "block"
            i += 1
        elif ch == '"':
            state = "string"
        elif ch == "'":
            state = "char"
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise AssertionError("unterminated actual source definition")


def definition(text, signature):
    match = re.search(signature, text, re.S)
    assert match, "actual product owner definition not found: " + signature
    opening = text.find("{", match.end())
    end = _closing_brace(text, opening)
    result = text[match.start():end + 1]
    assert "PENDING_BODY" not in result and "BODY GOES HERE" not in result
    return result.replace("nullptr", "0")


def turn_upkeep_ascent(source):
    start = source.index("if (TurnCurrentNum == 0x4b &&", source.index("void __cdecl turn_upkeep()"))
    end = source.index("if ((TurnCurrentNum & 3) == 1)", start)
    return "void turn_upkeep_ascent_control() {\n" + source[start:end] + "\n}"


def load_actual_bodies():
    faction = (ROOT / "src/faction.cpp").read_text()
    game = (ROOT / "src/game.cpp").read_text()
    return {
        "pact": definition(faction, r"void\s+__cdecl\s+pact_treaty\s*\("),
        "ascent": definition(game, r"void\s+__cdecl\s+ascent\s*\("),
        "turn_ascent": turn_upkeep_ascent(game),
    }


def build(binary_dir, bodies, compiler):
    source_text = FIXTURE
    for key, token in (
        ("pact", "// @@PACT_TREATY_DEFINITION@@"),
        ("ascent", "// @@ASCENT_DEFINITION@@"),
        ("turn_ascent", "// @@TURN_UPKEEP_ASCENT_FRAGMENT@@"),
    ):
        source_text = source_text.replace(token, bodies[key], 1)
    assert "@@PACT_TREATY_DEFINITION@@" not in source_text
    cpp = binary_dir / "actual_source_control.cpp"
    exe = binary_dir / "actual_source_control"
    cpp.write_text(source_text)
    result = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=45
    )
    assert result.returncode == 0, result.stdout + result.stderr
    return exe


def run(binary, expected_success):
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    if expected_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "actual-source damage control failed to detect mutation"


def test_actual_source_callback_and_damage_controls():
    bodies = load_actual_bodies()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for this private host control"
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="pact-ascent-source-control-") as temp:
            temp_path = Path(temp)
            run(build(temp_path, bodies, compiler), True)

            damaged = dict(bodies)
            changed, count = re.subn(
                r'NetMsgPrimary\.pop\("PACTTREATY"',
                'NetMsgPrimary.pop("PACTTRUCE"', damaged["pact"], count=1
            )
            assert count == 1
            damaged["pact"] = changed
            run(build(temp_path, damaged, compiler), False)

            damaged = dict(bodies)
            changed, count = re.subn(
                r"Players\[faction_id_with\]\.noun_faction",
                "Players[faction_id].noun_faction",
                damaged["pact"], count=1
            )
            assert count == 1
            damaged["pact"] = changed
            run(build(temp_path, damaged, compiler), False)

            damaged = dict(bodies)
            changed, count = re.subn(
                r"PlayersData\[faction_for_flag_and_title\]\.flags\s*\|=\s*0x2000;",
                "/* one-shot mark omitted */",
                damaged["ascent"], count=1
            )
            assert count == 1
            damaged["ascent"] = changed
            run(build(temp_path, damaged, compiler), False)

            damaged = dict(bodies)
            changed, count = re.subn(
                r"PluralityDefault\s*=\s*0;\s*GenderDefault\s*=\s*Players\[",
                "GenderDefault = 0; PluralityDefault = Players[",
                damaged["ascent"], count=1
            )
            assert count == 1
            damaged["ascent"] = changed
            run(build(temp_path, damaged, compiler), False)

            damaged = dict(bodies)
            def reverse_defaults(match):
                row = match.group(1)
                return "GenderDefault = 0; PluralityDefault = Players[" + row + "].is_leader_female;"

            changed, count = re.subn(
                r"PluralityDefault\s*=\s*0;\s*GenderDefault\s*=\s*Players\[(\w+)\]\.is_leader_female;",
                reverse_defaults,
                damaged["turn_ascent"], count=2
            )
            assert count == 2
            damaged["turn_ascent"] = changed
            run(build(temp_path, damaged, compiler), False)

            damaged = dict(bodies)
            changed, count = re.subn(
                r"const int faction_for_name\s*=\s*ConsoleGlobal\.faction_id_;",
                "const int faction_for_name = faction_for_flag_and_title;",
                damaged["ascent"], count=1
            )
            assert count == 1
            damaged["ascent"] = changed
            run(build(temp_path, damaged, compiler), False)

            damaged = dict(bodies)
            changed, count = re.subn(
                r"const int faction_for_name\s*=\s*ConsoleGlobal\.faction_id_;",
                "const int faction_for_name = faction;",
                damaged["turn_ascent"], count=1
            )
            assert count == 1
            damaged["turn_ascent"] = changed
            run(build(temp_path, damaged, compiler), False)

            damaged = dict(bodies)
            changed, count = re.subn(
                r"parse_says\(0,\s*Players\[faction_id\]\.title_leader",
                "parse_says(0, Players[faction_id].name_leader",
                damaged["pact"], count=1
            )
            assert count == 1
            damaged["pact"] = changed
            run(build(temp_path, damaged, compiler), False)


