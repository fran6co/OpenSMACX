"""Bounded host controls for the installed Win callback contract closure.

Bodies come from current source owners. Host doubles do not qualify native
ABI, generated receiver adjustments, linked tables, or whole-game behavior.
"""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_file, read_text
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())


def extract_function(source: str, signature: str) -> str:
    masked = mask_cpp_noncode(source)
    starts = []
    offset = 0
    while True:
        found = masked.find(signature, offset)
        if found < 0:
            break
        starts.append(found)
        offset = found + 1
    assert len(starts) == 1, f"expected one code-visible body for {signature}"
    start = starts[0]
    opening = masked.find("{", start + len(signature))
    assert opening >= 0, f"no code-visible body for {signature}"
    depth = 0
    i = opening
    while i < len(masked):
        if masked[i] == "{":
            depth += 1
        elif masked[i] == "}":
            depth -= 1
            if depth == 0:
                return source[start:i + 1]
        i += 1
    raise AssertionError(f"unterminated body: {signature}")


def mask_cpp_noncode(source: str) -> str:
    """Use the package lexical mask for this VC6-compatible source."""
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )


POPUP_SIGS = (
    "void Popup::on_scrolled(int first, int second)",
    "void Popup::on_scrolling(int first, int second)",
)
SELECT_SIGS = (
    "void SelectPartWin::on_scrolled(int, int row)",
    "void SelectPartWin::on_scrolling(int a1, int a2)",
)
DIALOG_SIGS = (
    "void Dialogs::on_scrolled(int code, int pos)",
    "void Dialogs::on_scrolling(int a, int b)",
)
ALPHA_SIGS = (
    "void AlphaSave::on_scrolled(int code, int pos)",
    "void AlphaSave::on_scrolling(int code, int pos)",
)
STRING_SIGS = (
    "void StringBox::on_scrolled(int code, int pos)",
    "void StringBox::on_scrolling(int code, int position)",
)
REPLAY_SIG = "void ReplayWin::on_right_double_click(int, int)"

# Original identities covered by this owner batch. Historical records and their
# claims/comments are parsed from the fixed baseline commit at test time rather
# than duplicated as a hash catalogue. Current symbol facts are intentionally
# excluded: candidate symbols require fresh COFF observation and writer output.
BASELINE_COMMIT = "7db715fbc"
BASELINE_SCOPE = {
    "src/alphasave.cpp": (0x004080E0, 0x00408110),
    "src/basewin.cpp": (0x0041B4A0, 0x0041B4C0),
    "src/dialogs.cpp": (0x00612EF0, 0x00612F40, 0x00612F80, 0x00612FA0),
    "src/gamma.cpp": (0x005C9390, 0x005C93A0),
    "src/listbox.cpp": (0x0060C5D0, 0x0060C6A0, 0x0060C6D0),
    "src/popup.cpp": (0x00406B70, 0x00406BA0),
    "src/prodpicker.cpp": (0x00493CB0, 0x00493E70),
    "src/replaywin.cpp": (0x005AD3E0,),
    "src/selectpartwin.cpp": (0x00433F30, 0x00433F50),
    "src/spritebox.cpp": (0x00611290, 0x00611330),
    "src/stringbox.cpp": (0x00629D20, 0x00629D40),
}


def actual_bodies():
    popup = (ROOT / "src/popup.cpp").read_text(encoding="utf-8")
    dialogs = (ROOT / "src/dialogs.cpp").read_text(encoding="utf-8")
    select = (ROOT / "src/selectpartwin.cpp").read_text(encoding="utf-8")
    alpha = (ROOT / "src/alphasave.cpp").read_text(encoding="utf-8")
    stringbox = (ROOT / "src/stringbox.cpp").read_text(encoding="utf-8")
    replaywin = (ROOT / "src/replaywin.cpp").read_text(encoding="utf-8")
    return [extract_function(popup, sig) for sig in POPUP_SIGS] + \
           [extract_function(dialogs, sig) for sig in DIALOG_SIGS] + \
           [extract_function(select, sig) for sig in SELECT_SIGS] + \
           [extract_function(alpha, sig) for sig in ALPHA_SIGS] + \
           [extract_function(stringbox, sig) for sig in STRING_SIGS] + \
           [extract_function(replaywin, REPLAY_SIG)]


