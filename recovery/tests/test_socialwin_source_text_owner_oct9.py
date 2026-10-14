"""Actual-source controls for SocialWin's fourteen source-owned text arrays."""
from pathlib import Path
import hashlib
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/socialwin.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"

EXPECTED = [
    (0x0068749C, "SOC"),
    (0x006874A0, "+"),
    (0x006874A4, "-"),
    (0x006874A8, "+"),
    (0x006874AC, "+"),
    (0x006874B0, "socwin.pcx"),
    (0x006874BC, "SOCWIN"),
    (0x00682E90, ", "),
    (0x00682E94, ": "),
    (0x00687480, "% "),
    (0x00687484, "% "),
    (0x00687488, "% "),
    (0x0068748C, "% "),
    (0x00687490, "+"),
]

ARRAYS = [
    "SocialWinOverviewSocText", "SocialWinOverviewPlusText",
    "SocialWinMouseoverMinusText", "SocialWinMouseoverPlusText",
    "SocialWinButtonCountPlusText", "SocialWinPcxName",
    "SocialWinTutorialName", "SocialWinCommaSpaceText",
    "SocialWinColonSpaceText", "SocialWinPercentSpaceTextA",
    "SocialWinPercentSpaceTextB", "SocialWinPercentSpaceTextC",
    "SocialWinPercentSpaceTextD", "SocialWinUpheavalPlusText",
]


def function_body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def statement(body: str, fragment: str) -> str:
    for line in body.splitlines():
        if fragment in line:
            return line.strip()
    raise AssertionError(f"missing source statement: {fragment}")


def source_parts():
    text = SOURCE.read_text()
    start = text.index("// These writable arrays are the source-owned versions")
    end = text.index("\n// ORIGINAL: 0x004AEF10", start)
    declarations = text[start:end]

    bodies = {
        "overview": function_body(text, "void SocialWin::draw_overview()"),
        "button": function_body(text, "int SocialWin::on_button_text_draw("),
        "mouseover": function_body(text, "void SocialWin::draw_mouseover()"),
        "info": function_body(text, "void SocialWin::draw_info()"),
        "init": function_body(text, "void SocialWin::init("),
    }
    refs = {
        "section": statement(bodies["overview"], "strcat(StringTemp, SocialWinOverviewSocText)"),
        "overview_plus": statement(bodies["overview"], "strcat(StringTemp, SocialWinOverviewPlusText)"),
        "count_plus": statement(bodies["button"], "strcat(StringTemp, SocialWinButtonCountPlusText)"),
        "minus": statement(bodies["mouseover"], "strcat(StringTemp, SocialWinMouseoverMinusText)"),
        "mouseover_plus": statement(bodies["mouseover"], "strcat(StringTemp, SocialWinMouseoverPlusText)"),
        "comma": statement(bodies["info"], "strcat(StringTemp, SocialWinCommaSpaceText)"),
        "colon": statement(bodies["info"], "strcat(StringTemp, SocialWinColonSpaceText)"),
        "percent_a": statement(bodies["info"], "strcat(StringTemp, SocialWinPercentSpaceTextA)"),
        "percent_b": statement(bodies["info"], "strcat(StringTemp, SocialWinPercentSpaceTextB)"),
        "percent_c": statement(bodies["info"], "strcat(StringTemp, SocialWinPercentSpaceTextC)"),
        "percent_d": statement(bodies["info"], "strcat(StringTemp, SocialWinPercentSpaceTextD)"),
        "upheaval_plus": statement(bodies["info"], "strcat(StringTemp, SocialWinUpheavalPlusText)"),
        "load": statement(bodies["init"], "sheet.load_pcx(SocialWinPcxName"),
        "tutorial": statement(bodies["init"], "TutorialWindow.tut_win(graphics, SocialWinTutorialName"),
    }

    assert "SocialWinEffects" not in text
    assert "strcat(StringTemp, SocialEffects[effect].set1);" in bodies["overview"]
    assert "static const char *const SocialOverviewSection = \"SOCIO\";" in text
    assert "(char *)0x006874" not in text
    assert "(char *)0x00682E90" not in text
    for name in ARRAYS:
        assert declarations.count(f"char {name}[]") == 1
    return declarations, refs


