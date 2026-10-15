"""Lexical ownership and mock deletion-expression controls; not native ABI or complete filefind behavior proof."""

from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path


def repository_root() -> Path:
    here = Path(__file__).resolve()
    for candidate in here.parents:
        if (candidate / "src").is_dir() and (candidate / "recovery").is_dir():
            return candidate
    raise AssertionError("could not find OpenSMACX source root")


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1 : index]
    raise AssertionError(f"unterminated function: {signature}")


def test_filefind_uses_source_virtual_scalar_delete_at_all_original_exits() -> None:
    root = repository_root()
    source = (root / "src/general.cpp").read_text()
    body = function_body(source, "int __cdecl filefind_init(")
    assert body.count("delete popup;") == 4
    assert "OriginalObject" not in body
    assert "vtable_method<delete_popup_fn>" not in body
    assert body.index("SetCurrentDirectoryA(saved_dir);\n                    delete popup;") < body.index("return 0;")
    assert "if (is_complete) {\n            delete popup;\n            return 0;" in body
    assert "if (response) {\n            delete popup;\n            return response;" in body
    assert body.rindex("SetCurrentDirectoryA(saved_dir);\n        delete popup;\n        return 0;") >= 0


def test_fixed_window_receivers_are_named_source_objects() -> None:
    root = repository_root()
    status_header = (root / "src/statuswin.h").read_text()
    battle_source = (root / "src/battlewin.cpp").read_text()
    status_source = (root / "src/statuswin.cpp").read_text()
    datalink_source = (root / "src/datalink.cpp").read_text()
    cat = function_body(datalink_source, "void Datalink::set_cat(")

    assert "sub_interface_global" not in status_header
    assert '#include "original_seam.h"' not in status_header
    assert "reinterpret_cast<SubInterface *>(0x006EEED8)" not in status_header
    assert "BattleWin g_BattleWin;  // 0x006EEED8" in battle_source
    assert "sub_interface_global" not in battle_source
    assert battle_source.count("g_BattleWin.release_iface_mode();") == 2
    assert status_source.count("g_BattleWin.release_iface_mode();") == 2
    assert cat.count("ReportWinGlobal.reset();") == 2
    assert "reinterpret_cast<ReportWin *>(0x876478)" not in cat
    assert "ReportWin ReportWinGlobal;  // 0x00876478" in (root / "src/reportwin.cpp").read_text()
    first_reset = cat.index("ReportWinGlobal.reset();")
    category_store = cat.index("field_29DC_ = category;")
    assert cat.index("if (field_29DC_ == 0xE)") < first_reset < category_store
    assert category_store < cat.index("ReportWinGlobal.reset();", first_reset + 1)


def test_extracted_delete_expression_calls_virtual_destructor_once() -> None:
    root = repository_root()
    source = (root / "src/general.cpp").read_text()
    body = function_body(source, "int __cdecl filefind_init(")
    assert "delete popup;" in body
    compiler = os.environ.get("CXX") or shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host C++ compiler required for the source-expression control"

    # Use the exact cleanup expression extracted from the live source, not a
    # separately transcribed deletion helper.
    expression = next(
        line.strip() for line in body.splitlines() if line.strip() == "delete popup;"
    )
    harness = f"""
struct BasePop {{
    int *events;
    virtual ~BasePop() {{ events[0] += 1; }}
}};
struct Popup : BasePop {{
    ~Popup() override {{ events[1] += 1; }}
}};
int main() {{
    int events[2] = {{0, 0}};
    BasePop *popup = new Popup;
    popup->events = events;
    {expression}
    return events[0] == 1 && events[1] == 1 ? 0 : 1;
}}
    """
    with tempfile.TemporaryDirectory(prefix="osmx-address-call-") as temp:
        executable = Path(temp) / "control"
        subprocess.run(
            [compiler, "-x", "c++", "-std=c++11", "-o", str(executable), "-"],
            input=harness,
            text=True,
            check=True,
        )
        subprocess.run([str(executable)], check=True)
