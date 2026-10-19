"""Actual battle reset and timer callback share the original hold cell.

The host control extracts both production bodies and the one production global
definition. Pinned instructions independently establish the write/read sites;
this is not native object/ABI, linked-provider, or game-runtime proof.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import function_line
from match_recovery.source.reader import _extract_forward_text


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _image_path() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured is not None:
        return Path(configured)
    for parent in (ROOT, *ROOT.parents):
        candidate = parent / ".opensmacx/game/terranx_original.exe"
        if candidate.is_file():
            return candidate
    raise FileNotFoundError("pinned original image not found from worktree/common root")


def _body(path: Path, name: str) -> str:
    text = path.read_text(encoding="utf-8")
    line = function_line(text, name)
    return _extract_forward_text(text.splitlines(), line, name).strip()


def _timer_hold_definition() -> str:
    text = (ROOT / "src/time.cpp").read_text(encoding="utf-8")
    matches = re.findall(r"(?m)^int TimerHoldFlag;\s*(?://[^\n]*)?$", text)
    assert len(matches) == 1
    return matches[0]


def _facts(image: PeImage, start: int, end: int):
    code = image.read(Span(start, end))
    instructions = tuple(Cs(CS_ARCH_X86, CS_MODE_32).disasm(code, start))
    assert sum(item.size for item in instructions) == end - start
    return {item.address: (item.mnemonic, item.op_str) for item in instructions}


def _pinned_image(path: Path | None = None) -> PeImage:
    data = (path if path is not None else _image_path()).read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN, "original image does not match the pinned SHA256"
    return PeImage(data, PIN)


def test_pinned_cells_have_independent_battle_and_timer_producers_consumers():
    image = _pinned_image()
    battle = _facts(image, 0x00501D30, 0x00501D47)
    assert battle[0x00501D3C] == ("mov", "dword ptr [0x91561c], eax")
    assert battle[0x00501D41] == ("mov", "dword ptr [0x915620], eax")

    planet = _facts(image, 0x00500B60, 0x005010BB)
    assert planet[0x00500EE1] == ("mov", "dword ptr [0x91561c], 1")
    assert planet[0x0050103A] == ("mov", "dword ptr [0x91561c], 0")
    kill = _facts(image, 0x005C0B00, 0x005C0CB0)
    assert kill[0x005C0C35] == ("mov", "eax, dword ptr [0x91561c]")

    timer = _facts(image, 0x0050EA40, 0x0050EE25)
    assert timer[0x0050EA55] == ("mov", "eax, dword ptr [0x915620]")
    alt = _facts(image, 0x00591290, 0x00591894)
    assert alt[0x005914B1] == ("mov", "eax, dword ptr [0x91561c]")
    assert alt[0x00591651] == ("mov", "eax, dword ptr [0x91561c]")

    battle_compute = _facts(image, 0x00501DA0, 0x005044C4)
    assert battle_compute[0x00501DC0] == ("mov", "dword ptr [0x91561c], eax")
    assert battle_compute[0x00501DC5] == ("mov", "dword ptr [0x915620], eax")
    boom = _facts(image, 0x00504AA0, 0x00505D1B)
    assert boom[0x005054FD] == ("mov", "dword ptr [0x915620], 1")
    assert boom[0x00505D02] == ("mov", "dword ptr [0x915620], 0")


def test_pinned_image_refuses_missing_corrupt_and_bad_override(tmp_path, monkeypatch):
    missing = tmp_path / "missing.exe"
    with pytest.raises(FileNotFoundError):
        _pinned_image(missing)

    corrupt = tmp_path / "corrupt.exe"
    corrupt.write_bytes(b"not the pinned executable")
    with pytest.raises(AssertionError, match="pinned SHA256"):
        _pinned_image(corrupt)

    monkeypatch.setenv("OPENSMACX_IMAGE", str(missing))
    assert _image_path() == missing
    with pytest.raises(FileNotFoundError):
        _pinned_image()


def _compile_and_run(tmp_path: Path, battle_init: str, compiler: str) -> int:
    from test_blink_timer_actual_source import FIXTURE as timer_fixture

    fixture = timer_fixture.replace("int TimerHoldFlag;", _timer_hold_definition(), 1)
    assert fixture.count("TimerHoldFlag;") == 1
    source = fixture + r'''
#define MEASURED
uint32_t VehBattleModCount[2];
int VehBattleUnkTgl;
''' + "\n" + battle_init + "\n" + _body(ROOT / "src/time.cpp", "blink_timer")
    source += r'''
int main() {
    TimerHoldFlag = 1;
    TutWinBaseWindow.visible = 1;
    blink_timer(7);
    if (TutorialWindow.arrows != 0) return 1;

    VehBattleModCount[0] = 7;
    VehBattleModCount[1] = 9;
    VehBattleUnkTgl = 1;
    battle_init();
    if (VehBattleModCount[0] != 0 || VehBattleModCount[1] != 0
        || VehBattleUnkTgl != 0 || TimerHoldFlag != 0) return 2;

    blink_timer(7);
    return TutorialWindow.arrows == 1 ? 0 : 3;
}
'''
    cpp, exe = tmp_path / "actual_shared_state.cpp", tmp_path / "actual_shared_state"
    cpp.write_text(source)
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=45)
    assert result.returncode == 0, result.stdout + result.stderr
    return subprocess.run([str(exe)], check=False, timeout=15).returncode


def test_actual_battle_init_and_timer_callback_share_one_storage(tmp_path):
    header = (ROOT / "src/veh.h").read_text(encoding="utf-8")
    owner = (ROOT / "src/veh.cpp").read_text(encoding="utf-8")
    annotation = "RECOVERY_OBJECT: v1 0x0091561C ?VehBattleUnkTgl@@3HA"
    assert annotation not in header
    assert owner.count(annotation) == 1
    assert owner.index(annotation) < owner.index("BOOL VehBattleUnkTgl;")

    battle_init = _body(ROOT / "src/veh.h", "battle_init")
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers
    for compiler in compilers:
        assert _compile_and_run(tmp_path, battle_init, compiler) == 0

        removed = battle_init.replace("    TimerHoldFlag = 0;", "", 1)
        assert removed != battle_init
        assert _compile_and_run(tmp_path, removed, compiler) != 0

        redirected = battle_init.replace(
            "    TimerHoldFlag = 0;", "    VehBattleUnkTgl = 0;", 1)
        assert redirected != battle_init
        assert _compile_and_run(tmp_path, redirected, compiler) != 0