def source_comments(region: str) -> tuple[str, ...]:
    """Collect comments in order, ignoring literals and current symbol facts."""
    found = []
    for match in COMMENTS_OR_STRINGS.finditer(region):
        comment = match[0]
        if not comment.startswith(("//", "/*")):
            continue
        # Symbol facts can also be nested in a legacy block comment. Preserve
        # every other line, including original identity and structural evidence.
        lines = [line for line in comment.splitlines(keepends=True)
                 if not line.lstrip().startswith("// symbol ")]
        normalized = "".join(lines)
        if normalized:
            found.append(normalized)
    return tuple(found)


def preserved_record(row) -> tuple:
    """Historical source facts only; intentionally excludes parsed symbols."""
    return (
        row.address,
        row.name,
        tuple(row.image_spans),
        row.mode.value,
        row.byte_exact,
        row.semantic,
        row.kind,
        row.exclusion,
        tuple(row.levers),
        tuple(row.ruled_out),
        tuple(row.unrecoverable),
        tuple(row.deferred),
        row.source_role.value,
        row.source_role_reason,
        source_comments(row.region),
    )


HOST_TYPES = r"""
#include <cassert>
struct Win {
    int base_scrolled, base_scrolling, redraws, right_double_clicks;
    Win(): base_scrolled(0), base_scrolling(0), redraws(0), right_double_clicks(0) {}
    virtual ~Win() {}
    virtual void on_scrolled(int, int) { ++base_scrolled; }
    virtual void on_scrolling(int, int) { ++base_scrolling; }
    virtual void on_right_double_click(int, int) { ++right_double_clicks; }
    virtual void redraw() { ++redraws; }
};
struct GraphicWin : Win {};
struct ReplayWin : GraphicWin { void on_right_double_click(int, int); };
struct ListBox : virtual GraphicWin {
    int scrolled_calls, scrolling_calls, code, position;
    ListBox(): scrolled_calls(0), scrolling_calls(0), code(0), position(0) {}
    virtual void on_scrolled(int a, int b) {
        ++scrolled_calls; code = a; position = b;
    }
    virtual void on_scrolling(int a, int b) {
        ++scrolling_calls; code = a; position = b;
    }
};
struct Dialogs : ListBox {
    int kind_;
    Dialogs(): kind_(2) {}
    void on_scrolled(int, int) override;
    void on_scrolling(int, int) override;
};
struct BasePop : GraphicWin {
    Dialogs dialog;
    Dialogs &dialogs() { return dialog; }
};
struct Popup : BasePop {
    void on_scrolled(int, int);
    void on_scrolling(int, int);
};
struct SelectPartWin : GraphicWin {
    int field_2B60_, field_2B6C_;
    SelectPartWin(): field_2B60_(0), field_2B6C_(1) {}
    void on_scrolled(int, int) override;
    void on_scrolling(int, int) override;
};
struct DerivedSelect : SelectPartWin {
    int calls, code, row;
    DerivedSelect(): calls(0), code(0), row(0) {}
    void on_scrolled(int a, int b) override { ++calls; code = a; row = b; }
};
struct FileWin { ListBox list_box_; };
struct AlphaSave : GraphicWin {
    FileWin file_win_;
    void on_scrolled(int, int) override;
    void on_scrolling(int, int) override;
};
struct StringBox : GraphicWin {
    int top_line_;
    StringBox(): top_line_(-1) {}
    void on_scrolled(int, int) override;
    void on_scrolling(int, int) override;
};
"""


