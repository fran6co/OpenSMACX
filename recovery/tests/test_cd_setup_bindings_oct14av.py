"""Host controls for the pinned DirectX setup tail in Win::cd_check.

The no-CD scan has an intentionally uninitialized original value. These
controls start at the found-media tail, so they do not assign a deterministic
meaning to the original no-CD behavior or model the game runtime/SDK provider.
"""

import hashlib
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
WIN_CPP = ROOT / "src/win.cpp"
# Frozen after the bounded cd_check correction; this rejects unrelated source
# edits while allowing the test harness itself to evolve.
WIN_CPP_SHA256 = "c61f344464e719615bb6a93a5689c8f3a8a7e4bb2bd4362aac5c954d2273ca86"
CD_CHECK = "int __cdecl cd_check()"
TAIL_START = "HMODULE module = LoadLibraryA(root_path);"
TAIL_END = "    return 1;"


def _extract_function(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError(f"unterminated source function: {signature}")


def _setup_tail(source: str, mutation: str | None = None) -> str:
    body = _extract_function(source, CD_CHECK)
    start = body.index(TAIL_START)
    end = body.index(TAIL_END, start) + len(TAIL_END)
    tail = body[start:end]
    if mutation == "wrong_parent_guard":
        old = "if (BasePopDefaultField322C != 0)"
        assert tail.count(old) == 1
        tail = tail.replace(old, "if (HandleMain != 0)", 1)
    elif mutation == "wrong_hwnd_argument":
        old = "(reinterpret_cast<setup_a_fn>(setup_a))(HandleMain, 0, 0x10018);"
        assert tail.count(old) == 1
        tail = tail.replace(old,
                            "(reinterpret_cast<setup_a_fn>(setup_a))(nullptr, 0, 0x10018);",
                            1)
    return tail


HOST_PREFIX = r"""
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define __stdcall
struct HWND__ { int tag; };
typedef HWND__ *HWND;
typedef void *HMODULE;
typedef void (*FARPROC)();
typedef unsigned long DWORD;

struct ParentWindow {};
static ParentWindow *BasePopDefaultField322C;
static HWND HandleMain;
static int version_result;
static int popup_result;
static int setup_result;
static int missing_export;
static int load_fails;
static int setup_calls;
static HWND setup_hwnd;
static int setup_arg2;
static int setup_flags;
static char current_directory[260] = "C:\\game";
static int events[24];
static int event_count;

static int fail(const char *message) {
    std::fprintf(stderr, "CONTROL_FAIL:%s\n", message);
    return 97;
}
static void event(int value) {
    if (event_count >= 24) std::exit(fail("event-overflow"));
    events[event_count++] = value;
}
static int __stdcall fake_setup_a(HWND hwnd, int arg2, int flags) {
    event(10);
    ++setup_calls;
    setup_hwnd = hwnd;
    setup_arg2 = arg2;
    setup_flags = flags;
    return setup_result;
}
static int __stdcall fake_setup_version(int *version, int arg) {
    event(4);
    if (arg != 0) std::exit(fail("version-zero-argument"));
    *version = 0x12345678;
    return version_result;
}
static HMODULE LoadLibraryA(const char *path) {
    event(1);
    if (std::strcmp(path, "A:\\directx\\dsetup.dll") != 0)
        std::exit(fail("load-path"));
    return load_fails ? 0 : reinterpret_cast<HMODULE>(0x1234);
}
static FARPROC GetProcAddress(HMODULE module, const char *name) {
    if (module != reinterpret_cast<HMODULE>(0x1234))
        std::exit(fail("provider-module"));
    if (std::strcmp(name, "DirectXSetupA") == 0) {
        event(2);
        return missing_export == 1 ? 0 : reinterpret_cast<FARPROC>(fake_setup_a);
    }
    if (std::strcmp(name, "DirectXSetupGetVersion") == 0) {
        event(3);
        return missing_export == 2 ? 0 : reinterpret_cast<FARPROC>(fake_setup_version);
    }
    std::exit(fail("unexpected-export"));
    return 0;
}
static int pop_caption_title(char *, char *title, int, void *) {
    event(5);
    if (std::strcmp(title, "DIRECTXNOVERSION") == 0) return popup_result;
    if (std::strcmp(title, "DIRECTXVERSION") == 0) return popup_result;
    std::exit(fail("unexpected-popup"));
    return 0;
}
static void parse_num(int, int value) {
    event(6);
    if (value != 0x5678) std::exit(fail("parsed-version"));
}
static DWORD GetCurrentDirectoryA(DWORD size, char *buffer) {
    event(8);
    if (size != 260) std::exit(fail("saved-directory-size"));
    std::strcpy(buffer, current_directory);
    return static_cast<DWORD>(std::strlen(buffer));
}
static int SetCurrentDirectoryA(const char *path) {
    event(9);
    if (std::strcmp(path, "A:\\") == 0) {
        std::strcpy(current_directory, path);
    } else if (std::strcmp(path, "C:\\game") == 0) {
        std::strcpy(current_directory, path);
        event(12);
    } else {
        std::exit(fail("working-directory-path"));
    }
    return 1;
}
static int FreeLibrary(HMODULE module) {
    event(11);
    if (module != reinterpret_cast<HMODULE>(0x1234))
        std::exit(fail("free-module"));
    return 1;
}
"""


def _host_source(tail: str) -> str:
    return HOST_PREFIX + r"""
static int run_tail(char root_path[256]) {
    int version;
    int response;
    char saved_dir[260];
""" + tail + r"""
}

static void reset() {
    event_count = 0;
    setup_calls = 0;
    setup_hwnd = reinterpret_cast<HWND>(0x1);
    setup_arg2 = -1;
    setup_flags = -1;
    std::strcpy(current_directory, "C:\\game");
    version_result = 1;
    popup_result = 0;
    setup_result = -1;
    missing_export = 0;
    load_fails = 0;
}
static int expect_events(const int *expected, int count, const char *label) {
    if (event_count != count) return fail(label);
    for (int i = 0; i < count; ++i) {
        if (events[i] != expected[i]) return fail(label);
    }
    return 0;
}
static int run_combo(int has_parent, int has_hwnd) {
    reset();
    ParentWindow parent;
    HWND__ main_window = {42};
    BasePopDefaultField322C = has_parent ? &parent : 0;
    HandleMain = has_hwnd ? &main_window : 0;
    char root_path[256] = "A:\\directx\\dsetup.dll";
    const int result = run_tail(root_path);
    if (result != 1) return fail("setup-result-ignored-and-success");
    if (setup_calls != 1) return fail("setup-called-once");
    if (setup_hwnd != (has_parent ? HandleMain : 0))
        return fail("parent-guard-selects-hwnd");
    if (setup_arg2 != 0 || setup_flags != 0x10018)
        return fail("setup-zero-and-flags-arguments");
    if (std::strcmp(current_directory, "C:\\game") != 0)
        return fail("directory-restored");
    const int expected[] = {1, 2, 3, 4, 6, 5, 8, 9, 10, 11, 9, 12};
    return expect_events(expected, 12, "setup-and-cleanup-order");
}

static int run_failure(int which) {
    reset();
    ParentWindow parent;
    HWND__ main_window = {42};
    BasePopDefaultField322C = &parent;
    HandleMain = &main_window;
    char root_path[256] = "A:\\directx\\dsetup.dll";
    if (which == 1) load_fails = 1;
    if (which == 2) missing_export = 1;
    if (which == 3) missing_export = 2;
    if (which == 4) { version_result = 0; popup_result = -1; }
    if (which == 5) { version_result = 1; popup_result = 1; }
    const int result = run_tail(root_path);
    if (result != 0 || setup_calls != 0) return fail("failure-return-before-setup");
    if (which <= 3 && event_count == 0) return fail("failure-path-events");
    for (int i = 0; i < event_count; ++i) {
        if (events[i] == 11 || events[i] == 12)
            return fail("original-early-return-leaves-cleanup-unreached");
    }
    return 0;
}

int main() {
    for (int parent = 0; parent < 2; ++parent) {
        for (int hwnd = 0; hwnd < 2; ++hwnd) {
            const int result = run_combo(parent, hwnd);
            if (result != 0) return result;
        }
    }
    for (int failure = 1; failure <= 5; ++failure) {
        const int result = run_failure(failure);
        if (result != 0) return result;
    }
    return 0;
}
"""


def _compile_run(source: str, compiler: str, tmp_path: Path, stem: str):
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-no-pie", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


def test_cd_check_source_diff_is_limited_to_the_reviewed_binding_correction():
    source = WIN_CPP.read_text(encoding="utf-8")
    assert hashlib.sha256(source.encode("utf-8")).hexdigest() == WIN_CPP_SHA256
    rows = {row.address: row for row in read_text(source, WIN_CPP)}
    row = rows[0x628F30]
    assert row.symbol == "?cd_check@@YAHXZ"
    assert row.image_spans == ((0x628F30, 0x6290E0),)
    assert "ExpansionEnabled" not in _extract_function(source, CD_CHECK)
    assert "typedef int(__stdcall *setup_a_fn)(HWND, int, int);" in source


def test_setup_tail_keeps_four_parent_and_handle_combinations_and_failures(
    tmp_path: Path,
):
    source = WIN_CPP.read_text(encoding="utf-8")
    tail = _setup_tail(source)
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        if compiler is None:
            continue
        result = _compile_run(_host_source(tail), compiler, tmp_path,
                              f"cd_setup_{compiler_name.replace('+', 'p')}")
        assert result.returncode == 0, result.stdout + result.stderr
    if shutil.which("g++") is None and shutil.which("clang++") is None:
        pytest.skip("neither g++ nor clang++ is available")


@pytest.mark.parametrize(("mutation", "diagnostic"), (
    ("wrong_parent_guard", "parent-guard-selects-hwnd"),
    ("wrong_hwnd_argument", "parent-guard-selects-hwnd"),
))
@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compile_success_binding_mutations_are_observable(
    mutation: str, diagnostic: str, compiler_name: str, tmp_path: Path,
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = WIN_CPP.read_text(encoding="utf-8")
    result = _compile_run(_host_source(_setup_tail(source, mutation)), compiler,
                          tmp_path, f"cd_setup_{mutation}_{compiler_name.replace('+', 'p')}")
    assert result.returncode == 97, result.stdout + result.stderr
    assert f"CONTROL_FAIL:{diagnostic}" in result.stderr
