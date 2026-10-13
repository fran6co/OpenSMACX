"""Actual-source C++98 behavior proposal for two AlphaMovie/MCI seed owners.

The tests extract method definitions from tracked product owners, compile and
run them against deliberately small host API fixtures, and damage the extracted
body to ensure each control catches a real behavior change. They do not read
the neighboring ignored proposal fragments.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE_SIGNATURES = {
    "movie_unk1": ("src/alphamovie.cpp", r"^\s*void\s+AlphaMovie::UNK1\s*\(\s*\)"),
    "mci_status": ("src/alphamovie.cpp", r"^\s*int\s+MCIVideo::status\s*\(\s*\)"),
}
PINNED_IDENTITIES = {
    "movie_unk1": ("0x00404200", "entity-cebc9283d8177ee90234540955d8c1c2e94476190015b96682ae5e14f0955d27"),
    "mci_status": ("0x006002F0", "entity-d6d04911bac6dc3f3a3bdc02f69459ba675742c7ffd6bfdef7a297301a058480"),
}


def _extract_definition(source: str, signature: str) -> str:
    match = re.search(signature, source, re.MULTILINE)
    assert match, "tracked product definition missing: " + signature
    opening = source.find("{", match.end())
    assert opening >= 0
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        char = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                i += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                i += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start() : i + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
        else:
            if char == "\\":
                i += 2
                continue
            if (state == "string" and char == '"') or (
                state == "char" and char == "'"
            ):
                state = "code"
        i += 1
    raise AssertionError("unterminated definition: " + signature)


@pytest.fixture(scope="module")
def bodies() -> dict[str, str]:
    source = (ROOT / "src/alphamovie.cpp").read_text(encoding="utf-8")
    return {
        name: _extract_definition(source, signature)
        for name, (_, signature) in SOURCE_SIGNATURES.items()
    }


def test_tracked_headers_witness_the_real_receivers_and_members():
    alpha = (ROOT / "src/alphamovie.h").read_text(encoding="utf-8")
    win = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert re.search(r"class\s+AlphaMovie\s*:\s*public\s+GraphicWin", alpha)
    assert re.search(r"void\s+UNK1\s*\(\s*\)\s*;", alpha)
    assert re.search(r"class\s+MCIVideo", alpha)
    assert re.search(r"int\s+status\s*\(\s*\)\s*;", alpha)
    assert re.search(r"uint16_t\s+mciId_\s*;\s*//\s*0x0*4", alpha)
    assert re.search(r"virtual\s+void\s+release_modal\s*\(\s*\)\s*;[^\n]*slot 58", win)


MOVIE_HOST = r'''#include <cassert>
struct Buffer {};
struct Win {
  int base_calls;
  Win() : base_calls(0) {}
  virtual ~Win() {}
  virtual void release_modal() { ++base_calls; }
};
struct GraphicWin : Win, Buffer {};
struct AlphaMovie : GraphicWin { void UNK1(); };
struct TestMovie : AlphaMovie {
  int dispatched_calls;
  TestMovie() : dispatched_calls(0) {}
  virtual void release_modal() { ++dispatched_calls; }
};
// INSERT_MOVIE_UNK1
int main() {
  TestMovie movie;
  movie.UNK1();
  assert(movie.dispatched_calls == 1);
  assert(movie.base_calls == 0);
}
'''


STATUS_HOST = r'''#include <cassert>
#include <stdint.h>
#include <stddef.h>
typedef uint32_t DWORD;
typedef uint32_t DWORD_PTR; // x86 SDK layout; body call is widened for LP64 below.
typedef uint16_t MCIDEVICEID;
typedef uint32_t MCIERROR;
enum { MCI_STATUS=0x814, MCI_STATUS_ITEM=0x100 };
// Host-only x86-width model of the real digitalv.h SDK members.
// Native VC6 verification compiles against the actual Windows SDK declaration.
typedef uint32_t LPTSTR;
struct MCI_DGV_STATUS_PARMS {
  DWORD_PTR dwCallback;
  DWORD_PTR dwReturn;
  DWORD dwItem;
  DWORD dwTrack;
  LPTSTR lpstrDrive;
  DWORD dwReference;
};
typedef char mci_status_parms_must_be_x86_size[
  sizeof(MCI_DGV_STATUS_PARMS) == 0x18 ? 1 : -1];
typedef char mci_status_dwreturn_offset[
  offsetof(MCI_DGV_STATUS_PARMS, dwReturn) == 4 ? 1 : -1];
typedef char mci_status_dwitem_offset[
  offsetof(MCI_DGV_STATUS_PARMS, dwItem) == 8 ? 1 : -1];
typedef char mci_status_drive_offset[
  offsetof(MCI_DGV_STATUS_PARMS, lpstrDrive) == 16 ? 1 : -1];
MCIDEVICEID observed_device=0;
DWORD observed_command=0, observed_flags=0;
MCIERROR api_result=0;
DWORD status_result=0;
MCIERROR mciSendCommandA(MCIDEVICEID device, DWORD command,
                         DWORD flags, uintptr_t param) {
  observed_device=device; observed_command=command; observed_flags=flags;
  MCI_DGV_STATUS_PARMS *p=reinterpret_cast<MCI_DGV_STATUS_PARMS *>(param);
  assert(p->dwItem==2);
  p->dwReturn=status_result;
  return api_result;
}
struct MCIVideo { uint32_t field_0_; uint16_t mciId_; int status(); };
// INSERT_MCI_STATUS
int main() {
  MCIVideo video; video.mciId_=0x1234abcdu;
  api_result=91; status_result=0x76543210u;
  assert(video.status()==static_cast<int>(status_result));
  assert(observed_device==0xabcdu);
  assert(observed_command==MCI_STATUS && observed_flags==MCI_STATUS_ITEM);
}
'''


FIXTURES = {
    "movie": (MOVIE_HOST, {"movie_unk1": "// INSERT_MOVIE_UNK1"}),
    "status": (STATUS_HOST, {"mci_status": "// INSERT_MCI_STATUS"}),
}


def _fixture_source(name: str, bodies: dict[str, str], mutant=None) -> str:
    template, markers = FIXTURES[name]
    source = template
    for body_name, marker in markers.items():
        body = bodies[body_name]
        if mutant and mutant[0] == body_name:
            before, after = mutant[1], mutant[2]
            assert before in body, (body_name, before)
            body = body.replace(before, after, 1)
        if body_name == "mci_status":
            # x86 DWORD_PTR preserves the real local structure layout, but a
            # host process needs a pointer-width fourth API argument.
            body = body.replace(
                "reinterpret_cast<DWORD_PTR>(&params)",
                "reinterpret_cast<uintptr_t>(&params)",
            )
        assert marker in source
        source = source.replace(marker, body, 1)
    return source


def _compile_and_run(tmp_path: Path, compiler: str, source: str, label: str):
    executable = tmp_path / (label + "-" + Path(compiler).name)
    cpp = tmp_path / (label + ".cpp")
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic", str(cpp), "-o", str(executable)],
        text=True, capture_output=True,
    )
    return built, executable


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("fixture_name", tuple(FIXTURES))
def test_actual_owner_bodies_compile_and_execute_cxx98(
    tmp_path, bodies, compiler_name, fixture_name
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is not installed")
    source = _fixture_source(fixture_name, bodies)
    built, executable = _compile_and_run(tmp_path, compiler, source, fixture_name)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(
    "fixture_name,body_name,before,after",
    (
        ("movie", "movie_unk1", "release_modal();", "Win::release_modal();"),
        ("status", "mci_status", "params.dwItem = 2;", "params.dwItem = 1;"),
        ("status", "mci_status", "const MCIDEVICEID device_id = mciId_;",
         "const MCIDEVICEID device_id = 0;"),
        ("status", "mci_status", "return static_cast<int>(params.dwReturn);", "return 0;"),
        (
            "status", "mci_status",
            "mciSendCommandA(device_id, MCI_STATUS,",
            "return mciSendCommandA(device_id, MCI_STATUS,",
        ),
    ),
)
def test_behavior_controls_reject_damaged_extracted_body(
    tmp_path, bodies, compiler_name, fixture_name, body_name, before, after
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is not installed")
    source = _fixture_source(fixture_name, bodies, (body_name, before, after))
    built, executable = _compile_and_run(tmp_path, compiler, source, "mutant-" + body_name)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    assert ran.returncode != 0, "host control accepted damaged body " + body_name
