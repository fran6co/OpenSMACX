"""Host controls for routing radius consumers to their existing storage owner.

These controls reject duplicate-header storage.
The linked test compiles declarations/definitions extracted verbatim from the
source after the owner migration; it does not compile veh.cpp or prove native
image allocation, original-data equality, or world_erosion runtime behavior.
"""

from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
X_VALUES = (1, 2, 1, 0, -1, -2, -1, 0, 0)
Y_VALUES = (-1, 0, 1, 2, 1, 0, -1, -2, 0)


def _array_definition(source: str, name: str) -> str:
    match = re.search(
        rf"(?m)^int {name}\[9\]\s*=\s*\{{[^}}]*\}}\s*;", source
    )
    assert match, f"missing source-owned definition for {name}[9]"
    return match.group(0)


def _values(definition: str) -> tuple[int, ...]:
    body = definition[definition.index("{") + 1:definition.rindex("}")]
    return tuple(int(value) for value in re.findall(r"-?\d+", body))


def _owner_sources() -> tuple[str, str]:
    header = (ROOT / "src/map.h").read_text()
    owner = (ROOT / "src/veh.cpp").read_text()
    return header, owner


def test_radius_base_tables_have_one_declared_owner_and_pinned_mapping():
    header, owner = _owner_sources()
    assert re.search(r"(?m)^extern int VehMoveDeltaX\[9\];$", header)
    assert re.search(r"(?m)^extern int VehMoveDeltaY\[9\];$", header)
    assert not re.search(r"(?m)^const int RadiusBase[XY]\[", header)

    x_definition = _array_definition(owner, "VehMoveDeltaX")
    y_definition = _array_definition(owner, "VehMoveDeltaY")
    assert _values(x_definition) == X_VALUES
    assert _values(y_definition) == Y_VALUES

    # The image starts, half-open extents, and direct historical consumer are
    # the evidence from the ignored ownership memo; keep the two rows distinct.
    assert "RECOVERY_OBJECT: v2 0x0066EF50 0x0066EF74 ?VehMoveDeltaX@@3PAHA 0x004BFE00" in owner
    assert "RECOVERY_OBJECT: v2 0x0066EF74 0x0066EF98 ?VehMoveDeltaY@@3PAHA 0x004BFE00" in owner
    assert len(re.findall(r"(?m)^int VehMoveDeltaX\[9\]\s*=", owner)) == 1
    assert len(re.findall(r"(?m)^int VehMoveDeltaY\[9\]\s*=", owner)) == 1

    for index in range(4):
        assert X_VALUES[index + 4] == -X_VALUES[index]
        assert Y_VALUES[index + 4] == -Y_VALUES[index]
    assert X_VALUES[8] == Y_VALUES[8] == 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_source_extracted_two_tu_tables_are_one_linked_object(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")

    header, owner = _owner_sources()
    x_declaration = "extern int VehMoveDeltaX[9];"
    y_declaration = "extern int VehMoveDeltaY[9];"
    assert x_declaration in header
    assert y_declaration in header
    x_definition = _array_definition(owner, "VehMoveDeltaX")
    y_definition = _array_definition(owner, "VehMoveDeltaY")

    (tmp_path / "tables.h").write_text(
        "#pragma once\n" + x_declaration + "\n" + y_declaration + "\n"
    )
    (tmp_path / "owner.cpp").write_text(
        '#include "tables.h"\n' + x_definition + "\n" + y_definition + "\n"
    )
    (tmp_path / "client_a.cpp").write_text(
        '#include "tables.h"\n'
        'extern "C" const int *x_a() { return VehMoveDeltaX; }\n'
        'extern "C" const int *y_a() { return VehMoveDeltaY; }\n'
    )
    (tmp_path / "client_b.cpp").write_text(
        '#include "tables.h"\n'
        'extern "C" const int *x_b() { return VehMoveDeltaX; }\n'
        'extern "C" const int *y_b() { return VehMoveDeltaY; }\n'
    )
    (tmp_path / "main.cpp").write_text(
        "#include <cassert>\n"
        'extern "C" const int *x_a(); extern "C" const int *y_a();\n'
        'extern "C" const int *x_b(); extern "C" const int *y_b();\n'
        "int main() {\n"
        "  assert(x_a() == x_b()); assert(y_a() == y_b());\n"
        "  const int x[9] = {1,2,1,0,-1,-2,-1,0,0};\n"
        "  const int y[9] = {-1,0,1,2,1,0,-1,-2,0};\n"
        "  for (int i=0;i<9;++i) { assert(x_a()[i]==x[i]); assert(y_a()[i]==y[i]); }\n"
        "  for (int i=0;i<4;++i) { assert(x_a()[i+4]==-x_a()[i]); assert(y_a()[i+4]==-y_a()[i]); }\n"
        "}\n"
    )

    binary = tmp_path / "radius-owner-control"
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(tmp_path / "owner.cpp"), str(tmp_path / "client_a.cpp"),
         str(tmp_path / "client_b.cpp"), str(tmp_path / "main.cpp"),
         "-o", str(binary)],
        check=True, capture_output=True, text=True,
    )
    subprocess.run([str(binary)], check=True)


def test_world_erosion_remains_the_recorded_consumer():
    source = (ROOT / "src/map.cpp").read_text()
    assert "ORIGINAL: 0x005C2CB0 ?world_erosion@@YAXXZ" in source
    body_start = source.index("void __cdecl world_erosion()")
    body = source[body_start:source.index("\n}\n", body_start) + 2]
    assert "VehMoveDeltaX" in body
    assert "VehMoveDeltaY" in body
