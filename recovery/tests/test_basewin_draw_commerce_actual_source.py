from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("basewin_draw_commerce_host_control.cpp")
SIGNATURE = "void BaseWin::draw_commerce(int update_mode)"

OLD_BRANCH = """            if (own_sanctions != 0 || partner_sanctions != 0) {
                canvas->set_text_color(TextColourTable[110], TextColourTable[111], 1, 1);
                strcat(StringTemp, label_get(450));
                strcat(StringTemp, " (");
                char turns[32];
                _itoa(own_sanctions > partner_sanctions
                          ? own_sanctions : partner_sanctions, turns, 10);
                strcat(StringTemp, turns);
                strcat(StringTemp, ")");
                CharUpperA(StringTemp);
                canvas->write_l(StringTemp, report.left + 5, detail_y,
                                strlen(StringTemp));
            } else {
                strcat(StringTemp, label_get((PlayersData[owner].diplo_treaties[partner]
                                              & DTREATY_PACT) != 0 ? 645 : 644));
                int text_end = canvas->write_l(StringTemp, report.left + 5,
                                               detail_y, strlen(StringTemp));
                if (BaseCommercePartnerReceipts[slot] != BaseCommerceReceipts[slot]) {
                    StringTemp[0] = 0;
                    strcat(StringTemp, " (");
                    strcat(StringTemp, label_get(648));
                    strcat(StringTemp, " +");
                    char partner_amount[32];
                    _itoa(BaseCommercePartnerReceipts[slot], partner_amount, 10);
                    strcat(StringTemp, partner_amount);
                    strcat(StringTemp, ")");
                    canvas->write_l(StringTemp, text_end, detail_y, strlen(StringTemp));
                }
            }
"""

NEW_BRANCH = """            int text_end;
            if (own_sanctions != 0 || partner_sanctions != 0) {
                canvas->set_text_color(TextColourTable[110], TextColourTable[111], 1, 1);
                strcat(StringTemp, label_get(450));
                strcat(StringTemp, " (");
                char turns[32];
                _itoa(own_sanctions > partner_sanctions
                          ? own_sanctions : partner_sanctions, turns, 10);
                strcat(StringTemp, turns);
                strcat(StringTemp, ")");
                CharUpperA(StringTemp);
                text_end = canvas->write_l(StringTemp, report.left + 5,
                                           detail_y, strlen(StringTemp));
            } else {
                strcat(StringTemp, label_get((PlayersData[owner].diplo_treaties[partner]
                                              & DTREATY_PACT) != 0 ? 645 : 644));
                text_end = canvas->write_l(StringTemp, report.left + 5,
                                           detail_y, strlen(StringTemp));
            }
            if (BaseCommercePartnerReceipts[slot] != BaseCommerceReceipts[slot]) {
                StringTemp[0] = 0;
                strcat(StringTemp, " (");
                strcat(StringTemp, label_get(648));
                strcat(StringTemp, " +");
                char partner_amount[32];
                _itoa(BaseCommercePartnerReceipts[slot], partner_amount, 10);
                strcat(StringTemp, partner_amount);
                strcat(StringTemp, ")");
                canvas->write_l(StringTemp, text_end, detail_y, strlen(StringTemp));
            }
"""


def _extract_function(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated draw_commerce definition")


def _compile_run(compiler: str, tmp_path: Path, body: str, suffix: str,
                 mode: int = 0) -> int:
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count("/*__DRAW_COMMERCE_BODY__*/") == 1
    cpp = tmp_path / f"commerce-{suffix}.cpp"
    binary = tmp_path / f"commerce-{suffix}"
    cpp.write_text(fixture.replace("/*__DRAW_COMMERCE_BODY__*/", body),
                   encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp),
         "-o", str(binary)], capture_output=True, text=True, check=False,
        timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary), str(mode)], capture_output=True, text=True,
                          check=False, timeout=30).returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_source_preserves_receipt_delta_during_sanctions(
    compiler_name: str, tmp_path: Path,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host compiler unavailable: {compiler_name}")
    source = (ROOT / "src" / "basewin.cpp").read_text(encoding="utf-8")
    actual = _extract_function(source)
    assert NEW_BRANCH in actual
    mutant = actual.replace(NEW_BRANCH, OLD_BRANCH, 1)
    for mode in (0, 1, 2):
        assert _compile_run(compiler, tmp_path, actual,
                            f"actual-{mode}", mode) == 0
    # The prior wrong branch suppresses the common-tail append during sanctions.
    assert _compile_run(compiler, tmp_path, mutant, "old-branch-negative") != 0
    # The existing treaty path already appends the difference, and equal
    # receipts must not create an annotation in either implementation.
    assert _compile_run(compiler, tmp_path, mutant, "mutant-treaty", 1) == 0
    assert _compile_run(compiler, tmp_path, mutant, "mutant-equal", 2) == 0
