"""Actual-source and pinned controls for get_rating's typed rules mask.

Host execution checks the recovered C++ body with small, defined inputs. It is
not a native ABI/build check; the pinned PE assertion separately documents the
historical byte load and function span.
"""

from pathlib import Path
import hashlib
import os
import re
import shutil
import subprocess
import textwrap

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x005ABFF0
END = 0x005AC051
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BODY_SHA256 = "1c643b71135203664968a6eee217582d29952daa249951ed990ebed48baf3fbf"
BASELINE = "c84e9372dd0f850ef5b91c27208258630eea347a"
OLD_MASK = "if (*(unsigned char *)&GameDataStateStorage.fields_.game_rules_ & 0x40) {"
NEW_MASK = "if ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) != 0) {"
SIGNATURE = "int __cdecl get_rating(int faction_id, int arg2)"


def _definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/":
            state = "line"
            index += 1
        elif char == "/" and nxt == "*":
            state = "block"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _source():
    cpp = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    return cpp, _definition(cpp, SIGNATURE)


def _host_program(body):
    return textwrap.dedent(r"""\
        #include <cassert>
        #include <cstdint>
        #define __cdecl
        enum { RULES_INTENSE_RIVALRY = 0x40 };
        struct PlayerData { int diff_level; };
        static PlayerData PlayersData[8];
        struct GameDataStateWindowFields { uint32_t game_rules_; };
        struct GameDataStateWindow { GameDataStateWindowFields fields_; };
        static GameDataStateWindow GameDataStateStorage;
        BODY

        static int expected_rating(int difficulty, int scale, uint32_t rules) {
            const int added = difficulty >= 3
                ? ((difficulty - 1) * (difficulty - 1)) / 4 : 0;
            const int rivalry = (rules & RULES_INTENSE_RIVALRY) != 0 ? 1 : 0;
            return (difficulty + added + 6 + rivalry) * scale / 160;
        }

        int main() {
            const int difficulties[8] = {-3, -1, 0, 1, 2, 3, 4, 12};
            const int scales[] = {-100000, -320, -160, -1, 0, 1, 160, 320, 100000};
            uint32_t rules[72] = {
                0u, 0x20u, 0x80u, 0x80000000u, 0xffffffbfu,
                0x40u, 0xa5a50040u, 0xffffffffu
            };
            for (unsigned int bit = 0; bit < 32; ++bit) {
                rules[8 + 2 * bit] = uint32_t(1) << bit;
                rules[9 + 2 * bit] = (uint32_t(1) << bit) | 0x40u;
            }
            for (int faction = 0; faction < 8; ++faction)
                PlayersData[faction].diff_level = difficulties[faction];

            // Positive anchors for the legacy formula and low-byte flag.
            PlayersData[6].diff_level = 4;
            GameDataStateStorage.fields_.game_rules_ = 0;
            assert(get_rating(6, 160) == 12);
            GameDataStateStorage.fields_.game_rules_ = 0x40u;
            assert(get_rating(6, 160) == 13);

            for (int faction = 0; faction < 8; ++faction) {
                for (unsigned int rule_index = 0;
                     rule_index < sizeof(rules) / sizeof(rules[0]); ++rule_index) {
                    GameDataStateStorage.fields_.game_rules_ = rules[rule_index];
                    for (unsigned int scale_index = 0;
                         scale_index < sizeof(scales) / sizeof(scales[0]); ++scale_index) {
                        const int scale = scales[scale_index];
                        assert(get_rating(faction, scale) == expected_rating(
                            difficulties[faction], scale, rules[rule_index]));
                    }
                }
            }
            return 0;
        }
    """).replace("BODY", body)


def _run_host(tmp_path, compiler, body, name):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    cpp_path = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp_path.write_text(_host_program(body), encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp_path), "-o", str(binary)],
        capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_owner_body_is_a_single_typed_mask_change_from_immutable_baseline():
    cpp, body = _source()
    baseline = subprocess.check_output(
        ["git", "show", f"{BASELINE}:src/base.cpp"],
        cwd=ROOT, text=True, timeout=30)
    assert baseline.count(OLD_MASK) == 1
    assert cpp == baseline.replace(OLD_MASK, NEW_MASK, 1)
    assert NEW_MASK in body and OLD_MASK not in body

    fields = (ROOT / "src/game_data_state_window.h").read_text(encoding="utf-8")
    assert re.search(r"uint32_t\s+game_rules_;\s*//\s*\+0x00C\b", fields)
    assert re.search(
        r"offsetof\(GameDataStateWindowFields,\s*game_rules_\)\s*==\s*0x00C",
        fields)
    game = (ROOT / "src/game.h").read_text(encoding="utf-8")
    assert re.search(r"RULES_INTENSE_RIVALRY\s*=\s*0x40\b", game)
    producer = (ROOT / "src/game.cpp").read_text(encoding="utf-8")
    assert "GameDataStateStorage.fields_.game_rules_ = AlphaIniPrefs.rules;" in producer
    assert "GameDataStateStorage.fields_.game_rules_ |= 0x20;" in producer

    records = read_text(cpp, ROOT / "src/base.cpp")
    row, = [record for record in records if record.address == ADDRESS]
    assert row.name == "?get_rating@@YAHHH@Z"
    assert row.image_spans == ((ADDRESS, END),)
    assert row.byte_exact is True and row.semantic is False


def test_pinned_original_full_span_anchors_byte_test_branch_and_return():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original required; set OPENSMACX_IMAGE")
    assert image_pin(ROOT) == PIN
    data = image_path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    raw = image.read(Span(ADDRESS, END))
    assert len(raw) == 97
    assert hashlib.sha256(raw).hexdigest() == BODY_SHA256

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(raw, ADDRESS))
    cursor = ADDRESS
    by_address = {}
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded original byte at {cursor:#x}"
        by_address[instruction.address] = instruction
        cursor += instruction.size
    assert cursor == END and len(instructions) == 40
    test = by_address[0x005AC029]
    assert test.mnemonic == "test"
    assert test.operands[0].size == 1
    assert test.operands[0].mem.disp == 0x009A649C
    assert test.operands[1].imm == 0x40
    assert by_address[0x005AC030].mnemonic == "je"
    assert by_address[0x005AC030].operands[0].imm == 0x005AC033
    assert by_address[0x005AC032].mnemonic == "inc"
    assert by_address[0x005AC032].op_str == "edi"
    assert by_address[0x005AC050].mnemonic == "ret"
    assert not any(instruction.mnemonic == "call" for instruction in instructions)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_source_formula_and_mask_run_on_host_compilers(tmp_path, compiler):
    _, body = _source()
    result = _run_host(tmp_path, compiler, body, "get_rating_actual_source")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("RULES_INTENSE_RIVALRY", "0x20u"),
    ("!= 0) {", "== 0) {"),
    ("PlayersData[faction_id]", "PlayersData[(faction_id + 1) % 8]"),
    ("* arg2 / 0xa0", "* (arg2 + 1) / 0xa0"),
))
def test_semantic_mutants_fail_at_runtime(tmp_path, compiler, old, new):
    _, body = _source()
    assert old in body
    damaged = body.replace(old, new, 1)
    assert damaged != body
    result = _run_host(tmp_path, compiler, damaged, "get_rating_mutant")
    assert result.returncode != 0, "mutant unexpectedly passed actual-source fixture"