HARNESS = r'''#include <cstdio>
#include <cstdlib>
#include <cstring>
DECLARATIONS
static char StringTemp[256];
static void fail(const char *message) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
static void run_actual_consumer_statements() {
    StringTemp[0] = 0;
    SECTION
    std::strcat(StringTemp, "IAL");
    if (std::strcmp(StringTemp, "SOCIAL") != 0) fail("overview SOC path prefix");

    StringTemp[0] = 0;
    if (1) { OVERVIEW_PLUS }
    std::strcat(StringTemp, "5");
    if (std::strcmp(StringTemp, "+5") != 0) fail("overview positive prefix");

    StringTemp[0] = 0;
    if (1) { COUNT_PLUS }
    std::strcat(StringTemp, "3");
    if (std::strcmp(StringTemp, "+3") != 0) fail("button count prefix");

    StringTemp[0] = 0;
    MINUS
    std::strcat(StringTemp, "2");
    if (std::strcmp(StringTemp, "-2") != 0) fail("mouseover negative prefix");
    StringTemp[0] = 0;
    MOUSEOVER_PLUS
    std::strcat(StringTemp, "2");
    if (std::strcmp(StringTemp, "+2") != 0) fail("mouseover positive prefix");

    StringTemp[0] = 0;
    COMMA COLON PERCENT_A PERCENT_B PERCENT_C PERCENT_D UPHEAVAL_PLUS
    if (std::strcmp(StringTemp, ", : % % % % +") != 0)
        fail("info separator and distinct percent spellings");
}
int main() {
    char *values[] = { ARRAY_VALUES };
    const char *expected[] = { EXPECTED_VALUES };
    for (unsigned i = 0; i < sizeof(values) / sizeof(values[0]); ++i) {
        if (std::strcmp(values[i], expected[i]) != 0) fail("array initial contents");
        for (unsigned j = 0; j < i; ++j)
            if (values[i] == values[j]) fail("duplicate text identities merged");
        const char first = values[i][0];
        values[i][0] = static_cast<char>(first ^ 1);
        values[i][0] = first;
    }
    run_actual_consumer_statements();
    return 0;
}
'''


def fixture(declarations, refs):
    source = HARNESS.replace("DECLARATIONS", declarations)
    for key, token in {
        "section": "SECTION", "overview_plus": "OVERVIEW_PLUS",
        "count_plus": "COUNT_PLUS", "minus": "MINUS",
        "mouseover_plus": "MOUSEOVER_PLUS", "comma": "COMMA",
        "colon": "COLON", "percent_a": "PERCENT_A",
        "percent_b": "PERCENT_B", "percent_c": "PERCENT_C",
        "percent_d": "PERCENT_D", "upheaval_plus": "UPHEAVAL_PLUS",
    }.items():
        source = source.replace(token, refs[key])
    source = source.replace("ARRAY_VALUES", ", ".join(ARRAYS))
    source = source.replace("EXPECTED_VALUES", ", ".join(f'"{value}"' for _, value in EXPECTED))
    return source


COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


