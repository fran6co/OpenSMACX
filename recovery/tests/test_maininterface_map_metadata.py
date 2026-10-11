"""Mapped full editor method with actual Map storage; host-only behavioral controls."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_map_metadata_owner import ROOT, definition


def projected(damage=None):
    header = (ROOT / "src/map.h").read_text()
    landmark = re.search(r"struct Landmark \{.*?\n\};", header, re.S).group()
    start = header.index("struct MapMetadataFields {")
    layout = landmark + "\nconst int MaxLandmarkNum=64;\n" + header[start:header.index("// The tile-flag mask", start)]
    record = next(r for r in read_file(ROOT / "src/maininterface.cpp") if r.address == 0x45B550)
    assert record.image_spans == ((0x45B550, 0x45BBF7),)
    body = definition(record.region, "void MainInterface::draw_edit_pane()")
    if damage:
        old, new = damage
        assert body.count(old) == 1
        body = body.replace(old, new)
    fixture = Path(__file__).with_name("maininterface_map_metadata_control.cpp").read_text()
    return fixture.replace("// @@ACTUAL_MAP_LAYOUT@@", layout).replace("// @@ACTUAL_DRAW_EDIT_PANE@@", body)


def run_control(tmp_path, damage=None):
    compiler = shutil.which("c++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    source = tmp_path / "editor.cpp"
    source.write_text(projected(damage))
    binary = tmp_path / "editor"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)], capture_output=True, timeout=60)
    assert built.returncode == 0, built.stderr.decode(errors="replace")
    return subprocess.run([str(binary)], capture_output=True, timeout=15)


def test_actual_draw_edit_pane_cloud_setting_and_append(tmp_path):
    result = run_control(tmp_path)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("damage", [
    ("static_cast<uint32_t>(MapCloudCover)", "static_cast<uint32_t>(MapSizePlanet)"),
    ("strcat(line, \"{\");\n    int label;", "line[0]=0; strcat(line, \"{\");\n    int label;"),
])
def test_compiling_wrong_metadata_or_cleared_append_detected(tmp_path, damage):
    assert run_control(tmp_path, damage).returncode != 0
