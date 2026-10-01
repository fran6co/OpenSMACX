"""Actual-source input/timed-writer checks; not application runtime proof."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def definition(text, signature):
    match = re.search(signature + r"\s*\{", text)
    assert match
    opening = text.index("{", match.start())
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[match.start():end + 1]
    raise AssertionError("unterminated product definition")


def source():
    methods = ROOT / "src/uv2player.cpp"
    callback = methods
    assert {0x4BEA30, 0x4BF390, 0x4BEDC0} <= {r.address for r in read_file(methods)}
    assert sum(r.address == 0x4BEE60 for r in read_file(callback)) == 1
    return "\n\n".join((
        definition((ROOT / "src/buffer.h").read_text(),
                   r"void set_write_raw_callback\(BufferWriteRawCallback callback\)"),
        definition(methods.read_text(), r"UV2Player::UV2Player\(\)"),
        definition(methods.read_text(), r"int __cdecl UV2Player::on_key\(\)"),
        definition(methods.read_text(), r"int UV2Player::write_timed\([^)]*\)"),
        definition(callback.read_text(),
                   r'extern "C" int __cdecl uv2_player_writer_redirect\([^)]*\)'),
    ))


def run(tmp_path, product, scenario):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host compiler unavailable")
    setter, rest = product.split("\n\n", 1)
    template = Path(__file__).with_name("uv2player_owner_harness.cpp").read_text()
    cpp, exe = tmp_path / "owner.cpp", tmp_path / "owner"
    cpp.write_text(template.replace("/* SETTER */", setter).replace("/* PRODUCT */", rest))
    subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                    str(cpp), "-o", str(exe)], check=True, capture_output=True, text=True)
    return subprocess.run([str(exe), scenario], capture_output=True, text=True,
                          timeout=15, env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0",
                                              UBSAN_OPTIONS="halt_on_error=1"))


@pytest.mark.parametrize("scenario", ["constructor", "keys", "member", "callback", "clock"])
def test_actual_input_and_timed_writer(tmp_path, scenario):
    result = run(tmp_path, source(), scenario)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("old,new,scenario", [
    ("field_10C_ = 0;", "field_10C_ = 7;", "constructor"),
    ("!GetMessageA(&message, 0, 0, 0)", "GetMessageA(&message, 0, 0, 0) <= 0", "keys"),
    ("buffer_.set_write_raw_callback(0);", "buffer_.set_write_raw_callback(&uv2_player_writer_redirect);", "member"),
    ("instance->write_delay_", "100u", "clock"),
    ("buffer.set_write_raw_callback(&uv2_player_writer_redirect);", "buffer.set_write_raw_callback(0);", "callback"),
])
def test_damage_controls(tmp_path, old, new, scenario):
    product = source()
    assert old in product
    result = run(tmp_path, product.replace(old, new, 1), scenario)
    assert result.returncode != 0, "deliberate damage escaped the actual-source control"
