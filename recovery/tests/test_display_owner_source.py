"""Behavioral boundary controls compile the actual shared-display owner bodies.

The host SDK models do not prove native ABI, link selection or game acceptance.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).resolve().parent / "fixtures/display_owner_control.cpp"


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated product definition: " + signature)


def test_actual_display_owner_bodies():
    source = (ROOT / "src/general.cpp").read_text()
    signatures = (
        "BOOL CALLBACK direct_draw_device_callback(",
        "GUID *__cdecl select_direct_draw_device(",
        "HRESULT CALLBACK sub_62cd10(",
        "int __cdecl sub_62cd50(",
        "int __cdecl sub_62d0f0(",
        'extern "C" int __cdecl sub_62d100(',
        "HDC __cdecl sub_62d160(",
        "void __cdecl sub_62d1a0(",
        "void __cdecl sub_62d1e0(",
        "int __cdecl sub_62d2c0(",
    )
    bodies = definition(source, "struct DirectDrawClipRegion {") + ";\n"
    bodies += "\n".join(definition(source, signature) for signature in signatures)
    fixture = FIXTURE.read_text()
    marker = "/* PRODUCT_DISPLAY_BODIES */"
    assert fixture.count(marker) == 1
    fixture = fixture.replace(marker, bodies)
    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers, "a host C++ compiler is required"
    with tempfile.TemporaryDirectory() as directory:
        temporary = Path(directory)
        cpp = temporary / "control.cpp"
        cpp.write_text(fixture)
        for index, compiler in enumerate(compilers):
            executable = temporary / ("control" + str(index))
            subprocess.run([compiler, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)], check=True, capture_output=True, text=True, timeout=60)
            subprocess.run([str(executable)], check=True, capture_output=True, text=True, timeout=30)
