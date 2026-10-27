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
    # Saved process rows are now a disjoint NetDaemon member rather than an
    # overlapping NetDaemonSaveStorageView projection. The file-index owner is
    # also independent storage and is not part of this save-copy fixture.
    assert "NetDaemonSaveStorageView" not in daemon
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
"""
    return "\n".join((lock_projection, live, saved, checks))


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


@pytest.mark.parametrize("mutation", ["wrong_word", "wrong_api", "recapture", "clear_high_bit"])
@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_tick_storage_compile_success_mutations_fail(tmp_path, mutation, compiler):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    body = extract_definition((ROOT / "src/game.cpp").read_text(),
                              "int __cdecl save_daemon(char *save_path)",
                              expect_semicolon=False)
    anchor = "fwrite(&save_tick, 4, 1, file)"
    assert body.count(anchor) == 1
    if mutation == "wrong_word":
        body = body.replace(anchor, "fwrite(&GameDataPlayerCountSnapshot, 4, 1, file)")
    elif mutation == "recapture":
        body = body.replace(anchor, "(save_tick = timeGetTime(), " + anchor + ")")
    elif mutation == "wrong_api":
        original = "DWORD save_tick = timeGetTime();"
        assert body.count(original) == 1
        body = body.replace(original, "DWORD save_tick = GetTickCount();")
    else:
        original = "DWORD save_tick = timeGetTime();"
        assert body.count(original) == 1
        body = body.replace(original, "DWORD save_tick = timeGetTime() & 0x7FFFFFFFu;")
    source = FIXTURE.read_text().replace("// ACTUAL_STORAGE_DECLARATIONS",
                                         actual_storage_declarations())
    source = source.replace("/* ACTUAL_GAME_DATA_STATE_HEADER */",
                            f'#include "{ROOT / "src/game_data_state_window.h"}"')
    source = source.replace("// ACTUAL_SAVE_DAEMON_BODY", body)
    cpp, binary = tmp_path / "tick_mutation.cpp", tmp_path / "tick_mutation"
    cpp.write_text(source)
    built = subprocess.run([executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(binary)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode != 0, "compile-success mutation escaped the actual-source control"


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
    owner_clear = "deletion_list_.current_owner_ = 0;"
    assert ctor.count(owner_clear) == 1
    assert ctor.index(owner_clear) < ctor.index("for (int32_t m = 0;")
    assert "deletion_list_.records_[m].acknowledgements_" in ctor
    assert "primary_state_.leading_words_[0] = 0;" in ctor
    assert "for (int32_t m = 0; m < 0x18; ++m)" in ctor
    assert "process_slots_" not in ctor
