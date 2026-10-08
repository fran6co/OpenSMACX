"""Actual wrap bodies and NetWin expressions; bounded host dependency controls."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_secret_project_rules_owner_actual_source import extract_function, code_only

ROOT = Path(__file__).resolve().parents[2]

FIXTURE = r'''
#include <cassert>
#include <cstring>
struct Font { void *font_obj_; int unk_1_, line_height_, height_; };
Font font = {0, -1, 10, 8};
Font *FontDefault = &font;
int BufferStrHeight;
int BufferLastWrapWriteResult;
struct RECT { int left, top, right, bottom; };
struct Buffer {
    Font *font1_;
    int field_10_, field_14_, field_18_, split_count, writer_count;
    char *find_line_break_l(char *text, int *, int length) {
        ++split_count;
        return split_count == 1 && length > 2 ? text + 2 : 0;
    }
    int write_multi_font_raw_l(char *, int x, int, int length) {
        ++writer_count; return x + length;
    }
    int wrap(char *, int, int, int);
    int wrap(char *, RECT *);
    int text_line_height() { return 10; }
};
// ACTUAL_WRAP_FOUR
// ACTUAL_WRAP_RECT
struct Arrow {
    int x, y;
    int transparent_index() { return 0; }
    void draw(Buffer *, int, int px, int py) { x=px; y=py; }
};
void actual_netwin_expressions(Buffer *buffer, RECT *panel, Arrow &arrow) {
    char StringTemp[]="abcdef";
    int col=20, row=100;
    // ACTUAL_NETWIN_CALL
    // ACTUAL_NETWIN_ARROW
    assert(bottom==120);
}
int main() {
    // Process-static zero initialization, not a per-call reset in product.
    assert(BufferLastWrapWriteResult==0);
    font.font_obj_=&font;
    Buffer buffer={&font, 0, 0, 0, 0, 0};
    RECT panel={0,0,100,100}; Arrow arrow={0,0};
    actual_netwin_expressions(&buffer,&panel,arrow);
    assert(BufferLastWrapWriteResult==24 && buffer.writer_count==2);
    assert(arrow.x==27 && arrow.y==110 && buffer.field_18_==2);

    char text[]="abcdef"; RECT rect={20,100,90,200};
    Buffer inset={&font,5,0,0,0,0};
    assert(inset.wrap(text,&rect)==120);
    assert(BufferLastWrapWriteResult==29);
    assert(inset.field_14_==0 && inset.field_18_==2);

    // The original no-glyph branch stores current x and still advances y.
    char empty[]="";
    Buffer empty_rect={&font,5,0,0,0,0};
    assert(empty_rect.wrap(empty,&rect)==110);
    assert(BufferLastWrapWriteResult==20 && empty_rect.writer_count==0);
    assert(empty_rect.field_14_==0 && empty_rect.field_18_==1);

    Buffer empty_four={&font,0,0,0,0,0};
    assert(empty_four.wrap(empty,20,100,70)==110);
    assert(BufferLastWrapWriteResult==20 && empty_four.writer_count==0);

    // Negative carry remains a signed adjustment, not a clamped inset.
    Buffer negative_carry={&font,-5,0,0,0,0};
    assert(negative_carry.wrap(text,&rect)==120);
    assert(BufferLastWrapWriteResult==19 && negative_carry.writer_count==2);
    assert(negative_carry.field_14_==0 && negative_carry.field_18_==2);

    // Nonnegative first Font word selects height + that word, not line_height.
    font.unk_1_=3;
    Buffer alternate_height={&font,0,0,0,0,0};
    assert(alternate_height.wrap(text,&rect)==122);
    assert(BufferLastWrapWriteResult==24);
    font.unk_1_=-1;

    // A non-null font without its provider takes the original sentinel path.
    // This does not establish reachability of that dependency state in game.
    font.font_obj_=0;
    Buffer failed={&font,0,0,0,0,0};
    actual_netwin_expressions(&failed,&panel,arrow);
    assert(BufferLastWrapWriteResult==3 && failed.writer_count==0);
    assert(arrow.x==6 && arrow.y==110);
    Buffer failed_rect={&font,0,0,0,0,0};
    assert(failed_rect.wrap(text,&rect)==120);
    assert(BufferLastWrapWriteResult==3 && failed_rect.field_18_==2);

    Buffer default_font={0,0,0,0,0,0};
    assert(default_font.wrap(text,&rect)==120);
    assert(default_font.font1_==FontDefault && BufferLastWrapWriteResult==3);

    BufferLastWrapWriteResult=77;
    assert(buffer.wrap(0,20,33,70)==33);
    assert(buffer.wrap(text,20,33,0)==33);
    assert(BufferLastWrapWriteResult==77);
}
'''


def fixture_source():
    source = (ROOT / "src/buffer.cpp").read_text()
    four = extract_function(source, "int Buffer::wrap(char * a1, int a2, int a3, int a4)")
    rect = extract_function(source, "int Buffer::wrap(char * a1, RECT * a2)")
    netwin = extract_function((ROOT / "src/netwin.cpp").read_text(),
                             "void NetWin::draw_session_info()")
    call = re.search(r"int bottom\s*=\s*buffer->wrap\(StringTemp,\s*col,\s*row,\s*"
                     r"panel->right\s*-\s*panel->left\s*-\s*0x1E\s*\);", netwin)
    arrow = re.search(r"arrow\.draw\(buffer,\s*arrow\.transparent_index\(\),\s*"
                      r"BufferLastWrapWriteResult\s*\+\s*3,\s*"
                      r"bottom\s*-\s*buffer->text_line_height\(\)\s*\);", netwin)
    assert call and arrow
    return (FIXTURE.replace("// ACTUAL_WRAP_FOUR", four)
            .replace("// ACTUAL_WRAP_RECT", rect)
            .replace("// ACTUAL_NETWIN_CALL", call.group(0))
            .replace("// ACTUAL_NETWIN_ARROW", arrow.group(0)))


def test_one_shared_owner_and_no_fixed_result_aliases():
    buffer = (ROOT / "src/buffer.cpp").read_text()
    netwin = (ROOT / "src/netwin.cpp").read_text()
    assert code_only(buffer).count("int BufferLastWrapWriteResult;") == 1
    assert "extern int BufferLastWrapWriteResult;" in (ROOT / "src/buffer.h").read_text()
    assert "RECOVERY_OBJECT: v1 0x009B39D8 ?BufferLastWrapWriteResult@@3HA" in buffer
    assert "BufferWriteStatus" not in code_only(buffer)
    assert "NetWinWrapX" not in code_only(netwin)
    rect = extract_function(buffer, "int Buffer::wrap(char * a1, RECT * a2)")
    assert "reinterpret_cast<int *>(0x14)" not in rect
    assert "reinterpret_cast<int *>(0x18)" not in rect


def test_font_first_buffer_methods_use_named_slots():
    buffer = (ROOT / "src/buffer.cpp").read_text()
    signatures = (
        "int Buffer::write_cent_l(Font* a1, char * a2, RECT* a3, int a4)",
        "int Buffer::wrap_cent(Font *a1, char *a2, int a3, int a4, int a5, int a6)",
        "int Buffer::write_l(Font * font, char * text, int x, int y, int maxLen)",
        "int Buffer::wrap_right(Font * a1, char * a2, int a3, int a4, int a5)",
        "int Buffer::wrap(Font *a1, char *a2, RECT *a3)",
    )
    bodies = [extract_function(buffer, signature) for signature in signatures]
    assert all("reinterpret_cast<BufferFontTriple *>" not in body for body in bodies)
    assert all("reinterpret_cast<int *>(&font1_)" not in body for body in bodies)
    assert "reinterpret_cast<Font **>(reinterpret_cast<char *>(this) + 0x52c)" not in buffer
    assert "font1_ = a1;" in bodies[0] and "font2_ = 0;" in bodies[0]
    assert "font3_ = 0;" in bodies[0]
    assert "Font *const *const fonts = &font1_;" in buffer
    assert buffer.count("Font *const *fontArray = &font1_;") == 2


FONT_WRAPPER_FIXTURE = r'''
#include <cassert>
#include <cstring>
struct Font { void *font_obj_; };
struct RECT { int left, top, right, bottom; };
struct BufferFontTriple { Font *primary, *second, *third; };
Font replacement = {&replacement}, invalid = {0};
Font mutation1 = {0}, mutation2 = {0}, mutation3 = {0};
Font *expected1, *expected2, *expected3;
int observed, findCalls, writeCentCalls, writeCalls, rightCalls, rectCalls;
int BufferStrHeight;
struct Buffer {
    Font *font1_, *font2_, *font3_;
    int field_18_;
    int check_slots() {
        assert(font1_ == expected1 && font2_ == expected2 && font3_ == expected3);
        ++observed;
        return 0;
    }
    void mutate_slots() { font1_=&mutation1; font2_=&mutation2; font3_=&mutation3; }
    char *find_line_break_l(char *text, int *width, unsigned int len) {
        check_slots(); assert(width && *width == 31 && len > 0);
        ++findCalls; return findCalls == 1 ? text + 2 : 0;
    }
    int write_cent_l(char *text, int x, int y, int width, unsigned int len) {
        check_slots(); assert(text && x == 21 && width == 31 && len == 2);
        assert(y == 40 || y == 45); ++writeCentCalls;
        if (writeCentCalls == 2) mutate_slots(); return 0;
    }
    int write_cent_l(char *text, RECT *rect, int value) {
        check_slots(); assert(text == (char *)0x1234 && rect == (RECT *)0x2345 && value == 19);
        mutate_slots(); return 73;
    }
    int wrap_right(char *text, int a, int b, int c) {
        check_slots(); assert(text == (char *)0x3456 && a == 7 && b == 8 && c == 9);
        ++rightCalls; mutate_slots(); return 81;
    }
    int wrap(char *text, RECT *rect) {
        check_slots(); assert(text == (char *)0x4567 && rect == (RECT *)0x5678);
        ++rectCalls; mutate_slots(); return 92;
    }
    int write_multi_font_raw_l(char *text, int x, int y, int len) {
        check_slots(); assert(text && strcmp(text, "four") == 0 && x == 11 && y == 12 && len == 4);
        ++writeCalls; mutate_slots(); return 104;
    }
    int wrap_right(Font *, char *, int, int, int);
    int wrap_cent(Font *, char *, int, int, int, int);
    int write_l(Font *, char *, int, int, int);
    int write_cent_l(Font *, char *, RECT *, int);
    int wrap(Font *, char *, RECT *);
};
// ACTUAL_FONT_WRAPPERS
void expect(Font *a, Font *b, Font *c) { expected1=a; expected2=b; expected3=c; }
void reset_counts() { observed=findCalls=writeCentCalls=writeCalls=rightCalls=rectCalls=0; }
void check_restored(Buffer &b, Font *a, Font *c, Font *d) {
    assert(b.font1_==a && b.font2_==c && b.font3_==d);
}
int main() {
    char text[]="four"; RECT rect={1,2,3,4};
    Font old1={&old1}, old2={0}, old3={0};
    Buffer b={&old1,&old2,&old3,0};

    // Valid replacement is visible to each actual inner call; inner mutation
    // proves the wrapper restores all three original slots after the call.
    expect(&replacement,0,0);
    assert(b.write_cent_l(&replacement,(char *)0x1234,(RECT *)0x2345,19)==73);
    check_restored(b,&old1,&old2,&old3);
    expect(&replacement,0,0); reset_counts();
    assert(b.wrap_cent(&replacement,text,21,40,31,5)==50);
    assert(findCalls==2 && writeCentCalls==2 && observed==4);
    check_restored(b,&old1,&old2,&old3);
    expect(&replacement,0,0);
    assert(b.wrap_right(&replacement,(char *)0x3456,7,8,9)==81 && rightCalls==1);
    check_restored(b,&old1,&old2,&old3);
    expect(&replacement,0,0);
    assert(b.wrap(&replacement,(char *)0x4567,(RECT *)0x5678)==92 && rectCalls==1);
    check_restored(b,&old1,&old2,&old3);
    expect(&replacement,0,0);
    assert(b.write_l(&replacement,text,11,12,4)==104 && writeCalls==1);
    check_restored(b,&old1,&old2,&old3);

    // Null and invalid candidate fonts leave the entry slots installed while
    // each delegated body runs, then preserve them after inner mutation.
    expect(&old1,&old2,&old3);
    assert(b.write_cent_l(0,(char *)0x1234,(RECT *)0x2345,19)==73);
    check_restored(b,&old1,&old2,&old3);
    expect(&old1,&old2,&old3);
    assert(b.wrap_right(&invalid,(char *)0x3456,7,8,9)==81);
    check_restored(b,&old1,&old2,&old3);
    expect(&old1,&old2,&old3);
    assert(b.wrap(&invalid,(char *)0x4567,(RECT *)0x5678)==92);
    check_restored(b,&old1,&old2,&old3);
    expect(&old1,&old2,&old3); reset_counts();
    assert(b.wrap_cent(0,text,21,40,31,5)==50);
    assert(findCalls==2 && writeCentCalls==2);
    check_restored(b,&old1,&old2,&old3);
    expect(&old1,&old2,&old3);
    assert(b.write_l(&invalid,text,11,12,4)==104);
    check_restored(b,&old1,&old2,&old3);

    // write_l's null-text and unusable-primary error paths do not delegate,
    // return their historical values, and still restore the three slots.
    expect(&replacement,0,0);
    assert(b.write_l(&replacement,0,33,12,4)==33);
    check_restored(b,&old1,&old2,&old3);
    expect(&old1,&old2,&old3);
    old1.font_obj_=0;
    assert(b.write_l(0,text,44,12,4)==3);
    check_restored(b,&old1,&old2,&old3);
    old1.font_obj_=&old1;
}
'''


def font_wrapper_source(mutant=None):
    source = (ROOT / "src/buffer.cpp").read_text()
    signatures = (
        "int Buffer::write_cent_l(Font* a1, char * a2, RECT* a3, int a4)",
        "int Buffer::wrap_cent(Font *a1, char *a2, int a3, int a4, int a5, int a6)",
        "int Buffer::write_l(Font * font, char * text, int x, int y, int maxLen)",
        "int Buffer::wrap_right(Font * a1, char * a2, int a3, int a4, int a5)",
        "int Buffer::wrap(Font *a1, char *a2, RECT *a3)",
    )
    bodies = [extract_function(source, signature) for signature in signatures]
    if mutant == "omit_restore":
        assert "font3_ = old2;" in bodies[0]
        bodies[0] = bodies[0].replace("font3_ = old2;", "", 1)
    elif mutant == "wrong_restore":
        assert "font3_ = savedFonts.third;" in bodies[2]
        bodies[2] = bodies[2].replace("font3_ = savedFonts.third;", "font3_ = savedFonts.second;", 1)
    return FONT_WRAPPER_FIXTURE.replace("// ACTUAL_FONT_WRAPPERS", "\n".join(bodies))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_font_first_wrappers_restore_slots(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    executable = tmp_path / "font_wrappers"
    source = tmp_path / "font_wrappers.cpp"
    source.write_text(font_wrapper_source())
    built = subprocess.run([compiler, "-std=c++11", str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutant", ("omit_restore", "wrong_restore"))
def test_actual_font_wrapper_control_detects_restore_mutants(compiler_name, mutant, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    executable = tmp_path / "font_wrappers_mutant"
    source = tmp_path / "font_wrappers_mutant.cpp"
    source.write_text(font_wrapper_source(mutant))
    built = subprocess.run([compiler, "-std=c++11", str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode != 0


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "shared_result", "arrow_axis", "sentinel", "counter"))
def test_actual_wrap_and_consumer_dependencies(compiler_name, damage, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = fixture_source()
    if damage == "shared_result":
        assert source.count("BufferLastWrapWriteResult = wrapResult;") == 1
        source = source.replace("BufferLastWrapWriteResult = wrapResult;",
                                "(void)wrapResult; BufferLastWrapWriteResult = a3;", 1)
    elif damage == "arrow_axis":
        source = source.replace("BufferLastWrapWriteResult + 3", "bottom + 3")
    elif damage == "sentinel":
        source = source.replace("BufferLastWrapWriteResult = 3;",
                                "BufferLastWrapWriteResult = 0;")
        source = source.replace("wrapResult = 3;", "wrapResult = 0;")
        source = source.replace("lineResult = 3;", "lineResult = 0;")
    elif damage == "counter":
        assert source.count("field_18_++;") == 1
        source = source.replace("field_18_++;", "field_18_=0;", 1)
    exe = tmp_path / "wrap_control"
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-x", "c++", "-o", str(exe), "-"], input=source,
                           text=True, capture_output=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], text=True, capture_output=True)
    assert (ran.returncode == 0) == (damage is None), ran.stderr