MAIN = r"""
int main() {
    Popup popup;
    Win *popup_view = &popup;
    popup_view->on_scrolled(17, 29);
    assert(popup.dialog.scrolled_calls == 0 && popup.dialog.scrolling_calls == 1 &&
           popup.dialog.code == 17 && popup.dialog.position == 29 &&
           popup.base_scrolled == 0);
    popup_view->on_scrolling(31, 43);
    assert(popup.dialog.scrolling_calls == 2 && popup.dialog.code == 31 &&
           popup.dialog.position == 43 && popup.base_scrolling == 0);

    SelectPartWin select;
    select.field_2B6C_ = 5;
    Win *select_view = &select;
    select_view->on_scrolled(7, 3);
    assert(select.field_2B60_ == 15 && select.redraws == 1);
    select_view->on_scrolling(9, 4);
    assert(select.field_2B60_ == 20 && select.redraws == 2);
    DerivedSelect derived;
    Win *derived_view = &derived;
    derived_view->on_scrolling(13, 6);
    assert(derived.calls == 1 && derived.code == 13 && derived.row == 6);
    assert(derived.field_2B60_ == 0 && derived.redraws == 0);

    AlphaSave alpha;
    Win *alpha_view = &alpha;
    alpha_view->on_scrolled(5, 8);
    assert(alpha.file_win_.list_box_.scrolled_calls == 1 &&
           alpha.file_win_.list_box_.scrolling_calls == 0);
    alpha_view->on_scrolling(11, 14);
    assert(alpha.file_win_.list_box_.scrolling_calls == 1 &&
           alpha.file_win_.list_box_.code == 11 &&
           alpha.file_win_.list_box_.position == 14);

    StringBox string_box;
    Win *string_view = &string_box;
    string_view->on_scrolling(19, 23);
    assert(string_box.top_line_ == 23 && string_box.redraws == 1);

    ReplayWin replay;
    Win *replay_view = &replay;
    replay_view->on_right_double_click(37, 41);
    assert(replay.right_double_clicks == 0);
}
"""


