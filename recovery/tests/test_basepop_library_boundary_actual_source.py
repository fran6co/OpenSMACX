"""Host controls for BasePop-owned generic allocator and caption storage."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError(f"unterminated production definition: {signature}")


def _actual_fixture() -> str:
    basepop_h = (ROOT / "src/basepop.h").read_text()
    main_h = (ROOT / "src/main.h").read_text()
    popup_h = (ROOT / "src/popup.h").read_text()
    basepop_cpp = (ROOT / "src/basepop.cpp").read_text()
    main_cpp = (ROOT / "src/main.cpp").read_text()
    popup_cpp = (ROOT / "src/popup.cpp").read_text()

    typedef = "typedef BasePop *(__cdecl func_popup_alloc)();"
    hook_declaration = "extern func_popup_alloc *PopupAllocHook;"
    caption_declaration = "extern char PopupStartCaption[256];"
    assert basepop_h.count(typedef) == 1
    assert basepop_h.count(hook_declaration) == 1
    assert basepop_h.count(caption_declaration) == 1
    assert typedef not in main_h and hook_declaration not in main_h
    assert caption_declaration not in popup_h

    hook_definition = "func_popup_alloc *PopupAllocHook = &BasePop::basepop_alloc;"
    caption_definition = "char PopupStartCaption[256];  // 0x009B8AA8"
    assert basepop_cpp.count(hook_definition) == 1
    assert basepop_cpp.count(caption_definition) == 1
    assert hook_definition not in main_cpp
    assert caption_definition not in popup_cpp

    registration = (
        "    PopupAllocHook = &BasePop::basepop_alloc;\n"
        "    if (&Popup::alloc) {\n"
        "        PopupAllocHook = &Popup::alloc;\n"
        "    }"
    )
    assert main_cpp.count(registration) == 1

    # The five broad subsystem headers no longer supply incidental globals to
    # BasePop. `general.h` and `text.h` remain real function dependencies.
    for header in ("main.h", "popup.h", "worldwin.h", "dipedit.h", "datalink.h"):
        assert f'#include "{header}"' not in basepop_cpp

    basepop_alloc = _definition(basepop_cpp, "BasePop *BasePop::basepop_alloc() {")
    set_caption = _definition(basepop_cpp, "int BasePop::set_def_file(char *text) {")
    popup_alloc = _definition(popup_cpp, "BasePop *Popup::alloc() {")
    popup_start = _definition(popup_cpp, "void Popup::start(const char *label) {")

    return r'''#include <cassert>
#include <cstring>
#define __cdecl

class BasePop {
public:
    int owner_kind;
    BasePop() : owner_kind(1) {}
    virtual ~BasePop() {}
    static BasePop *basepop_alloc();
    static int set_def_file(char *text);
};

''' + typedef + "\n" + hook_declaration + "\n" + caption_declaration + r'''

class Popup : public BasePop {
public:
    Popup() { owner_kind = 2; }
    static BasePop *alloc();
    void start(char *file, const char *label, int value, char *override_text,
               int flags, void *owner) {
        started_file = file;
        started_label = label;
        started_value = value;
        started_override = override_text;
        started_flags = flags;
        started_owner = owner;
    }
    void start(const char *label);
    char *started_file;
    const char *started_label;
    int started_value;
    char *started_override;
    int started_flags;
    void *started_owner;
};

''' + hook_definition + "\n" + caption_definition + "\n" + basepop_alloc + "\n" + \
        set_caption + "\n" + popup_alloc + "\n" + popup_start + "\n" + \
        "void register_application_popup() {\n" + registration + "\n}\n" + r'''

int main() {
    assert(PopupAllocHook == &BasePop::basepop_alloc);
    BasePop *generic = PopupAllocHook();
    assert(generic != 0 && generic->owner_kind == 1);
    delete generic;

    assert(BasePop::set_def_file(const_cast<char *>("SHARED")) == 0);
    assert(std::strcmp(PopupStartCaption, "SHARED") == 0);
    Popup popup;
    popup.start("QUESTION");
    assert(popup.started_file == PopupStartCaption);
    assert(std::strcmp(popup.started_file, "SHARED") == 0);
    assert(std::strcmp(popup.started_label, "QUESTION") == 0);

    register_application_popup();
    assert(PopupAllocHook == &Popup::alloc);
    BasePop *application = PopupAllocHook();
    assert(application != 0 && application->owner_kind == 2);
    delete application;
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_basepop_library_owns_default_hook_and_shared_caption(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    translation_unit = tmp_path / "actual_basepop_library_boundary.cpp"
    executable = tmp_path / "actual-basepop-library-boundary"
    translation_unit.write_text(_actual_fixture())
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-Wno-address",
         str(translation_unit), "-o", str(executable)],
        check=True, capture_output=True, text=True, timeout=30,
    )
    result = subprocess.run([str(executable)], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


def test_startup_order_and_original_mappings_remain_attached() -> None:
    from match_recovery.source import read_file

    basepop_records = {record.address: record for record in read_file(ROOT / "src/basepop.cpp")}
    main_records = {record.address: record for record in read_file(ROOT / "src/main.cpp")}
    for address in (0x00604E40, 0x006043D0):
        assert address in basepop_records
        assert basepop_records[address].image_spans
    assert 0x0045F950 in main_records
    startup = main_records[0x0045F950].region
    assert startup.index("PopupAllocHook = &BasePop::basepop_alloc;") < startup.index(
        "PopupAllocHook = &Popup::alloc;"
    )

    basepop = (ROOT / "src/basepop.cpp").read_text()
    popup = (ROOT / "src/popup.cpp").read_text()
    assert "text_open(text_file != 0 ? text_file : (char *)PopupStartCaption" in basepop
    assert "start(PopupStartCaption, label, -1, nullptr, 0, nullptr);" in popup

    # Vehicle-specific presentation is application code, not a BasePop method.
    popup_records = {record.address: record for record in read_file(ROOT / "src/popup.cpp")}
    assert 0x0048CF00 not in basepop_records
    helper = popup_records[0x0048CF00]
    assert helper.image_spans == ((0x0048CF00, 0x0048CF4A),)
    assert helper.byte_exact  # Retained historical obligation, not fresh matching.
    assert "owner->set_redraw_callback(veh_sprite_draw);" in helper.region
