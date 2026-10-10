"""Compile and exercise the actual SaveDaemon body and storage declarations."""
from pathlib import Path
import shutil
import subprocess

import pytest


def repo_root():
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/game.cpp").is_file():
            return candidate
    raise AssertionError("cannot locate OpenSMACX source root")


ROOT = repo_root()
FIXTURE = Path(__file__).with_name("save_daemon_host.cpp")


def extract_definition(source, signature, expect_semicolon=True):
    start = source.find(signature)
    assert start >= 0, f"missing actual product definition: {signature}"
    opening = source.find("{", start)
    depth = 0
    for cursor in range(opening, len(source)):
        if source[cursor] == "{":
            depth += 1
        elif source[cursor] == "}":
            depth -= 1
            if depth == 0:
                end = cursor + 1
                while end < len(source) and source[end].isspace():
                    end += 1
                if expect_semicolon:
                    assert source[end] == ";"
                    end += 1
                return source[start:end]
    raise AssertionError(f"unterminated product declaration: {signature}")


def actual_storage_declarations():
    alpha = (ROOT / "src/alphanet.h").read_text()
    daemon = (ROOT / "src/netdaemon.h").read_text()
    live = extract_definition(alpha, "struct AlphaNetProcessSlot {")
    saved = extract_definition(daemon, "struct NetDaemonSaveProcessSlot {")
    view = extract_definition(daemon, "struct NetDaemonSaveStorageView {")
    assert "void *file_index_records_;" in view
    # The host has no 32-bit libc headers; project the target pointer word to
    # uint32_t only in this compiled host view. The source declaration itself
    # is checked above and the native build checks its real layout.
    view = view.replace("void *file_index_records_;", "uint32_t file_index_records_;")
    lock_projection = extract_definition(daemon, "struct LockStateEntry {")
    assert "int *vehicle_selection_follow;" in lock_projection
    lock_projection = lock_projection.replace(
        "int *vehicle_selection_follow;", "uint32_t vehicle_selection_follow;"
    )
    checks = r"""
static_assert(sizeof(LockStateEntry) == 0x3C, "32-bit lock projection");
static_assert(sizeof(AlphaNetProcessSlot) == 0x19C, "live slot stride");
static_assert(sizeof(NetDaemonSaveProcessSlot) == sizeof(AlphaNetProcessSlot),
              "save/live slot stride");
static_assert(sizeof(NetDaemonSaveProcessSlot) * 8 == 0xCE0,
              "eight-slot transfer extent");
static_assert(offsetof(NetDaemonSaveStorageView, file_index_records_) == 0xB4,
              "file-index pointer offset");
static_assert(offsetof(NetDaemonSaveStorageView, process_slots_) == 0xBC,
              "snapshot offset");
"""
    return "\n".join((lock_projection, live, saved, view, checks))


def test_actual_product_body_and_storage_under_cpp98(tmp_path):
    body = extract_definition((ROOT / "src/game.cpp").read_text(),
                              "int __cdecl save_daemon(char *save_path)",
                              expect_semicolon=False)
    fixture = FIXTURE.read_text()
    source = fixture.replace("// ACTUAL_STORAGE_DECLARATIONS",
                             actual_storage_declarations())
    state_header = ROOT / "src/game_data_state_window.h"
    state_marker = "/* ACTUAL_GAME_DATA_STATE_HEADER */"
    assert source.count(state_marker) == 1
    source = source.replace(state_marker, f'#include "{state_header}"')
    assert source.count("// ACTUAL_SAVE_DAEMON_BODY") == 1
    source = source.replace("// ACTUAL_SAVE_DAEMON_BODY", body)
    cpp = tmp_path / "save_daemon_actual.cpp"
    cpp.write_text(source)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    for compiler in compilers:
        binary = tmp_path / ("save-daemon-" + Path(compiler).name)
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(binary)], check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)


def test_seed_identity_and_history_are_owned_by_real_definition():
    owner = (ROOT / "src/game.cpp").read_text()
    assert not (ROOT / "src/unrecovered/005a94f0.cpp").exists()
    assert "// ORIGINAL: 0x005A94F0 ?save_daemon@@YAHPAD@Z 0x005A94F0-0x005A96C3" in owner
    assert "fclose`/`remove`" in owner and "return-value accumulator" in owner
    assert "int __cdecl save_daemon(char *save_path);" in (ROOT / "src/game.h").read_text()


def test_constructor_partial_alias_initialization_is_preserved():
    source = (ROOT / "src/netdaemon.cpp").read_text()
    ctor = extract_definition(source, "NetDaemon::NetDaemon() : AlphaNet()",
                              expect_semicolon=False)
    owner_clear = "lock_state_[0].leading_words_[0] = 0;"
    assert ctor.count(owner_clear) == 1
    assert ctor.index(owner_clear) < ctor.index("for (int32_t m = 0;")
    assert "lock_state_[m].leading_words_[0] = 0;" not in ctor
    assert "for (int32_t m = 0; m < 0x18; ++m)" in ctor
    assert "process_slots_" not in ctor