def compile_run(tmp_path, compiler, source):
    cpp = tmp_path / "socialwin-source-text.cpp"
    binary = tmp_path / "socialwin-source-text"
    cpp.write_text(source)
    build = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(binary)], capture_output=True,
                           text=True, timeout=20)
    if build.returncode != 0:
        return build, None
    return build, subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_socialwin_text_owners_and_consumers(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    declarations, refs = source_parts()
    build, run = compile_run(tmp_path, compiler, fixture(declarations, refs))
    assert build.returncode == 0, build.stderr
    assert run is not None and run.returncode == 0, run.stderr if run else "no executable"


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_soc_path_content_mutant_is_detected(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    declarations, refs = source_parts()
    refs["section"] = "strcat(StringTemp, SocialWinTutorialName);"
    build, run = compile_run(tmp_path, compiler, fixture(declarations, refs))
    assert build.returncode == 0, build.stderr
    assert run is not None and run.returncode != 0
    assert "overview SOC path prefix" in run.stderr


def test_pinned_text_bytes_and_duplicate_locations():
    image_path = Path("/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe")
    if not image_path.is_file():
        pytest.skip("pinned executable unavailable; host controls remain independent")
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    data = image_path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    for address, value in EXPECTED:
        assert image.read(Span(address, address + len(value) + 1)) == value.encode() + b"\0"


def test_socialwin_init_uses_typed_console_parent_and_real_font_globals(tmp_path):
    text = SOURCE.read_text()
    init = function_body(text, "void SocialWin::init(")
    call_start = init.index("GraphicWin::init(cx, cy, 0x341, 0x162, 0, 0x1000000,")
    call_end = init.index("0, 0);", call_start) + len("0, 0);")
    init_call = init[call_start:call_end]
    assert "static_cast<Win *>(&ConsoleGlobal)" in init_call
    assert "ConsoleIfaceSlot" not in text
    assert "#include \"interface_fonts.h\"" in text
    assert "FontNameArialSlot" not in text and "FontNameNarrowSlot" not in text
    expected_fonts = [
        "font1_.init(DefaultFontFace, 0xe, 1);",
        "font2_.init(DefaultFontFace, 0xe, 1);",
        "font3_.init(DefaultLargeFontFace, 0xe, 1);",
        "font4_.init(DefaultLargeFontFace, 0xe, 1);",
        "font5_.init(DefaultFontFace, 0xc, 0);",
        "font6_.init(DefaultFontFace, 0xb, 1);",
        "font7_.init(DefaultFontFace, 0xb, 0);",
        "font8_.init(DefaultLargeFontFace, 0xe, 1);",
        "font9_.init(DefaultLargeFontFace, 0xe, 1);",
    ]
    positions = [init.index(line) for line in expected_fonts]
    assert positions == sorted(positions) and len(set(positions)) == 9
    fonts = (ROOT / "src/interface_fonts.h").read_text()
    assert "extern LPCSTR DefaultFontFace;" in fonts
    assert "extern LPCSTR DefaultLargeFontFace;" in fonts

    font_harness = r'''#include <cstdlib>
#include <cstring>
typedef const char *LPCSTR;
LPCSTR DefaultFontFace, DefaultLargeFontFace;
const char *arial[10], *large[10];
int font_call;
struct Font { const char *face; int size, mode;
  void init(LPCSTR f, int s, int m) {
    face=f; size=s; mode=m;
    ++font_call;
    DefaultFontFace=arial[font_call]; DefaultLargeFontFace=large[font_call];
  }
};
Font font1_,font2_,font3_,font4_,font5_,font6_,font7_,font8_,font9_;
void run() { __ACTUAL_FONT_CALLS__ }
int main() {
  for (int i=0;i<10;++i) { arial[i]=i==0?"arial0":"arial-next"; large[i]=i==0?"large0":"large-next"; }
  DefaultFontFace=arial[0]; DefaultLargeFontFace=large[0]; run();
  Font *items[]={&font1_,&font2_,&font3_,&font4_,&font5_,&font6_,&font7_,&font8_,&font9_};
  const char *faces[]={"arial0","arial-next","large-next","large-next","arial-next","arial-next","arial-next","large-next","large-next"};
  if (font_call!=9) return 1;
  for (int i=0;i<9;++i) if (std::strcmp(items[i]->face,faces[i])!=0) return 2+i;
  if (font1_.size!=0xe || font1_.mode!=1 || font5_.size!=0xc || font5_.mode!=0 ||
      font6_.size!=0xb || font6_.mode!=1 || font7_.size!=0xb || font7_.mode!=0) return 20;
}
'''.replace("__ACTUAL_FONT_CALLS__", "\n".join(expected_fonts))
    for compiler in COMPILERS:
        build, run_result = compile_run(tmp_path, compiler, font_harness)
        assert build.returncode == 0, build.stderr
        assert run_result is not None and run_result.returncode == 0, run_result.stderr

    harness = r'''#include <cstdlib>
#include <cstring>
struct Win { int marker; };
struct Buffer { int marker; };
struct GraphicWin : Win, Buffer { static int cx, cy, w, h, x, y, flags; static Win *parent;
  static void init(int a,int b,int c,int d,int e,int f,Win *g,int i,int j) {
    cx=a;cy=b;w=c;h=d;x=e;flags=f;parent=g;y=i;(void)j;
  }
};
int GraphicWin::cx,GraphicWin::cy,GraphicWin::w,GraphicWin::h,GraphicWin::x,GraphicWin::y,GraphicWin::flags;
Win *GraphicWin::parent;
struct MapWin : virtual GraphicWin {};
struct Console : MapWin {};
Console ConsoleGlobal;
int cx=7, cy=9;
void run() { __ACTUAL_INIT_CALL__ }
int main() {
  run();
  Win *expected=static_cast<Win *>(&ConsoleGlobal);
  if (GraphicWin::parent != expected || GraphicWin::parent == reinterpret_cast<Win *>(0x1000000)) return 1;
  if (GraphicWin::flags != 0x1000000 || GraphicWin::cx!=7 || GraphicWin::cy!=9 ||
      GraphicWin::w!=0x341 || GraphicWin::h!=0x162 || GraphicWin::x!=0 || GraphicWin::y!=0) return 2;
  if (static_cast<void *>(&ConsoleGlobal) == static_cast<void *>(expected)) return 3;
}
'''.replace("__ACTUAL_INIT_CALL__", init_call)
    for compiler in COMPILERS:
        build, run_result = compile_run(tmp_path, compiler, harness)
        assert build.returncode == 0, build.stderr
        assert run_result is not None and run_result.returncode == 0, run_result.stderr
