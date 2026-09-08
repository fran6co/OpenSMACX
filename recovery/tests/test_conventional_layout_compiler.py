"""Actual compiler controls for conventional hints and packed inherited members."""

import json
from pathlib import Path
import sys

import pytest

from match_recovery.layout_check import check_layout
from recovery.environment.compile import compile_object
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_compiler_variants import variant_project
from test_symbol_check import mapped


@pytest.mark.parametrize("use_in_product", [True, False])
def test_real_compiler_detects_shift_and_restoration_without_an_abi_size_table(variant_project, use_in_product):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    (root / "code/IMAGE_PIN").write_text("sha256 " + "a" * 64)
    source = root / "code/unit.cpp"
    source.write_text('#include "types.h"\n' + mapped() +
                      ('\nint read(Widget *p) { return p->field_5_; }\n' if use_in_product else ''))
    header = root / "code/types.h"
    original = ("struct Base { int prefix; };\n#pragma pack(push, 1)\n"
                "struct Widget : Base {\n  char narrow;\n  int field_5_; // 0x5\n};\n#pragma pack(pop)\n")
    header.write_text(original)
    (root / "build").mkdir()
    database = root / "build/compile_commands.json"
    database.write_text(json.dumps([{"file": "code/unit.cpp", "directory": str(root), "arguments": ["compiler"]}]))
    policy = root / "recovery/project.json"
    config = json.loads(policy.read_text())
    config["mappings"] = {"compile_commands": "build/compile_commands.json", "forwarder_macros": []}
    config["measurement"]["flag_sets"] = [["/c", "/O2", "/Gy"], ["/c", "/O2", "/Gy", "/Oy-"]]
    policy.write_text(json.dumps(config))
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    def measure():
        class Compiler:
            def __call__(self, *args):
                return compile_object(*args)

            def overlay(self, source, replacements, command, flags):
                from recovery.environment.compile import compile_overlay
                return compile_overlay(root, source, replacements, command, flags)
        return check_layout(root, conventional_offsets=True, witness_unobserved=not use_in_product,
                            compiler=Compiler(), command_for=lambda _: command)
    good = measure()
    assert good["result"] == "pass" and good["counts"] == {"pass": 2}
    assert good["compile_attempts"] == (2 if use_in_product else 8)
    header.write_text(original.replace("char narrow", "short narrow"))
    bad = measure()
    assert bad["result"] == "fail" and bad["counts"] == {"fail": 2}
    assert {row["actual_offset"] for row in bad["observations"]} == {6}
    header.write_text(original)
    assert measure()["result"] == "pass"
    assert header.read_text() == original
