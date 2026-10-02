"""Actual MonuWin class/member statements under explicit typed host doubles.

These portable regression contracts preserve findings from independent native
review, but do not themselves execute original instructions or establish native
ABI, linked preservation, helper correctness, or full-game acceptance. No image,
Unicorn, cached native trace, copied recovered body, or private path is needed.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("monuwin_host_fixture.inc")
LEXEMES = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|'
    r"'(?:\\.|[^'\\])*'|[{}]"
)
METHODS = (
    "MonuWin::MonuWin()", "MonuWin::~MonuWin()", "void MonuWin::close()",
    "void MonuWin::on_button_clicked(", "int MonuWin::on_key_click(",
    "void MonuWin::on_left_click_event(", "void MonuWin::on_mouse_move(",
    "int MonuWin::init()", "void MonuWin::on_redraw()",
)
DAMAGES = (
    "spot_output", "primary_spot", "sprite_owner", "local_scalar",
    "palette_offset", "duplicate_close", "current_clip", "selected_button",
    "sort_hole", "sparse_victory", "parser_owner", "parser_grammar",
    "secrets_sentinel", "first_battle_year", "parser_defaults",
)


def braced(source, marker, declaration=False):
    matches = list(re.finditer(r"(?m)^" + re.escape(marker), source))
    assert len(matches) == 1, f"expected one live definition: {marker}"
    start = matches[0].start()
    opening = source.index("{", start)
    depth = 0
    for token in LEXEMES.finditer(source, opening):
        if token.group() == "{":
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if not depth:
                end = token.end()
                if declaration:
                    assert source[end] == ";"
                    end += 1
                return source[start:end]
    raise AssertionError(f"unterminated live definition: {marker}")


def replace_once(source, old, new):
    assert source.count(old) == 1, f"mutation anchor changed: {old!r}"
    return source.replace(old, new, 1)


def damage(methods, kind):
    if kind == "spot_output":
        return replace_once(methods, "spot_.check(x, y, &type, &spot_type);",
                            "spot_.check(x, y, &spot_type, &type);")
    if kind == "primary_spot":
        return replace_once(methods, "spot_.add(0x45, 0, primary.left, primary.top,",
                            "spot_.add(0, 0x45, primary.left, primary.top,")
    if kind == "sprite_owner":
        return replace_once(methods, "sprites_34E0_[slot].extract(&temporary, transparent, 0xA4, 0x6D,",
                            "sprites_33AC_[slot].extract(&temporary, transparent, 0xA4, 0x6D,")
    if kind == "local_scalar":
        return replace_once(methods, "if (player == ConsoleGlobal.faction_id_) {",
                            "if (true) {")
    if kind == "palette_offset":
        return replace_once(methods, "animation.set_palette_offset(10);", "/* damaged palette offset omitted */")
    if kind == "duplicate_close":
        return replace_once(methods, "  preview_faction_or_page_ = ConsoleGlobal.faction_id_;\n  return 0;",
                            "  preview_faction_or_page_ = ConsoleGlobal.faction_id_;\n  animation.close();\n  return 0;")
    if kind == "current_clip":
        # The second restoration must be before image painting. Replacing all
        # restoral calls also catches the pre-branch active/full clip distinction.
        assert methods.count("canvas->restore_clip();") == 2
        return methods.replace("canvas->restore_clip();", "canvas->set_clip(const_cast<RECT *>(&canvas->current_clip()));")
    if kind == "selected_button":
        return replace_once(methods, "    buttons_[1].show(0);", "    buttons_[1].hide();")
    if kind == "sort_hole":
        return replace_once(methods, "if (order[j] == -1) {", "if (order[j] == -1 || order[i - 1] == -1) {")
    if kind == "sparse_victory":
        return replace_once(methods, "record_type += 2;", "record_type += 1;")
    if kind == "parser_owner":
        return replace_once(methods, "parse_says(2, Players[record.owner_faction_].noun_faction,",
                            "parse_says(2, Players[faction].noun_faction,")
    if kind == "parser_grammar":
        start = methods.index("    case 0: {")
        end = methods.index("    case 1: {", start)
        case = replace_once(methods[start:end], "parse_says(4, Players[record.owner_faction_].title_leader,\n                 Players[faction].is_leader_female, -1);",
                            "parse_says(4, Players[record.owner_faction_].title_leader,\n                 Players[record.owner_faction_].is_leader_female, -1);")
        return methods[:start] + case + methods[end:]
    if kind == "secrets_sentinel":
        return replace_once(methods, 'description_section = "SECRETS1STA";',
                            'parse_num(2, record.word_4_);\n      description_section = "SECRETS1STA";')
    if kind == "first_battle_year":
        return replace_once(methods, 'description_section = "BATTLEWIN1ST";',
                            'parse_num(1, FactionMonuments[record.owner_faction_].records_[6].year_);\n      description_section = "BATTLEWIN1ST";')
    if kind == "parser_defaults":
        start = methods.index("    case 1: {")
        end = methods.index("    case 2: {", start)
        case = replace_once(methods[start:end],
                            "GenderDefault = Players[record.owner_faction_].noun_gender;",
                            "GenderDefault = Players[faction].noun_gender;")
        return methods[:start] + case + methods[end:]
    raise AssertionError(kind)


def projection(mutation=None):
    header = (ROOT / "src/monuwin.h").read_text()
    cpp = (ROOT / "src/monuwin.cpp").read_text()
    declaration = braced(header, "class MonuWin : public GraphicWin {", True)
    methods = "\n\n".join(braced(cpp, marker) for marker in METHODS)
    assert len(re.findall(r"(?m)^(?:\w+ )?MonuWin::", methods)) == 9
    if mutation:
        methods = damage(methods, mutation)
    game = (ROOT / "src/game.h").read_text()
    layout = "\n".join(braced(game, "struct " + name + " {", True)
                         for name in ("MonumentRecord", "MonumentFaction"))
    # Essential host-independent record storage is live-source too. Native
    # pointer-sized window ABI assertions remain a separate measurement.
    fixture = FIXTURE.read_text()
    assert fixture.count("__LAYOUT__") == 1
    assert fixture.count("__DECLARATION__") == 1
    assert fixture.count("__METHODS__") == 1
    assert fixture.count("__FLIC_SETTER__") == 1
    flic_setter = braced((ROOT / "src/flic.h").read_text(),
                        "  void set_palette_offset(")
    return (fixture.replace("__LAYOUT__", layout)
            .replace("__DECLARATION__", declaration)
            .replace("__FLIC_SETTER__", flic_setter)
            .replace("__METHODS__", methods))


def compile_and_run(tmp_path, compiler, *, sanitize=False, mutation=None):
    path = tmp_path / "monuwin_control.cpp"
    path.write_text(projection(mutation))
    exe = tmp_path / "monuwin_control"
    flags = ["-std=c++17", "-O1", "-Wall", "-Wextra", "-Wno-unused-parameter"]
    if sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                  "-fno-omit-frame-pointer"]
    built = subprocess.run([compiler, *flags, str(path), "-o", str(exe)],
                           text=True, capture_output=True, timeout=90)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, timeout=30,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("sanitize", (False, True))
def test_live_monuwin_owner_contracts(tmp_path, compiler_name, sanitize):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"host {compiler_name} unavailable")
    run = compile_and_run(tmp_path, compiler, sanitize=sanitize)
    assert run.returncode == 0, run.stdout + run.stderr
    assert "PASS MonuWin host contracts" in run.stdout


@pytest.mark.parametrize("mutation", DAMAGES)
def test_live_monuwin_damage_is_detected(tmp_path, mutation):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    run = compile_and_run(tmp_path, compiler, mutation=mutation)
    assert run.returncode != 0, f"undetected damage: {mutation}"
    assert "FAIL:" in run.stderr, run.stdout + run.stderr
