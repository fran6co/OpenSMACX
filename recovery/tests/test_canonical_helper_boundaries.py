"""Canonical out-of-line ownership for three pinned shared helpers."""

from pathlib import Path
import json
import hashlib
import os

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.pe_coff import PeImage, Span
import pytest

from match_recovery.source import read_file, read_text, write_body


ROOT = Path(__file__).resolve().parents[2]
HELPERS = (
    ("src/time.cpp", 0x50F440, 0x50F45E, "src/time.h", "src/time.cpp", "?stop_timers@@YAXXZ"),
    ("src/labels.cpp", 0x6169D0, 0x6169F7, "src/alpha.cpp", "src/labels.cpp", "?labels_shutdown@@YAXXZ"),
    ("src/map.cpp", 0x4F8090, 0x4F80CB, "src/map.h", "src/map.cpp", "?vector_dist@@YAHHH@Z"),
)


def test_original_read_rules_calls_labels_init_before_its_error_decision():
    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")
    ))
    if not image_path.is_file():
        pytest.skip(f"pinned image unavailable: {image_path}")
    raw = image_path.read_bytes()
    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert hashlib.sha256(raw).hexdigest() == pin
    image = PeImage(raw, pin)
    start, end = 0x5873C0, 0x58829C
    decoded = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(start, end)), start
    ))
    assert sum(item.size for item in decoded) == end - start
    instructions = {item.address: (item.mnemonic, item.op_str) for item in decoded}
    assert [item.address for item in decoded
            if item.mnemonic == "call" and item.op_str == "0x616a00"] == [0x5873DB]
    assert instructions[0x5873E0] == ("test", "eax, eax")
    assert instructions[0x5873E2] == ("je", "0x5873f0")
    assert instructions[0x5873F0] == ("call", "0x5fe270")


def test_helpers_keep_original_identity_and_have_canonical_cpp_bodies():
    for relative, address, end, _, body_path, symbol in HELPERS:
        path = ROOT / relative
        record, = (item for item in read_file(path) if item.address == address)
        assert record.name == symbol
        assert record.byte_exact
        assert record.body == body_path
        assert record.image_spans == ((address, end),)
        assert path.read_text().count("// ORIGINAL: 0x%08X" % address) == 1


def test_body_path_migrations_use_guarded_source_writer():
    for relative, address, _, old_body, new_body, _ in HELPERS:
        path = ROOT / relative
        current = path.read_bytes()
        current_record, = (item for item in read_file(path) if item.address == address)
        assert current_record.body == new_body

        marker = ("// ORIGINAL: 0x%08X" % address).encode()
        before, marked = current.split(marker, 1)
        marked = marked.replace(
            ("// body      " + new_body).encode(),
            ("// body      " + old_body).encode(),
            1,
        )
        legacy = before + marker + marked
        assert legacy != current
        old_record, = (item for item in read_text(
            legacy.decode("utf-8"), current_record.path
        ) if item.address == address)
        assert old_record.body == old_body
        assert write_body(legacy, old_record, new_body) == current


def test_definitions_and_callers_use_the_real_shared_helpers():
    time_cpp = (ROOT / "src/time.cpp").read_text()
    time_h = (ROOT / "src/time.h").read_text()
    alpha_cpp = (ROOT / "src/alpha.cpp").read_text()
    labels_cpp = (ROOT / "src/labels.cpp").read_text()
    alpha_h = (ROOT / "src/alpha.h").read_text()
    map_cpp = (ROOT / "src/map.cpp").read_text()
    map_h = (ROOT / "src/map.h").read_text()
    game_cpp = (ROOT / "src/game.cpp").read_text()
    probe_cpp = (ROOT / "src/probe.cpp").read_text()

    assert "void __cdecl stop_timers();" in time_h
    assert "MEASURED inline void __cdecl stop_timers()" not in time_h
    assert time_cpp.count("void __cdecl stop_timers() {") == 1
    stop_body = time_cpp.split("void __cdecl stop_timers() {", 1)[1].split("\n}", 1)[0]
    assert "Blink2Timer.close();" in stop_body
    assert "BlinkTimer.close();" in stop_body
    assert "LineTimer.close();" in stop_body
    assert "TurnTimer.close();" not in stop_body

    assert "void __cdecl labels_shutdown();" in alpha_h
    assert "MEASURED inline void __cdecl labels_shutdown()" not in alpha_h
    assert labels_cpp.count("void __cdecl labels_shutdown() {") == 1
    assert labels_cpp.count("BOOL __cdecl labels_init() {") == 1
    assert labels_cpp.index("void __cdecl labels_shutdown() {") < labels_cpp.index(
        "BOOL __cdecl labels_init() {"
    )
    assert "void __cdecl labels_shutdown() {" not in alpha_cpp
    assert "BOOL __cdecl labels_init() {" not in alpha_cpp
    assert "Label Labels;" not in alpha_cpp
    assert "Label Labels;                                       // 0x009B90F8" in labels_cpp
    assert "RECOVERY_OBJECT: v1 0x009B90F8 ?Labels@@3ULabel@@A" in labels_cpp

    assert "int __cdecl vector_dist(int x_distance, int y_distance);" in map_h
    assert "MEASURED inline int __cdecl vector_dist(int x_distance" not in map_h
    assert map_cpp.count("int __cdecl vector_dist(int x_distance, int y_distance) {") == 1
    assert map_cpp.index("int __cdecl vector_dist(int x_distance, int y_distance) {") < map_cpp.index(
        "int __cdecl vector_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {"
    )
    assert "vector_dist_call" not in probe_cpp
    assert "vector_dist(dx, dy);" in probe_cpp
    assert "stop_timers_call();" not in game_cpp
    assert "labels_shutdown_call();" not in game_cpp
    assert "stop_timers();" in game_cpp
    assert "labels_shutdown();" in game_cpp


def test_labels_owner_is_registered_with_its_scoped_measurement_flags():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    project = json.loads((ROOT / "recovery/project.json").read_text())
    assert "src/labels.cpp" in cmake
    assert project["measurement"]["unit_flag_sets"]["src/labels.cpp"]["flags"] == [
        "/c", "/O2", "/Gy", "/GR-", "/GX", "/Ob2"
    ]
    alpha_cpp = (ROOT / "src/alpha.cpp").read_text()
    assert "BODY IN alpha.h, as `MEASURED inline`: the image writes it out at" in alpha_cpp
