from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/netdaemon.cpp"
WINDOW_HEADER = ROOT / "src/game_data_state_window.h"


def _union_declarations() -> str:
    text = WINDOW_HEADER.read_text()
    match = re.search(
        r"struct GameDataLockMaskBytes\s*\{.*?\n\};\s*"
        r"union GameDataLockMaskView\s*\{.*?\n\};",
        text,
        re.S,
    )
    assert match, "actual lock-mask byte view declarations must remain present"
    return match.group(0)


def _human_clear_statement() -> str:
    text = SOURCE.read_text()
    match = re.search(
        r"GameDataStateStorage\.fields_\.lock_masks_\.bytes_\.human_mask_\s*=\s*"
        r"static_cast<uint8_t>\(GameDataStateStorage\.fields_\.lock_masks_\.bytes_\.human_mask_\s*&\s*"
        r"~static_cast<uint8_t>\(1u\s*<<\s*\(player_id\s*&\s*31\)\)\);",
        text,
        re.S,
    )
    assert match, "on_player_deleted must update only the named human byte"
    return match.group(0)


def _function_body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1 : index]
    raise AssertionError(f"unterminated function body: {signature}")


def _if_condition(text: str, marker: str) -> str:
    start = text.index(marker) + len(marker)
    depth = 1
    for index in range(start, len(text)):
        if text[index] == "(":
            depth += 1
        elif text[index] == ")":
            depth -= 1
            if depth == 0:
                return text[start:index]
    raise AssertionError(f"unterminated if condition after {marker}")


def _compile_and_run(compiler: str, code: str, *, run: bool = True):
    with tempfile.TemporaryDirectory(prefix="netdaemon-mask-test-") as temporary:
        root = Path(temporary)
        source = root / "test.cpp"
        binary = root / "test"
        source.write_text(code)
        compiled = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
            text=True,
            capture_output=True,
            timeout=20,
        )
        if not run or compiled.returncode != 0:
            return compiled, None
        executed = subprocess.run([str(binary)], text=True, capture_output=True, timeout=5)
        return compiled, executed


