"""Host controls for Console-owned writable text objects."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/console.cpp"

EXPECTED = {
    "ConsoleStrMenuSimple": "MENUSIMPLE",
    "ConsoleStrResetAllFac": "RESETALLFAC",
    "ConsoleStrResetAllTech": "RESETALLTECH",
    "ConsoleEditorRockinessLabel": "ROCKOSITY",
    "ConsoleStrEditorMinor": "EDITORMINOR",
    "ConsoleStrMapFlat": "MAPFLAT",
    "ConsoleStrMapRound": "MAPROUND",
    "ConsoleStrEditorYear": "EDITORYEAR",
    "ConsoleStrMenuDetail": "MENUDETAIL",
    "ConsoleStrFungosity": "FUNGOSITY",
    "ConsoleStrFungmotize": "FUNGMOTIZE",
    "ConsoleStrProtectOff": "PROTECTOFF",
    "ConsoleStrProtectOn": "PROTECTON",
    "ConsoleStrTerraSurvey2": "TERRASURVEY2",
    "ConsoleStrTechChange": "TECHCHANGE",
    "ConsoleStrAcLogoSm": "ac_logo_sm.pcx",
    "ConsoleStrVersionControlX": "VERSIONCONTROLX",
    "ConsoleStrDesignated": "DESIGNATED",
    "ConsoleStrUndesignated": "UNDESIGNATED",
    "ConsoleStrEditorEnergy": "EDITORENERGY",
    "ConsoleStrBrush": "BRUSH",
    "ConsoleStrTechRandom": "TECHRANDOM",
    "ConsoleStrSimulNo": "SIMULNO",
    "ConsoleStrUnitMoved": "UNITMOVED",
    "ConsoleStrSimulNo2": "SIMULNO",
    "ConsoleStrUnitMoved2": "UNITMOVED",
    "ConsoleStrOnlyCombat": "ONLYCOMBAT",
    "ConsoleStrNotBase": "NOTBASE",
    "ConsoleStrNotInTriad": "NOTINTRIAD",
    "ConsoleStrDestroyBomb": "DESTROYBOMB",
    "ConsoleStrDestroy": "DESTROY",
    "ConsoleStrNotHere": "NOTHERE",
    "ConsoleStrNoBomb": "NOBOMB",
    "ConsoleStrScenParam": "SCENPARAM",
    "ConsoleStrEditorWhoListTitle": "EDITORWHO",
    "ConsoleStrCustomNames": "CUSTOMNAMES",
    "ConsoleStrHostTime": "HOSTTIME",
    "ConsoleStrCustomTime": "CUSTOMTIME",
}

DECLARATION = re.compile(r'^char (Console\w+)\[\] = "([^"\\]*)";\s*// (0x[0-9A-Fa-f]+)$', re.M)


def declarations(source: str) -> dict[str, tuple[str, str, str]]:
    return {
        name: (whole, text, address)
        for whole, name, text, address in (
            (match.group(0), match.group(1), match.group(2), match.group(3))
            for match in DECLARATION.finditer(source)
        )
    }


def harness(lines: list[str], mutate: bool = False) -> str:
    decls = "\n".join(lines)
    checks = "\n".join(
        f'    assert(strcmp({name}, "{value}") == 0);\n'
        f'    assert(sizeof({name}) == {len(value) + 1});'
        for name, value in EXPECTED.items()
    )
    return f'''#include <cassert>
#include <cstring>
{decls}
int main() {{
{checks}
    assert(&ConsoleStrSimulNo[0] != &ConsoleStrSimulNo2[0]);
    assert(&ConsoleStrUnitMoved[0] != &ConsoleStrUnitMoved2[0]);
    ConsoleStrSimulNo[0] = 'X';
    ConsoleStrUnitMoved[0] = 'Y';
    assert(ConsoleStrSimulNo2[0] == 'S');
    assert(ConsoleStrUnitMoved2[0] == 'U');
    return 0;
}}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_console_text_objects_are_writable_distinct_and_exact(tmp_path: Path, compiler: str) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")

    source = SOURCE.read_text()
    found = declarations(source)
    assert set(found) == set(EXPECTED)
    assert {name: value[1] for name, value in found.items()} == EXPECTED
    lines = [found[name][0] for name in EXPECTED]

    harness_path = tmp_path / "console_text.cpp"
    binary = tmp_path / "console_text"
    harness_path.write_text(harness(lines))
    built = subprocess.run([executable, "-std=c++11", str(harness_path), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr

    damaged = [line.replace('"RESETALLFAC"', '"RESETALLFXC"') for line in lines]
    harness_path.write_text(harness(damaged))
    built = subprocess.run([executable, "-std=c++11", str(harness_path), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True)
    assert ran.returncode != 0
