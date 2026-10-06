"""Bounded host controls compile actual product declarations and owner bodies."""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).resolve().parent / "fixtures/color_owner_control.cpp"

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

def test_actual_color_owner_declarations_and_bodies():
    header = (ROOT / "src/caviar.h").read_text()
    caviar = (ROOT / "src/caviar.cpp").read_text()
    popup = (ROOT / "src/popup.cpp").read_text()
    declarations = definition(header, "struct CvrRecord {") + ";\n" + definition(header, "struct CaviarData {") + ";\n"
    caviar_bodies = definition(caviar, "int CaviarData::apply_palette(") + "\n" + definition(caviar, "void CaviarData::release_palette_chunk(")
    globals_ = []
    for name in ("PopupSavedLocX", "PopupSavedLocY"):
        declarations_ = [line.split("//", 1)[0] for line in popup.splitlines() if line.startswith("static int " + name + " =")]
        assert len(declarations_) == 1
        globals_.append(declarations_[0])
    popup_bodies = "\n".join(globals_) + "\n" + definition(popup, "int __cdecl set_popup_saved_location(")
    fixture = FIXTURE.read_text()
    for marker, source in (("PRODUCT_DECLARATIONS", declarations), ("PRODUCT_CAVIAR_BODIES", caviar_bodies), ("PRODUCT_POPUP_BODIES", popup_bodies)):
        placeholder = "/* " + marker + " */"
        assert fixture.count(placeholder) == 1
        fixture = fixture.replace(placeholder, source)
    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert compilers, "a host C++ compiler is required"
    with tempfile.TemporaryDirectory() as directory:
        temporary = Path(directory)
        cpp = temporary / "control.cpp"
        cpp.write_text(fixture)
        for index, compiler in enumerate(compilers):
            executable = temporary / ("control" + str(index))
            subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)], check=True, capture_output=True, text=True, timeout=60)
            subprocess.run([str(executable)], check=True, capture_output=True, text=True, timeout=30)
