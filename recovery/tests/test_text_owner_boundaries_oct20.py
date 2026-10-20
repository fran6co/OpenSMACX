"""Actual-source controls for the Text and TextIndex source ownership boundary."""

from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess
import textwrap

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _function(source: str, signature: str) -> str:
    clean = re.sub(r"/\*.*?\*/|//[^\n]*", "", source, flags=re.S)
    matches = list(re.finditer(re.escape(signature), clean))
    assert len(matches) == 1, f"expected one definition of {signature!r}"
    start = matches[0].start()
    opening = clean.index("{", start)
    depth = 0
    for at in range(opening, len(clean)):
        if clean[at] == "{":
            depth += 1
        elif clean[at] == "}":
            depth -= 1
            if depth == 0:
                return clean[start:at + 1]
    raise AssertionError(f"unterminated definition {signature!r}")


def _host_source(helper: str, constructor: str) -> str:
    return textwrap.dedent(r"""
        #include <cassert>
        #include <cstring>

        #define __cdecl
        typedef const char *LPCSTR;
        typedef char *LPSTR;
        static const int MaxTextIndexNum = 4;
        static int counts[4] = {0, 0, 0, 0};
        static int results[4] = {-1, -1, -1, -1};
        static int reads[4] = {0, 0, 0, 0};
        static int searches[4] = {-1, -1, -1, -1};
        static int search_calls = 0;

        struct TextIndex {
            int slot;
            unsigned get_count() {
                ++reads[slot];
                return (unsigned)counts[slot];
            }
            int search_index(LPCSTR, LPCSTR) {
                searches[search_calls++] = slot;
                return results[slot];
            }
        };
        static TextIndex TxtIndex[MaxTextIndexNum];

        HELPER_BODY

        static int requested[2] = {0, 0};
        static int allocation_calls = 0;
        static LPSTR allocation_results[2] = {0, 0};
        static char storage_a[8];
        static char storage_b[8];
        static LPSTR mem_get(int size) {
            requested[allocation_calls] = size;
            return allocation_results[allocation_calls++];
        }

        class Text {
        public:
            char file_name_[80];
            char file_path_[256];
            LPSTR current_pos_;
            void *text_file_;
            LPSTR buffer_get_;
            LPSTR buffer_item_;
            Text(int size);
        };

        CONSTRUCTOR_BODY

        static void reset_index() {
            search_calls = 0;
            for (int i = 0; i < 4; ++i) {
                TxtIndex[i].slot = i;
                counts[i] = 0;
                results[i] = -1;
                reads[i] = 0;
                searches[i] = -1;
            }
        }
        static void check_standalone_search_contract() {
            reset_index();
            assert(text_search_index("file", "section") == -1);
            assert(search_calls == 0);
            assert(reads[0] == 1 && reads[1] == 1 && reads[2] == 1 && reads[3] == 1);

            reset_index();
            counts[1] = 1;
            counts[3] = 1;
            results[1] = -7;
            results[3] = 23;
            assert(text_search_index("file", "section") == 23);
            assert(search_calls == 2 && searches[0] == 1 && searches[1] == 3);

            reset_index();
            counts[0] = counts[2] = 1;
            results[0] = -1;
            results[2] = -9;
            assert(text_search_index("file", "section") == -1);
            assert(search_calls == 2 && searches[0] == 0 && searches[1] == 2);
        }
        static void check_constructor_allocation_order() {
            allocation_calls = 0;
            allocation_results[0] = storage_a;
            allocation_results[1] = storage_b;
            Text both(512);
            assert(both.file_name_[0] == 0 && both.current_pos_ == 0 && both.text_file_ == 0);
            assert(both.buffer_get_ == storage_a && both.buffer_item_ == storage_b);
            assert(allocation_calls == 2 && requested[0] == 512 && requested[1] == 512);

            allocation_calls = 0;
            allocation_results[0] = 0;
            allocation_results[1] = storage_b;
            Text first_failed(17);
            assert(first_failed.buffer_get_ == 0 && first_failed.buffer_item_ == 0);
            assert(allocation_calls == 1 && requested[0] == 17);

            allocation_calls = 0;
            allocation_results[0] = storage_a;
            allocation_results[1] = 0;
            Text second_failed(29);
            assert(second_failed.buffer_get_ == storage_a && second_failed.buffer_item_ == 0);
            assert(allocation_calls == 2 && requested[0] == 29 && requested[1] == 29);
        }
        int main() {
            check_standalone_search_contract();
            check_constructor_allocation_order();
            return 0;
        }
    """).replace("HELPER_BODY", helper).replace("CONSTRUCTOR_BODY", constructor)


