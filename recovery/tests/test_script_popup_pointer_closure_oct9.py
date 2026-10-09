"""Controls for replacing the popup-title facade with its mutable pointer cell."""

from pathlib import Path
import shutil
import subprocess
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
PRODUCTION_TUS = (
    "base.cpp", "basewin.cpp", "console.cpp", "designwin.cpp", "faction.cpp",
    "game.cpp", "netwin.cpp", "popup.cpp", "reportif.cpp", "reportwin.cpp",
    "veh.cpp",
)


def test_production_consumers_use_pointer_cell_not_facade():
    for name in PRODUCTION_TUS:
        source = (ROOT / "src" / name).read_text()
        live_lines = [line for line in source.splitlines()
                      if "ClimatePopupTitle" in line
                      and not line.lstrip().startswith("//")]
        assert not live_lines, f"{name} still has live facade references: {live_lines}"
    header = (ROOT / "src/game.h").read_text()
    assert "ClimatePopupTitle" not in header
    assert "extern char *ScriptPopupFile;" in header


def test_actual_consumer_reads_mutable_pointer_at_call_time(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ is unavailable")

    source_text = (ROOT / "src/game.cpp").read_text()
    match = re.search(
        r'popp\(ScriptPopupFile,\s*"SEAFALLING",\s*0,\s*"seafal_sm\.pcx",\s*0\)',
        source_text,
    )
    assert match, "expected actual direct popup call in the production owner"
    actual_call = match.group(0)
    source = f'''#include <cassert>
char startup_caption[] = "SCRIPT";
char changed_caption[] = "MUTATED";
char *ScriptPopupFile = startup_caption;
char *observed_caption;
int popp(char *caption, const char *, int, const char *, int) {{
    observed_caption = caption;
    return 0;
}}
int translated_consumer() {{ return {actual_call}; }}
int main() {{
    ScriptPopupFile = changed_caption;
    translated_consumer();
    assert(observed_caption == changed_caption);
}}
'''

    path = tmp_path / "script_popup_pointer.cpp"
    path.write_text(source)
    executable = tmp_path / "script_popup_pointer"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                    str(path), "-o", str(executable)], check=True,
                   capture_output=True)
    result = subprocess.run([str(executable)], capture_output=True)
    assert result.returncode == 0, result.stderr.decode(errors="replace")

    # A captured startup pointer in place of the actual source operand must fail.
    damaged = source.replace("popp(ScriptPopupFile,", "popp(startup_caption,")
    assert damaged != source
    path.write_text(damaged)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", "-fsanitize=undefined", "-fno-sanitize-recover=all",
                    str(path), "-o", str(executable)], check=True,
                   capture_output=True)
    result = subprocess.run([str(executable)], capture_output=True)
    assert result.returncode != 0, "captured-pointer mutant escaped the control"