def compile_and_run(tmp_path, compiler, bodies, should_pass):
    source = tmp_path / "actual_win_callbacks.cpp"
    binary = tmp_path / "actual_win_callbacks"
    source.write_text(HOST_TYPES + "\n".join(bodies) + MAIN, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            "-Wno-unused-parameter",
                            str(source), "-o", str(binary)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    if should_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "behavioral mutant unexpectedly passed"


def test_actual_owner_method_names_keep_original_identity_spans():
    specs = {
        "src/popup.cpp": {
            0x00406B70: ("sub_406b70", ((0x00406B70, 0x00406B9C),)),
            0x00406BA0: ("sub_406ba0", ((0x00406BA0, 0x00406BCC),)),
        },
        "src/selectpartwin.cpp": {
            0x00433F30: ("?vslot_48@SelectPartWin@@UAEXHH@Z", ((0x00433F30, 0x00433F4F),)),
            0x00433F50: ("?on_scrolling@SelectPartWin@@QAEXHH@Z", ((0x00433F50, 0x00433F67),)),
        },
        "src/dialogs.cpp": {
            0x00612F80: ("?on_scrolled@Dialogs@@QAEXHH@Z", ((0x00612F80, 0x00612FA0),)),
            0x00612FA0: ("?on_scrolling@Dialogs@@QAEXHH@Z", ((0x00612FA0, 0x00612FC0),)),
        },
        "src/alphasave.cpp": {
            0x004080E0: ("?on_scrolling@AlphaSave@@QAEXHH@Z", ((0x004080E0, 0x0040810C),)),
            0x00408110: ("?on_scrolled@AlphaSave@@QAEXHH@Z", ((0x00408110, 0x0040813C),)),
        },
        "src/replaywin.cpp": {
            0x005AD3E0: ("?on_right_double_click@ReplayWin@@QAEXHH@Z", ((0x005AD3E0, 0x005AD3E3),)),
        },
    }
    for path, entries in specs.items():
        rows = {row.address: row for row in read_file(ROOT / path)}
        for address, (name, spans) in entries.items():
            row = rows[address]
            assert row.mode is Mode.BODY
            assert row.name == name
            assert row.image_spans == spans
    replay_header = (ROOT / "src/replaywin.h").read_text(encoding="utf-8")
    assert "class ReplayWin : public GraphicWin" in replay_header
    assert "void on_right_double_click(int, int);" in replay_header
    popup = (ROOT / "src/popup.cpp").read_text(encoding="utf-8")
    assert "void Popup::on_scrolled(int first, int second)" in popup
    assert "void Popup::on_scrolling(int first, int second)" in popup
    select = (ROOT / "src/selectpartwin.cpp").read_text(encoding="utf-8")
    assert "void SelectPartWin::on_scrolled(int, int row)" in select
    assert "void SelectPartWin::on_scrolling(int a1, int a2)" in select


def test_full_original_identity_claim_and_comment_history_continuity():
    for relative_path, addresses in BASELINE_SCOPE.items():
        original_text = subprocess.check_output(
            ["git", "show", f"{BASELINE_COMMIT}:{relative_path}"],
            cwd=ROOT, text=True)
        original_rows = {row.address: row
                         for row in read_text(original_text, relative_path)}
        current_rows = {row.address: row for row in read_file(ROOT / relative_path)}
        for address in addresses:
            assert address in original_rows, f"baseline identity missing: {relative_path}:{address:08X}"
            assert address in current_rows, f"current identity missing: {relative_path}:{address:08X}"
            original = original_rows[address]
            current = current_rows[address]
            assert preserved_record(current) == preserved_record(original), (
                f"historical identity/claim/lesson/comment changed: "
                f"{relative_path}:{address:08X}")


def test_full_tu_extractor_masks_comments_and_literals_and_rejects_hidden_body():
    signature = "void Probe::target(int value)"
    translation_unit = (
        "/* fake declaration/body: " + signature + " { return; } */\n"
        'const char *note = "fake signature ' + signature + ' { }";\n'
        + signature + " { if (value) { return; } }\n")
    extracted = extract_function(translation_unit, signature)
    assert extracted == signature + " { if (value) { return; } }"

    hidden_only = "/* " + signature + " { return; } */\\n"
    with pytest.raises(AssertionError, match="code-visible body"):
        extract_function(hidden_only, signature)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_callback_bodies_dispatch_through_correct_owner_slots(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    bodies = actual_bodies()
    assert "dialogs()).on_scrolled(first, second)" in bodies[0]
    assert "dialogs()).on_scrolling(first, second)" in bodies[1]
    assert "field_2B6C_ * row" in bodies[4] and "redraw();" in bodies[4]
    assert "on_scrolled(a1, a2);" in bodies[5]
    assert "file_win_.list_box_.on_scrolled(code, pos);" in bodies[6]
    assert "file_win_.list_box_.on_scrolling(code, pos);" in bodies[7]
    assert "on_scrolled(code, position);" in bodies[9]
    assert "ListBox::on_scrolling(code, pos);" in bodies[2]
    assert "ListBox::on_scrolling(a, b);" in bodies[3]
    compile_and_run(tmp_path, compiler, bodies, should_pass=True)

    mutants = list(bodies)
    mutants[0] = mutants[0].replace(
        "static_cast<GraphicWin &>(dialogs()).on_scrolled(first, second);",
        "static_cast<Win &>(dialogs()).Win::on_scrolled(first, second);")
    assert mutants[0] != bodies[0]
    compile_and_run(tmp_path, compiler, mutants, should_pass=False)

    mutants = list(bodies)
    mutants[5] = mutants[5].replace("on_scrolled(a1, a2);", "Win::on_scrolled(a1, a2);")
    assert mutants[5] != bodies[5]
    compile_and_run(tmp_path, compiler, mutants, should_pass=False)

    mutants = list(bodies)
    mutants[2] = mutants[2].replace("ListBox::on_scrolling", "Win::on_scrolling")
    assert mutants[2] != bodies[2]
    compile_and_run(tmp_path, compiler, mutants, should_pass=False)

    mutants = list(bodies)
    mutants[6] = mutants[6].replace(
        "file_win_.list_box_.on_scrolled(code, pos);",
        "file_win_.list_box_.on_scrolling(code, pos);")
    assert mutants[6] != bodies[6]
    compile_and_run(tmp_path, compiler, mutants, should_pass=False)


def test_win_contract_declarations_are_typed_and_source_slots_are_rewired():
    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert "virtual void on_right_up(int = 0, int = 0)" in header
    assert "virtual void on_right_double_click(int = 0, int = 0)" in header
    assert "virtual void on_scrolled(int = 0, int = 0)" in header
    assert "virtual void on_scrolling(int = 0, int = 0)" in header
    alpha = (ROOT / "src/alphasave.cpp").read_text(encoding="utf-8")
    assert "void AlphaSave::on_scrolled(int code, int pos)" in alpha
    assert "void AlphaSave::on_scrolling(int code, int pos)" in alpha
    replay = (ROOT / "src/replaywin.cpp").read_text(encoding="utf-8")
    assert "void ReplayWin::on_right_double_click(int, int)" in replay
    win_cpp = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    assert "this->on_right_up(x, y);" in win_cpp
    assert "this->on_right_double_click(x, y);" in win_cpp
    scroll = (ROOT / "src/scroll.cpp").read_text(encoding="utf-8")
    assert "parent->on_scrolled(" in scroll and "parent->on_scrolling(" in scroll
    # The native candidate must later refresh these current emitted symbols;
    # the original mapping names/spans checked above remain historical facts.
