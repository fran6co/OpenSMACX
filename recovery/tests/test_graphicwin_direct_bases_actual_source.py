"""Actual-source structural/lifetime control for the GraphicWin base cleanup.

The compiled fixture deliberately substitutes tiny tracing member types for
NetWin's real members. It compiles the actual extracted NetWin constructor and
destructor bodies and derives its order from the current header declaration.
It does not compile the full application classes or prove native layout,
vtable selection, ABI, or original compiler equivalence.
"""

from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"


def mask_comments(text: str) -> str:
    """Replace comments while preserving source offsets/newlines."""
    return re.sub(
        r"//[^\n]*|/\*[\s\S]*?\*/",
        lambda match: re.sub(r"[^\n]", " ", match.group(0)),
        text,
    )


def class_region(text: str, name: str) -> tuple[str, str]:
    masked = mask_comments(text)
    declaration = re.search(
        rf"\bclass\s+{re.escape(name)}\s*:\s*([^{{]+)\{{", masked
    )
    assert declaration, f"class declaration not found: {name}"
    opening = masked.index("{", declaration.start())
    depth = 1
    cursor = opening + 1
    while depth:
        if masked[cursor] == "{":
            depth += 1
        elif masked[cursor] == "}":
            depth -= 1
        cursor += 1
    return declaration.group(1).strip(), text[opening + 1 : cursor - 1]


def direct_bases(text: str, name: str) -> list[str]:
    bases, _ = class_region(text, name)
    return re.findall(r"\bpublic\s+([A-Za-z_][A-Za-z_0-9]*)", bases)


def body(text: str, qualified_signature: str) -> str:
    masked = mask_comments(text)
    start = masked.find(qualified_signature)
    assert start >= 0, f"definition not found: {qualified_signature}"
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 1
    cursor = opening + 1
    while depth:
        if masked[cursor] == "{":
            depth += 1
        elif masked[cursor] == "}":
            depth -= 1
        cursor += 1
    return text[opening + 1 : cursor - 1]


def member_positions(header: str, owner: str, declarations: list[str]) -> list[int]:
    _, region = class_region(header, owner)
    masked = mask_comments(region)
    positions = []
    for declaration in declarations:
        match = re.search(declaration, masked)
        assert match, f"missing real member declaration for {owner}: {declaration}"
        positions.append(match.start())
    return positions


def structural_snapshot() -> tuple[str, str, str, str, str, str]:
    base_h = (SRC / "basewin.h").read_text()
    net_h = (SRC / "netwin.h").read_text()
    picker_h = (SRC / "prodpicker.h").read_text()
    graphic_h = (SRC / "graphicwin.h").read_text()
    base_cpp = (SRC / "basewin.cpp").read_text()
    net_cpp = (SRC / "netwin.cpp").read_text()
    picker_cpp = (SRC / "prodpicker.cpp").read_text()

    assert direct_bases(base_h, "BaseWin") == ["GraphicWin", "SubInterface"]
    assert direct_bases(net_h, "NetWin") == ["GraphicWin"]
    assert direct_bases(picker_h, "ProdPicker") == ["GraphicWin"]
    assert "ConstructedGraphicWin" not in mask_comments(graphic_h)

    # Confirm the actual currently installed constructor/destructor bodies
    # relevant to the three owner declarations. NetWin's bodies are compiled
    # in the focused fixture below; these source checks preserve BaseWin and
    # ProdPicker's selected lifecycle statements without pretending to compile
    # their full dependency-heavy classes here.
    assert body(base_cpp, "BaseWin::BaseWin()").strip() == ""
    assert body(base_cpp, "BaseWin::~BaseWin()").strip() == ""
    picker_ctor = body(picker_cpp, "ProdPicker::ProdPicker()")
    picker_dtor = body(picker_cpp, "ProdPicker::~ProdPicker()")
    picker_close = body(picker_cpp, "ProdPicker::close()")
    for statement in (
        "field_A830_ = 0x86;", "field_A834_ = 0x86;", "field_A14_ = 0;",
        "field_A838_ = 0x6C;", "field_A83C_ = 0x5E;",
    ):
        assert statement in picker_ctor
    assert "close();" in picker_dtor
    assert "field_A14_ = 0;" in picker_close and "GraphicWin::close();" in picker_close

    assert member_positions(
        base_h, "BaseWin", [r"\bProdPicker\s+prodPicker_\s*;", r"\bGraphicWin\s+graphicWin2_\s*;"]
    ) == sorted(member_positions(
        base_h, "BaseWin", [r"\bProdPicker\s+prodPicker_\s*;", r"\bGraphicWin\s+graphicWin2_\s*;"]
    ))
    assert member_positions(
        picker_h, "ProdPicker", [r"\bSprite\s+sprites_\s*\[\s*3\s*\]", r"\bFont\s+font1_\s*;"]
    ) == sorted(member_positions(
        picker_h, "ProdPicker", [r"\bSprite\s+sprites_\s*\[\s*3\s*\]", r"\bFont\s+font1_\s*;"]
    ))

    ordered_netwin_members = [
        r"\bSpot\s+spot_\s*;",
        r"\bStringBox\s+stringBox_\s*;",
        r"\bEditBox\s+editBox_\s*;",
        r"\bCheckBox\s+checkBox_\s*;",
        r"\bFlatButton\s+flatButton1_\s*;",
        r"\bFlatButton\s+flatButton2_\s*;",
    ]
    positions = member_positions(net_h, "NetWin", ordered_netwin_members)
    assert positions == sorted(positions), "NetWin member lifetime order changed"
    return base_h, net_h, picker_h, graphic_h, net_cpp, picker_cpp


