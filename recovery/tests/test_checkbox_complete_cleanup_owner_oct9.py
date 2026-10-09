"""Source-reader controls for the generated CheckBox cleanup identity."""
from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[2]
CPP = ROOT / "src/netwin.cpp"
HEADER = ROOT / "src/netwin.h"

ORIGINAL = "// ORIGINAL: 0x004837F0 ?UNK4@NetWin@@QAEXXZ 0x004837F0-0x00483813 FILE BYTE_EXACT"
SYMBOL = "// symbol    ??_DCheckBox@@QAEXXZ"


def test_original_mapping_and_generated_symbol_identity_are_retained():
    source = CPP.read_text()
    assert source.count(ORIGINAL) == 1
    section = source[source.index(ORIGINAL):source.index("*/", source.index(ORIGINAL))]
    assert SYMBOL in section
    assert "0x004837F9" in section and "0x00483804" in section and "0x0048380B" in section
    assert "0x0060E740" in section and "0x00608E10" in section and "0x005D4DD0" in section
    assert "CheckBox, Dialog and GraphicWin destructors" in section
    assert "src/recovered/004837f0.cpp" in section
    assert "historical NetWin catalogue interpretation" in section
    assert "not the current owner" in section
    assert "BYTE_EXACT" in ORIGINAL


def test_no_handwritten_netwin_cleanup_method_or_declaration_remains():
    source = CPP.read_text()
    header = HEADER.read_text()
    assert re.search(r"\bvoid\s+NetWin::UNK4\s*\(\s*\)\s*\{", source) is None
    assert re.search(r"\bvoid\s+UNK4\s*\(\s*\)\s*;", header) is None
    assert "reinterpret_cast<CheckBox *>(self + 0x1C)" not in source
    assert "reinterpret_cast<Dialog *>(self + 0xA34)" not in source
    assert "reinterpret_cast<GraphicWin *>(self + 0x1C)" not in source
    assert not re.search(r"->\s*~(?:CheckBox|Dialog|GraphicWin)\s*\(", source)


def test_real_check_box_member_and_netwin_raii_own_cleanup():
    source = CPP.read_text()
    header = HEADER.read_text()
    assert re.search(r"\bNetWin\s*\(\s*\)\s*;", header)
    assert re.search(r"~NetWin\s*\(\s*\)\s*;", header)
    assert re.search(r"\bCheckBox\s+checkBox_\s*;", header)
    member = header[header.index("CheckBox checkBox_") - 180:
                    header.index("CheckBox checkBox_") + 120]
    assert "compiler emits CheckBox's complete-object cleanup helper" in member
    assert "void NetWin::UNK4()" not in source
    assert "??_DCheckBox@@QAEXXZ" in source
    assert "not a handwritten NetWin method" in source
    destructor = source[source.index("NetWin::~NetWin()"):
                        source.index("\nvoid NetWin::close()", source.index("NetWin::~NetWin()"))]
    assert "checkBox_.close();" in destructor
    assert "compiler-owned teardown" in source
