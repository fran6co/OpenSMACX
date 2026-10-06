"""Installed-source regression for DesignWin's shared prototype scratch row.

This is deliberately source-only: it extracts definitions from src/designwin.cpp
and never imports or falls back to a build/recovery proposal.
"""

import re
import shutil
import subprocess
from pathlib import Path


SIGNATURES = {
    "unk3": "void DesignWin::UNK3()",
    "vehicle": "void DesignWin::draw_vehicle(int redraw)",
    "flash": "void DesignWin::draw_flash()",
    "special": "void DesignWin::select_special(int which)",
}


def _project_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/designwin.cpp").is_file():
            return parent
    raise AssertionError("cannot find project root containing src/designwin.cpp")


def _definition(source: str, signature: str) -> str:
    matches = list(re.finditer(rf"(?m)^{re.escape(signature)}\s*\{{", source))
    assert len(matches) == 1, f"installed definition {signature!r} must exist exactly once"
    opening = source.find("{", matches[0].start())
    depth, state, index = 0, "code", opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if char == "\n": state = "code"
        elif state == "block":
            if char == "*" and nxt == "/": state = "code"; index += 1
        elif state in ("string", "char"):
            if char == "\\": index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/": state = "line"; index += 1
        elif char == "/" and nxt == "*": state = "block"; index += 1
        elif char == '"': state = "string"
        elif char == "'": state = "char"
        elif char == "{": depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                result = source[matches[0].start():index + 1]
                assert "BODY GOES HERE." not in result, f"{signature} is still a placeholder"
                return result
        index += 1
    raise AssertionError(f"installed definition {signature!r} has unbalanced braces")


def _code_only(source: str) -> str:
    return re.sub(r"//[^\n]*|/\*.*?\*/", "", source, flags=re.S)


def test_same_tu_scratch_owners_use_exact_prototype_row_and_typed_caller_closure(tmp_path):
    root = _project_root()
    product = (root / "src/designwin.cpp").read_text(encoding="utf-8")
    unk3 = _definition(product, SIGNATURES["unk3"])
    vehicle = _definition(product, SIGNATURES["vehicle"])
    flash = _definition(product, SIGNATURES["flash"])
    special = _definition(product, SIGNATURES["special"])

    code = _code_only(product)
    assert "VehDraw" not in code
    for address in ("0x009AC554", "0x009AC558", "0x009AC559", "0x009AC55A",
                    "0x009AC55B", "0x009AC562"):
        assert address not in code
    row = "VehPrototypes[0x3f]."
    assert row + "ability_flags" in unk3 and row + "icon_offset" in unk3
    assert row + "ability_flags" in vehicle and row + "icon_offset" in vehicle
    assert row + "ability_flags" in flash and row + "icon_offset" in flash

    # The same caller ordering from the pinned source is retained, but all
    # five edges now resolve through declarations/definitions in source.
    special_code = _code_only(special)
    call_order = ("draw_special_2(0)", "draw_special_1(0)", "draw_vehicle(0)",
                  "draw_flash()", "draw_info()", "on_update()")
    positions = [special_code.index(token) for token in call_order]
    assert positions == sorted(positions)
    assert "abil_cond(ability," in special_code
    assert not any(token in special_code for token in
                   ("DesignWinIntSeam", "AbilCondSeam", "0x00435DE0",
                    "0x00435B30", "0x00436090", "0x004346A0", "0x00584B50"))

    # Compile the full installed UNK3/draw_vehicle definitions and the actual
    # store prefix of installed draw_flash against a VehPrototype layout whose
    # size/member offsets match src/veh.h. The long draw_flash body is not
    # duplicated or replaced by an ignored candidate in this focused control.
    flash_prefix_end = flash.index("Buffer *canvas")
    flash_prefix = flash[:flash_prefix_end] + "}"
    fixture_dir = Path(__file__).with_name("fixtures")
    support = fixture_dir / "designwin_scratch_storage_host.hpp"
    main = fixture_dir / "designwin_scratch_storage_host_main.inc"
    compiler = shutil.which("c++")
    assert compiler, "host C++ compiler required for installed-source behavior control"
    translation_unit = tmp_path / "designwin_scratch_storage.cpp"
    executable = tmp_path / "designwin_scratch_storage"
    translation_unit.write_text(
        support.read_text(encoding="utf-8") + "\n" + unk3 + "\n" + vehicle + "\n"
        + flash_prefix + "\n" + main.read_text(encoding="utf-8"),
        encoding="utf-8",
    )
    built = subprocess.run(
        [compiler, "-std=c++17", "-fpermissive", "-O0", str(translation_unit), "-o", str(executable)],
        text=True, capture_output=True,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True)
    assert ran.returncode == 0, ran.stderr