def test_mutants_are_rejected_by_source_facts() -> None:
    _, net_h, _, _, _, _ = structural_snapshot()
    fake_intermediate = net_h.replace(
        "class NetWin : public GraphicWin {",
        "class NetWin : public ConstructedGraphicWin {",
        1,
    )
    assert direct_bases(fake_intermediate, "NetWin") != ["GraphicWin"]

    moved_member = net_h.replace(
        "  Spot spot_;  // 0xD34\n  StringBox stringBox_;  // 0xD40, size == sizeof(StringBox)",
        "  StringBox stringBox_;  // 0xD40, size == sizeof(StringBox)\n  Spot spot_;  // 0xD34",
        1,
    )
    expected = [
        r"\bSpot\s+spot_\s*;",
        r"\bStringBox\s+stringBox_\s*;",
    ]
    assert member_positions(moved_member, "NetWin", expected) != sorted(
        member_positions(moved_member, "NetWin", expected)
    )


def test_actual_netwin_lifetime_bodies_with_focused_host_fixture() -> None:
    _, net_h, _, _, net_cpp, _ = structural_snapshot()
    constructor = body(net_cpp, "NetWin::NetWin()")
    destructor = body(net_cpp, "NetWin::~NetWin()")
    member_names = ["spot_", "stringBox_", "editBox_", "checkBox_", "flatButton1_", "flatButton2_"]
    _, net_region = class_region(net_h, "NetWin")
    ordered_names = [
        name for name in member_names
        if re.search(rf"\b{re.escape(name)}\s*;", mask_comments(net_region))
    ]
    assert ordered_names == member_names

    fields = "\n".join(f'    TrackedMember {name}{{"{name}"}};' for name in ordered_names)
    harness = f'''#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
static std::vector<std::string> events;
static void record(const std::string &s) {{ events.push_back(s); }}
struct GraphicWin {{
    GraphicWin() {{ record("GraphicWin+"); }}
    ~GraphicWin() {{ record("GraphicWin-"); }}
}};
struct TrackedMember {{
    const char *name;
    explicit TrackedMember(const char *n) : name(n) {{ record(std::string(name) + "+"); }}
    ~TrackedMember() {{ record(std::string(name) + "-"); }}
    void shutdown() {{ record(std::string(name) + ".shutdown"); }}
    void close() {{ record(std::string(name) + ".close"); }}
}};
struct NetWinProbe : public GraphicWin {{
    void *faction_records_;
    void *faction_records_copy_;
    int field_761C_;
    int field_7620_;
{fields}
    NetWinProbe() {{{constructor}}}
    ~NetWinProbe() {{{destructor}}}
}};
int main() {{
    {{
        NetWinProbe value;
        GraphicWin *const graphic_view = &value;
        if (static_cast<void *>(graphic_view) != static_cast<void *>(&value)) return 3;
    }}
    const char *expected[] = {{
        "GraphicWin+", "spot_+", "stringBox_+", "editBox_+", "checkBox_+",
        "flatButton1_+", "flatButton2_+",
        "spot_.shutdown", "stringBox_.close", "editBox_.close", "checkBox_.close",
        "flatButton1_.close",
        "flatButton2_-", "flatButton1_-", "checkBox_-", "editBox_-",
        "stringBox_-", "spot_-", "GraphicWin-"
    }};
    if (events.size() != sizeof(expected) / sizeof(expected[0])) return 1;
    for (size_t i = 0; i < events.size(); ++i) {{
        if (events[i] != expected[i]) {{ std::cerr << i << ":" << events[i] << "\\n"; return 2; }}
    }}
    return 0;
}}
'''
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert all(compilers), "both GCC and Clang are required for the host control"
    with tempfile.TemporaryDirectory(prefix="osmx-graphicwin-lifetime-") as tmp:
        work = Path(tmp)
        source = work / "fixture.cpp"
        source.write_text(harness)
        for compiler in compilers:
            executable = work / (Path(compiler).name + "-fixture")
            subprocess.run(
                [compiler, "-std=c++11", "-O0", "-Wall", "-Wextra", str(source), "-o", str(executable)],
                check=True, capture_output=True, text=True,
            )
            subprocess.run([str(executable)], check=True, capture_output=True, text=True)