def _write_harness(statement: str) -> str:
    declarations = _union_declarations()
    return f"""
#include <cstdint>
#include <cstring>
{declarations}
struct Fields {{ GameDataLockMaskView lock_masks_; }};
struct Storage {{ Fields fields_; }};
Storage GameDataStateStorage;
void clear_human(int player_id) {{ {statement} }}
int main() {{
    unsigned char initial[4] = {{0xFF, 0xA5, 0xC3, 0x5A}};
    std::memcpy(&GameDataStateStorage.fields_.lock_masks_, initial, sizeof initial);
    clear_human(2);
    unsigned char after[4];
    std::memcpy(after, &GameDataStateStorage.fields_.lock_masks_, sizeof after);
    return after[0] == 0xFB && after[1] == 0xA5 &&
           after[2] == 0xC3 && after[3] == 0x5A ? 0 : 1;
}}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_human_byte_update_preserves_adjacent_bytes(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    compiled, executed = _compile_and_run(compiler, _write_harness(_human_clear_statement()))
    assert compiled.returncode == 0, compiled.stderr
    assert executed.returncode == 0, executed.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_wrong_byte_destination_mutant_compiles_but_fails_neighbor_control(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    wrong_destination = (
        "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = "
        "static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & "
        "~static_cast<uint8_t>(1u << (player_id & 31)));"
    )
    compiled, executed = _compile_and_run(compiler, _write_harness(wrong_destination))
    assert compiled.returncode == 0, compiled.stderr
    assert executed.returncode != 0, "wrong-byte destination must fail the real neighbor check"


def test_human_update_avoids_the_old_wide_mask_read_modify_write():
    body = _function_body(SOURCE.read_text(), "void NetDaemon::on_player_deleted")
    start = body.index("if (primary_state_.trailing_words_[0] == 2)")
    update = body[start : body.index("\n        }", start)]
    assert "bytes_.human_mask_ =" in update
    assert "enable_mask_" not in update
    assert "0xFFFFFF00" not in update


def test_handlers_use_named_bytes_at_native_byte_read_sites_without_replacing_dword_counts():
    text = SOURCE.read_text()
    done_turn = _function_body(text, "static void netdaemon_handle_client_done_turn")
    server_turn = _function_body(text, "static void netdaemon_handle_server_turn_info")
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_" in done_turn
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_" in done_turn
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_" in server_turn
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_" in server_turn
    assert done_turn.count("const uint8_t alive =") == 2
    assert server_turn.count("const uint8_t alive =") == 1

    # Population-count operands remain dword reads in both turn handlers.
    assert re.search(
        r"const uint32_t enabled\s*=\s*\n\s*GameDataStateStorage\.fields_\.lock_masks_\.enable_mask_\s*&\s*0xFFu;",
        done_turn,
    )
    assert re.search(
        r"const uint32_t enabled\s*=\s*\n\s*GameDataStateStorage\.fields_\.lock_masks_\.enable_mask_\s*&\s*0xFFu;",
        server_turn,
    )
    # Raw checksum views and the table-E full-mask arguments are intentionally
    # outside this width migration.
    assert "checksum((char *)&GameDataStateStorage.fields_.lock_masks_.enable_mask_, 2, cs)" in text
    assert "GameDataStateStorage.fields_.lock_masks_.enable_mask_);" in text


def test_on_player_deleted_reads_byte_masks_at_their_iteration_sites():
    text = SOURCE.read_text()
    body = _function_body(text, "void NetDaemon::on_player_deleted")
    assert "const uint8_t alive =" in body
    assert "const uint8_t humans = static_cast<uint8_t>" not in body
    loop = body.index("for (int faction = 1; faction < 8; ++faction)")
    alive_read = body.index("GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & bit", loop)
    human_read = body.index("GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit", alive_read)
    assert alive_read > loop and human_read > alive_read
    activation = body.index("candidate.active_ = static_cast<uint8_t>(", human_read)
    reload = body.index("const uint8_t humans =", activation)
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_" in body[reload : reload + 150]
    assert body.index("(candidate.active_ & humans) == humans", reload) > reload
    assert body.index("message_data(0x1206", reload) > reload
    assert body.count("GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & bit") == 1
    first_loop = body.index("for (int next = player_id + 1; next < 8; ++next)")
    assert "(alive & bit) != 0" in body[first_loop : body.index("if (!informed)", first_loop)]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_player_scan_condition_reloads_source_bytes_between_iterations(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = SOURCE.read_text()
    body = _function_body(source, "void NetDaemon::on_player_deleted")
    loop = body.index("for (int faction = 1; faction < 8; ++faction)")
    condition = _if_condition(body[loop:], "if (")
    code = f"""
#include <cstdint>
{_union_declarations()}
struct Fields {{ GameDataLockMaskView lock_masks_; }};
struct Storage {{ Fields fields_; }};
Storage GameDataStateStorage;
uint8_t alive;
uint8_t humans;
struct PlayerLock {{ uint8_t active_; int active() const {{ return active_; }} }};
bool skip_candidate(uint8_t bit, int faction, int player_id, PlayerLock &candidate) {{
    return ({condition});
}}
int main() {{
    GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = 0x0E;
    GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 0x0E;
    alive = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
    humans = GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_;
    int accepted = 0;
    PlayerLock candidate = {{2}};
    for (int faction = 1; faction < 4; ++faction) {{
        const uint8_t bit = static_cast<uint8_t>(1u << (faction & 31));
        if (skip_candidate(bit, faction, 7, candidate)) continue;
        ++accepted;
        if (faction == 1) {{
            // Represents message_data mutating shared state before the next iteration.
            GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = 0x04;
        }}
    }}
    return accepted == 2 ? 0 : 1;
}}
"""
    compiled, executed = _compile_and_run(compiler, code)
    assert compiled.returncode == 0, compiled.stderr
    assert executed.returncode == 0, executed.stderr

    cached_mutant = condition.replace(
        "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_", "alive"
    ).replace("GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_", "humans")
    mutant = code.replace(
        "bool skip_candidate(uint8_t bit, int faction, int player_id, PlayerLock &candidate) {\n"
        f"    return ({condition});",
        "bool skip_candidate(uint8_t bit, int faction, int player_id, PlayerLock &candidate) {\n"
        f"    return ({cached_mutant});",
    )
    assert mutant != code, "cached-mask damage control must alter the actual predicate"
    compiled, executed = _compile_and_run(compiler, mutant)
    assert compiled.returncode == 0, compiled.stderr
    assert executed.returncode != 0, "cached-mask mutant must differ after shared-state mutation"
