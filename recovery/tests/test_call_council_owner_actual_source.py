"""GCC/Clang C++98 controls of actual call_council source and damaging mutants."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
HARNESS = Path(__file__).with_name("call_council_control.cpp")
PRODUCT = ROOT / "src/council.cpp"


def _body_from(source: str) -> str:
    marker = "void __cdecl call_council(int faction_id)"
    start = source.find(marker)
    if start < 0:
        raise AssertionError("call_council source definition not found")
    opening = source.find("{", start + len(marker))
    if opening < 0:
        raise AssertionError("call_council opening brace not found")
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                result = source[start : end + 1]
                if "BODY GOES HERE" in result:
                    raise AssertionError("product still contains call_council placeholder")
                return result
    raise AssertionError("call_council body is unterminated")


def _actual_body() -> str:
    return _body_from(PRODUCT.read_text())


def test_actual_body_has_no_executable_seed_address_scaffold() -> None:
    body = _actual_body()
    assert "reinterpret_cast<int *>(0x" not in body
    assert "0x009" not in body and "0x0068" not in body
    assert "popup.scroll_" not in body
    assert "BODY GOES HERE" not in body


def test_native_cleanup_lowering_is_not_repeated_manually() -> None:
    body = _actual_body()
    assert "popup.close()" not in body
    assert "scroll_.close()" not in body
    assert "Popup popup;" in body
    assert "Sprite council_sprite;" in body


def test_original_proposal_gate_and_uninitialized_stack_obligation() -> None:
    body = _actual_body()
    assert "if ((GameDataStateStorage.fields_.game_rules_ & 8) == 0 || " \
           "(GameDataStateStorage.fields_.game_state_ & 1) != 0" in body
    assert "proposed_target_priority:" not in body
    # The seed initializes s14 only after proposal 1 passes its gates. Keep
    # that residue obligation visible; do not invent a clean default here.
    assert body.count("alternate_selection = 0;") == 1
    assert body.index("alternate_selection = 0;") > body.index("supreme_leader_proposal:")
    assert '"NOTWHILEALIENSLIVE"' in body


def _compile_and_run(body, expect_success=True) -> None:
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for the host control"

    with tempfile.TemporaryDirectory(prefix="call-council-host-") as temp:
        body_path = Path(temp) / "call_council_extracted.inc"
        body_path.write_text(body)
        for compiler in compilers:
            executable = Path(temp) / (Path(compiler).name.replace("+", "p"))
            command = [
                compiler,
                "-std=c++98",
                "-O0",
                "-Wall",
                "-Wextra",
                "-DCALL_COUNCIL_EXTRACTED=\"%s\"" % body_path,
                str(HARNESS),
                "-o",
                str(executable),
            ]
            built = subprocess.run(command, cwd=ROOT, capture_output=True, text=True)
            assert built.returncode == 0, built.stdout + built.stderr
            result = subprocess.run([str(executable)], cwd=ROOT, capture_output=True, timeout=10)
            assert (result.returncode == 0) is expect_success, result.stderr.decode()


def test_actual_source_semantic_controls():
    _compile_and_run(_actual_body())


def test_post_modal_mask_mutant_is_rejected():
    body = _actual_body()
    anchor = "    if (alternate_selection != 0 && selected_item == 1)"
    assert body.count(anchor) == 1
    prefix, suffix = body.split(anchor)
    member = "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_"
    assert member in suffix
    suffix = suffix.replace(member, "human_mask")
    _compile_and_run(prefix + anchor + suffix, expect_success=False)


def test_initial_human_predicate_uses_only_the_human_byte():
    body = _actual_body()
    member = "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_"
    assert body.count(member) >= 1
    # The extracted-source harness sets human byte +0 to zero and adjacent
    # alive byte +1 to one for faction 8. A dword read wrongly admits it.
    _compile_and_run(body)
    damaged = body.replace(member, "GameDataStateStorage.fields_.lock_masks_.enable_mask_", 1)
    _compile_and_run(damaged, expect_success=False)


def test_manual_extra_cleanup_mutant_is_rejected():
    body = _actual_body()
    anchor = "    if (ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_ALIEN)) {\n        return;"
    assert body.count(anchor) == 1
    body = body.replace(anchor, "    if (ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_ALIEN)) {\n        popup.close();\n        return;", 1)
    _compile_and_run(body, expect_success=False)