def test_text_and_textindex_have_the_pinned_owner_boundaries() -> None:
    text_header = (ROOT / "src/text.h").read_text()
    text_source = (ROOT / "src/text.cpp").read_text()
    index_header = (ROOT / "src/textindex.h").read_text()
    index_source = (ROOT / "src/textindex.cpp").read_text()

    assert "MEASURED Text(int size);" in text_header
    assert "MEASURED inline Text(int size)" not in text_header
    ctor_at = text_source.index("Text::Text(int size)")
    global_at = text_source.index("Text Txt(512);")
    assert ctor_at < global_at
    ctor = _function(text_source, "Text::Text(int size)")
    assert "file_name_[0] = 0;" in ctor
    assert ctor.index("buffer_get_ = (LPSTR)mem_get(size);") < ctor.index("if (buffer_get_)")
    assert "buffer_item_ = (LPSTR)mem_get(size);" in ctor

    assert "MEASURED inline int __cdecl text_search_index" in index_header
    assert "int __cdecl text_search_index(" not in index_source
    helper = _function(index_header, "inline int __cdecl text_search_index(")
    assert "return -1;" in helper
    open_body = _function(text_source, "BOOL Text::open(LPCSTR src_file_id, LPCSTR section_id)")
    assert "int seek_addr = text_search_index(file_name_, section_id);" in open_body
    assert "TxtIndex[i].search_index(file_name_, section_id)" not in open_body


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_search_and_constructor_bodies_with_damage_controls(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    helper = _function((ROOT / "src/textindex.h").read_text(), "inline int __cdecl text_search_index(")
    constructor = _function((ROOT / "src/text.cpp").read_text(), "Text::Text(int size)")
    variants = [("actual", helper, constructor, True)]
    miss_mutant = helper.replace("return -1;", "return 0;", 1)
    assert miss_mutant != helper
    variants.append(("wrong-miss", miss_mutant, constructor, False))
    allocation_mutant = constructor.replace("if (buffer_get_) {", "if (1) {", 1)
    assert allocation_mutant != constructor
    variants.append(("unconditional-second-allocation", helper, allocation_mutant, False))

    for label, helper_body, constructor_body, expected_success in variants:
        cpp = tmp_path / f"text_owner_{label}.cpp"
        executable = tmp_path / f"text_owner_{label}"
        cpp.write_text(_host_source(helper_body, constructor_body))
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
            capture_output=True, text=True,
        )
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(executable)], capture_output=True, text=True)
        if expected_success:
            assert ran.returncode == 0, ran.stderr
        else:
            assert ran.returncode != 0, f"{label} damage control unexpectedly passed"


def test_pinned_text_open_miss_branch_bypasses_the_seek_block() -> None:
    pytest.importorskip("capstone")
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    image = PeImage(image_path.read_bytes(), PIN)
    instructions = Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x005FDA00, 0x005FDC04)), 0x005FDA00
    )
    by_address = {instruction.address: instruction for instruction in instructions}
    assert (by_address[0x005FDB0A].mnemonic, by_address[0x005FDB0A].op_str) == ("call", "0x5fe120")
    assert (by_address[0x005FDB13].mnemonic, by_address[0x005FDB13].op_str) == ("jge", "0x5fdb27")
    assert (by_address[0x005FDB15].mnemonic, by_address[0x005FDB15].op_str) == ("xor", "edi, edi")
    assert (by_address[0x005FDB25].mnemonic, by_address[0x005FDB25].op_str) == ("jmp", "0x5fdb53")
    assert by_address[0x005FDB25].address < 0x005FDB53

    string_list = Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x00608980, 0x00608B9B)), 0x00608980
    )
    direct_constructor_call = next(
        instruction for instruction in string_list if instruction.address == 0x006089BB
    )
    assert (direct_constructor_call.mnemonic, direct_constructor_call.op_str) == ("call", "0x5fd880")
