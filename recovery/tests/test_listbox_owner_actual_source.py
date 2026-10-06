"""Actual-installed-source controls for the three ListBox recovered methods.

This is ready to copy into recovery/tests after coordinator integration. It
never imports the proposal emitter and has no synthesized-source fallback:
missing product definitions, mappings, history, or a compiler are test errors.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import tempfile

import pytest

from match_recovery.source import read_text

ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/listbox.cpp').is_file())
HISTORY_REVISION = "5fe79d193062cacf6b9eb9c5b901f9a0501cd2d8"
SOURCE = "src/listbox.cpp"
IDENTITIES = {
    0x00609FA0: "void ListBox::set_selected_id(int id) {",
    0x0060CB70: "void ListBox::on_mousewheel(int delta) {",
    0x0060C7D0: "int ListBox::UNK2(char *text) {",
}
PRESERVED_FIELDS = (
    "address", "mode", "state", "name", "image_spans", "symbol", "kind",
    "exclusion", "byte_exact", "semantic", "recipe", "levers", "ruled_out",
    "unrecoverable", "deferred",
)


def _mask_noncode(text: str) -> str:
    """Replace comments/strings/chars with spaces while retaining offsets."""
    chars = list(text)
    state = "code"
    i = 0
    while i < len(text):
        c = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if c == "/" and nxt == "/":
                chars[i] = chars[i + 1] = " "
                state = "line_comment"
                i += 2
                continue
            if c == "/" and nxt == "*":
                chars[i] = chars[i + 1] = " "
                state = "block_comment"
                i += 2
                continue
            if c == '"':
                chars[i] = " "
                state = "string"
            elif c == "'":
                chars[i] = " "
                state = "char"
        elif state == "line_comment":
            if c == "\n":
                state = "code"
            else:
                chars[i] = " "
        elif state == "block_comment":
            if c == "*" and nxt == "/":
                chars[i] = chars[i + 1] = " "
                state = "code"
                i += 2
                continue
            if c != "\n":
                chars[i] = " "
        else:
            if c == "\\":
                if c != "\n":
                    chars[i] = " "
                if i + 1 < len(text) and text[i + 1] != "\n":
                    chars[i + 1] = " "
                i += 2
                continue
            if ((state == "string" and c == '"')
                    or (state == "char" and c == "'")):
                chars[i] = " "
                state = "code"
            elif c != "\n":
                chars[i] = " "
        i += 1
    return "".join(chars)


def _balanced_end(masked: str, opening: int, signature: str) -> int:
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return index + 1
    raise AssertionError(f"unterminated active definition: {signature}")


def _extract_active(source: str, signature: str) -> str:
    masked = _mask_noncode(source)
    offsets = []
    cursor = 0
    while True:
        found = masked.find(signature, cursor)
        if found < 0:
            break
        offsets.append(found)
        cursor = found + len(signature)
    assert len(offsets) == 1, (
        f"expected one active definition of {signature!r}, found {len(offsets)}"
    )
    start = offsets[0]
    opening = masked.find("{", start + len(signature) - 1)
    assert opening >= 0, f"definition has no body: {signature}"
    return source[start:_balanced_end(masked, opening, signature)]


def _current_source_and_records():
    path = ROOT / SOURCE
    assert path.is_file(), f"missing installed owner source: {path}"
    source = path.read_text(encoding="utf-8")
    records = {record.address: record for record in read_text(source, SOURCE)
               if record.address in IDENTITIES}
    assert set(records) == set(IDENTITIES), (
        "installed listbox.cpp is missing one or more original mappings"
    )
    for address, record in records.items():
        assert record.region and not record.extract_error, (
            f"0x{address:08X}: source reader could not extract a complete region"
        )
        assert record.source_role.value == "unclassified", (
            f"0x{address:08X}: definition remains classified as scaffolding"
        )
    listbox_h = (ROOT / "src/listbox.h").read_text(encoding="utf-8")
    dialog_h = (ROOT / "src/dialog.h").read_text(encoding="utf-8")
    strings_h = (ROOT / "src/stringstruct.h").read_text(encoding="utf-8")
    assert re.search(
        r"class\s+ListBox\s*:\s*public\s+virtual\s+GraphicWin\s*,\s*"
        r"public\s+virtual\s+Dialog\s*\{", listbox_h
    ), "ListBox's actual GraphicWin/Dialog hierarchy changed or is missing"
    for declaration in (
        "void on_mousewheel(int a) override;",
        "int UNK2(char *text);",
        "void set_selected_id(int id);",
    ):
        assert declaration in listbox_h, f"missing ListBox declaration: {declaration}"
    assert "StringList item_list_;" in dialog_h, "Dialog no longer owns item_list_"
    for declaration in ("StringStructEntry *next;", "StringStructEntry *previous;",
                        "StringStructPayload *string_payload();"):
        assert declaration in strings_h, f"missing typed list API: {declaration}"
    return source, records


def _baseline_records():
    completed = subprocess.run(
        ["git", "show", f"{HISTORY_REVISION}:src/leaf_recoveries.cpp"],
        cwd=ROOT, text=True, capture_output=True, timeout=15,
    )
    assert completed.returncode == 0, (
        f"cannot read pinned mapping-history baseline {HISTORY_REVISION}: "
        + completed.stderr
    )
    records = {record.address: record for record in read_text(
        completed.stdout, f"{HISTORY_REVISION}:src/leaf_recoveries.cpp")
        if record.address in IDENTITIES}
    assert set(records) == set(IDENTITIES), "history baseline lacks required ListBox rows"
    return records


def _validate_history(current, baseline):
    for address in IDENTITIES:
        actual = current[address]
        historical = baseline[address]
        for field in PRESERVED_FIELDS:
            if field == "symbol" and address == 0x0060CB70:
                # The historical leaf spelling was nonvirtual. The real
                # override's symbol was captured from fresh native COFF.
                assert historical.symbol == ""
                assert historical.name == "?on_mousewheel@ListBox@@QAEXH@Z"
                assert actual.symbol == "?on_mousewheel@ListBox@@UAEXH@Z"
                continue
            assert getattr(actual, field) == getattr(historical, field), (
                f"0x{address:08X}: original {field} changed during owner transfer"
            )
        assert actual.path.resolve() == (ROOT / SOURCE).resolve()


def _method_sources(mutant: str | None = None) -> dict[int, str]:
    source, current = _current_source_and_records()
    _validate_history(current, _baseline_records())
    methods = {address: _extract_active(source, signature)
               for address, signature in IDENTITIES.items()}
    if mutant == "case-sensitive":
        text = methods[0x0060C7D0]
        old = "_strcmpi(name, text) == 0"
        assert text.count(old) == 1
        methods[0x0060C7D0] = text.replace(old, "strcmp(name, text) == 0", 1)
    elif mutant == "wheel-direction":
        text = methods[0x0060CB70]
        old = "if (delta < 0)"
        assert text.count(old) == 1
        methods[0x0060CB70] = text.replace(old, "if (delta >= 0)", 1)
    elif mutant == "miss-selection":
        text = methods[0x00609FA0]
        old = "set_selected_pos(-1, 1);"
        assert text.count(old) == 1
        methods[0x00609FA0] = text.replace(old, "set_selected_pos(0, 1);", 1)
    elif mutant == "safe-address4":
        text = methods[0x0060C7D0]
        old = "name = *reinterpret_cast<char **>(4);"
        assert text.count(old) == 1
        methods[0x0060C7D0] = text.replace(old, "name = 0;", 1)
    return methods


HOST = r'''#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <vector>

static int _strcmpi(const char *left, const char *right) {
    if (!left || !right) return left == right ? 0 : (left ? 1 : -1);
    for (;;) {
        const unsigned char a = static_cast<unsigned char>(*left++);
        const unsigned char b = static_cast<unsigned char>(*right++);
        const int la = std::tolower(a), lb = std::tolower(b);
        if (la != lb) return la - lb;
        if (!a || !b) return 0;
    }
}

struct StringStructPayload { char *text; };
struct StringStructEntry {
    int id;
    StringStructPayload *payload;
    StringStructEntry *next;
    StringStructEntry *previous;
    StringStructPayload *string_payload() { return payload; }
};
struct StringList {
    StringStructEntry *head_;
    StringStructEntry *current_;
    int entry_count_;
    int current_position_;
};
struct Dialog { StringList item_list_; };
struct Win {
    static std::vector<int> down;
    static std::vector<int> up;
    void on_mousewheel_down(int value) { down.push_back(value); }
    void on_mousewheel_up(int value) { up.push_back(value); }
};
std::vector<int> Win::down;
std::vector<int> Win::up;
struct GraphicWin : Win {};
struct ListBox : virtual GraphicWin, virtual Dialog {
    int selected_position = -99;
    int selected_notify = -99;
    void set_selected_pos(int position, int notify) {
        selected_position = position;
        selected_notify = notify;
    }
    void set_selected_id(int id);
    void on_mousewheel(int delta);
    int UNK2(char *text);
};

__METHODS__

static void wire(StringStructEntry &a, StringStructEntry &b,
                 StringStructEntry &c) {
    a.next = &b; b.next = &c; c.next = &a;
    a.previous = &c; b.previous = &a; c.previous = &b;
}

int main(int argc, char **) {
    if (argc > 1) {
        ListBox box{};
        char entry_text[] = "entry";
        StringStructPayload payload{entry_text};
        StringStructEntry entry{1, &payload, &entry, &entry};
        box.item_list_ = {0, &entry, 1, 0};
        char query[] = "entry";
        box.UNK2(query);
        return 0; // reaching here means the pinned address-4 fault was lost
    }
    {
        ListBox box{};
        StringStructEntry a{10, 0, 0, 0}, b{20, 0, 0, 0}, c{30, 0, 0, 0};
        a.next = &b; b.next = &c; c.next = 0;
        box.item_list_ = {&a, &a, 3, 0};
        box.set_selected_id(20);
        assert(box.selected_position == 1 && box.selected_notify == 1);
        assert(box.item_list_.current_ == &b && box.item_list_.current_position_ == 1);
        box.selected_position = -99;
        box.set_selected_id(99);
        assert(box.selected_position == -1 && box.selected_notify == 1);
        assert(box.item_list_.current_ == 0 && box.item_list_.current_position_ == 3);
        box.item_list_ = {0, 0, 0, 12};
        box.set_selected_id(1);
        assert(box.selected_position == -1 && box.selected_notify == 1);
    }
    {
        ListBox box{};
        Win::down.clear(); Win::up.clear();
        box.on_mousewheel(-3);
        box.on_mousewheel(2);
        box.on_mousewheel(0);
        assert((Win::down == std::vector<int>{1, 1, 1}));
        assert((Win::up == std::vector<int>{1, 1}));
    }
    {
        ListBox box{};
        char alpha[] = "Alpha", beta[] = "Beta", gamma[] = "Gamma";
        StringStructPayload pa{alpha}, pb{beta}, pg{gamma};
        StringStructEntry a{1, &pa, 0, 0}, b{2, &pb, 0, 0}, c{3, &pg, 0, 0};
        wire(a, b, c);
        box.item_list_ = {&a, &a, 3, 0};
        char query[] = "bEtA";
        assert(box.UNK2(query) == 1);
        assert(box.item_list_.current_ == &a && box.item_list_.current_position_ == 0);

        box.item_list_.current_ = &b;
        box.item_list_.current_position_ = -1;
        char absent[] = "absent";
        assert(box.UNK2(absent) == -1);
        assert(box.item_list_.current_ == &c && box.item_list_.current_position_ == 2);

        box.item_list_.current_ = &a;
        box.item_list_.current_position_ = 7;
        assert(box.UNK2(absent) == -1);
        assert(box.item_list_.current_position_ == 10);
        assert(box.UNK2(0) == -1);
        assert(box.item_list_.current_position_ == 10);
    }
}
'''


def _compile(tmp: Path, compiler: str, methods: dict[int, str]) -> Path:
    source = HOST.replace("__METHODS__", "\n\n".join(methods[a] for a in IDENTITIES))
    cpp = tmp / "listbox_owner_control.cpp"
    exe = tmp / "listbox_owner_control"
    cpp.write_text(source, encoding="utf-8")
    result = subprocess.run(
        [compiler, "-std=c++17", "-O0", str(cpp), "-o", str(exe)],
        cwd=tmp, text=True, capture_output=True, timeout=45,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    return exe


def test_installed_listbox_methods_preserve_history_and_behavior():
    methods = _method_sources()
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for actual-source controls"
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="listbox-actual-source-") as tmp_name:
            tmp = Path(tmp_name)
            exe = _compile(tmp, compiler, methods)
            normal = subprocess.run([str(exe)], cwd=tmp, text=True,
                                    capture_output=True, timeout=15)
            assert normal.returncode == 0, normal.stdout + normal.stderr
            fault = subprocess.run([str(exe), "address4"], cwd=tmp, text=True,
                                   capture_output=True, timeout=15)
            if os.name == "posix":
                assert fault.returncode == -signal.SIGSEGV, (
                    "the malformed head-null/current-nonnull [4] read did not "
                    f"fault as pinned; return={fault.returncode}, stderr={fault.stderr}"
                )
            else:
                assert fault.returncode != 0, "address-4 malformed state unexpectedly returned"


@pytest.mark.parametrize("mutant", ("case-sensitive", "wheel-direction", "miss-selection"))
def test_installed_method_mutations_fail_behavior(mutant: str):
    methods = _method_sources(mutant)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="listbox-mutant-") as tmp_name:
            tmp = Path(tmp_name)
            exe = _compile(tmp, compiler, methods)
            result = subprocess.run([str(exe)], cwd=tmp, text=True,
                                    capture_output=True, timeout=15)
            assert result.returncode != 0, (
                f"behavioral mutant {mutant} unexpectedly passed with {compiler}"
            )


def test_address4_fault_control_detects_a_safety_normalization():
    methods = _method_sources("safe-address4")
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler
    with tempfile.TemporaryDirectory(prefix="listbox-address4-mutant-") as tmp_name:
        tmp = Path(tmp_name)
        exe = _compile(tmp, compiler, methods)
        result = subprocess.run([str(exe), "address4"], cwd=tmp, text=True,
                                capture_output=True, timeout=15)
        assert result.returncode == 0, (
            "safe-fallback mutation should return normally and thereby expose "
            "loss of the pinned address-4 fault"
        )


def test_comments_and_literals_cannot_stand_in_for_active_methods():
    source_path = ROOT / SOURCE
    source = source_path.read_text(encoding="utf-8")
    decoys = "\n/* void ListBox::UNK2(char *text) { return 0; } */\n"
    decoys += 'const char *decoy = "void ListBox::UNK2(char *text) { return 0; }";\n'
    with_decoys = source + decoys
    actual = _extract_active(with_decoys, IDENTITIES[0x0060C7D0])
    assert "items.current_->string_payload()->text" in actual
    hidden = source.replace(
        IDENTITIES[0x0060C7D0],
        "/* " + IDENTITIES[0x0060C7D0] + " */",
        1,
    ) + decoys
    with pytest.raises(AssertionError, match="expected one active definition"):
        _extract_active(hidden, IDENTITIES[0x0060C7D0])


def test_current_source_contains_typed_owner_paths_and_preserves_fault_branch():
    source, _ = _current_source_and_records()
    assert "StringList &items = Dialog::item_list_;" in source
    assert "items.current_->string_payload()->text" in source
    assert "Win::on_mousewheel_down(1);" in source
    assert "Win::on_mousewheel_up(1);" in source
    assert "name = *reinterpret_cast<char **>(4);" in source
    assert "vbt[2]" not in "\n".join(
        _extract_active(source, IDENTITIES[address]) for address in IDENTITIES
    )
