"""Actual-source controls for Text::open's original ordinary copy calls."""

from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess
import textwrap

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/text.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def text_open_body(source: str) -> str:
    signature = "BOOL Text::open(LPCSTR src_file_id, LPCSTR section_id)"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for at in range(opening, len(source)):
        if source[at] == "{":
            depth += 1
        elif source[at] == "}":
            depth -= 1
            if depth == 0:
                return source[start:at + 1]
    raise AssertionError("Text::open body is incomplete")


def text_search_body(source: str) -> str:
    signature = "inline int __cdecl text_search_index("
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for at in range(opening, len(source)):
        if source[at] == "{":
            depth += 1
        elif source[at] == "}":
            depth -= 1
            if depth == 0:
                return source[start:at + 1]
    raise AssertionError("text_search_index body is incomplete")


def fixture(body: str, helper: str) -> str:
    return textwrap.dedent(r"""
        #include <cassert>
        #include <cctype>
        #include <cstdio>
        #include <cstring>

        #define __cdecl
        typedef int BOOL;
        typedef const char *LPCSTR;
        typedef char *LPSTR;
        static const BOOL TRUE_VALUE = 1;
        struct Filefind { char last_path[256]; };
        static Filefind FilefindPath = {0};
        static const char *file_contents = "#OPTIONS\nbody\n";
        static const char *last_opened_name = 0;
        static const char *last_opened_mode = 0;
        static int open_fails = 0;
        static int search_calls = 0;
        static int count_reads[4] = {0, 0, 0, 0};
        static int search_slots[4] = {-1, -1, -1, -1};
        static int index_counts[4] = {0, 0, 0, 0};
        static int index_results[4] = {-1, -1, -1, -1};
        static int log_calls = 0;
        static int seek_calls = 0;
        static long seek_offset = -1;
        static int rewind_calls = 0;
        static int read_calls = 0;
        static const char *searched_file = 0;
        static const char *searched_section = 0;
        static char *section_to_mutate = 0;

        static FILE *env_open(LPCSTR name, LPCSTR mode) {
            last_opened_name = name;
            last_opened_mode = mode;
            if (open_fails) return 0;
            FILE *stream = tmpfile();
            assert(stream);
            fwrite(file_contents, 1, strlen(file_contents), stream);
            rewind(stream);
            return stream;
        }
        static const int MaxTextIndexNum = 4;
        struct TextIndex {
            int slot;
            unsigned get_count() {
                ++count_reads[slot];
                return (unsigned)index_counts[slot];
            }
            int search_index(LPCSTR file, LPCSTR section) {
                search_slots[search_calls++] = slot;
                searched_file = file;
                searched_section = section;
                if (section_to_mutate == section) {
                    strcpy(section_to_mutate, "MUTATED");
                    section_to_mutate = 0;
                }
                return index_results[slot];
            }
        };
        static TextIndex TxtIndex[MaxTextIndexNum];
        HELPER_BODY
        static void log_say(LPCSTR, LPCSTR, int, int, int) { ++log_calls; }
        static int host_feof(FILE *stream) { return feof(stream); }
        static char *host_fgets(char *buffer, int size, FILE *stream) {
            ++read_calls;
            return fgets(buffer, size, stream);
        }
        static int host_fseek(FILE *stream, long offset, int origin) {
            ++seek_calls;
            seek_offset = offset;
            return fseek(stream, offset, origin);
        }
        static void host_rewind(FILE *stream) { ++rewind_calls; rewind(stream); }
        static void kill_lf(char *text) {
            size_t size = strlen(text);
            if (size && text[size - 1] == '\n') text[size - 1] = 0;
        }
        static void purge_spaces(char *) {}
        static int _stricmp(const char *left, const char *right) {
            while (*left && *right) {
                int a = tolower((unsigned char)*left++);
                int b = tolower((unsigned char)*right++);
                if (a != b) return a - b;
            }
            return (unsigned char)*left - (unsigned char)*right;
        }

        class Text {
        public:
            char file_name_[80];
            char file_path_[256];
            char *current_pos_;
            FILE *text_file_;
            char *buffer_get_;
            char buffer_[512];
            int close_calls;
            Text() : current_pos_(0), text_file_(0), buffer_get_(buffer_), close_calls(0) {
                file_name_[0] = file_path_[0] = buffer_[0] = 0;
            }
            void close() {
                ++close_calls;
                if (text_file_) { fclose(text_file_); text_file_ = 0; }
            }
            BOOL open(LPCSTR src_file_id, LPCSTR section_id);
        };

        #define feof host_feof
        #define fgets host_fgets
        #define fseek host_fseek
        #define rewind host_rewind
        """).replace("HELPER_BODY", helper) + body + textwrap.dedent(r"""
        #undef feof
        #undef fgets
        #undef fseek
        #undef rewind

        static void reset_controls() {
            last_opened_name = last_opened_mode = searched_file = searched_section = 0;
            section_to_mutate = 0;
            open_fails = search_calls = log_calls = seek_calls = rewind_calls = read_calls = 0;
            seek_offset = -1;
            for (int i = 0; i < MaxTextIndexNum; ++i) {
                TxtIndex[i].slot = i;
                count_reads[i] = 0;
                index_counts[i] = 0;
                index_results[i] = -1;
                search_slots[i] = -1;
            }
            file_contents = "#OPTIONS\nbody\n";
            strcpy(FilefindPath.last_path, "C:\\Data\\rules.txt");
        }
        static void check_filename_and_section() {
            reset_controls();
            index_counts[0] = 1;
            Text text;
            assert(text.open("dir.withdot/rules", "OPTIONS") == 0);
            assert(strcmp(text.file_name_, "dir.withdot/rules") == 0);
            assert(strcmp(last_opened_name, "dir.withdot/rules") == 0);
            assert(strcmp(last_opened_mode, "rt") == 0);
            assert(strcmp(text.file_path_, "C:\\Data\\rules.txt") == 0);
            assert(search_calls == 1 && search_slots[0] == 0);
            assert(strcmp(searched_file, text.file_name_) == 0);
            assert(strcmp(searched_section, "OPTIONS") == 0);
            assert(strcmp(text.buffer_get_, "#OPTIONS") == 0);
            assert(text.current_pos_ == text.buffer_get_);
            assert(log_calls == 0 && text.close_calls == 1);
            text.close();

            reset_controls();
            index_counts[0] = 1;
            Text dotted;
            assert(dotted.open("rules", "OPTIONS") == 0);
            assert(strcmp(dotted.file_name_, "rules.txt") == 0);
            dotted.close();
        }
        static void check_null_section_and_open_failure() {
            reset_controls();
            Text no_section;
            assert(no_section.open("rules", 0) == 0);
            assert(strcmp(no_section.file_name_, "rules.txt") == 0);
            assert(no_section.text_file_ && search_calls == 0 && read_calls == 0);
            no_section.close();

            reset_controls();
            open_fails = 1;
            Text failed;
            strcpy(failed.file_path_, "unchanged");
            assert(failed.open("rules", "OPTIONS") == TRUE_VALUE);
            assert(failed.close_calls == 1 && !failed.text_file_);
            assert(strcmp(failed.file_path_, "unchanged") == 0);
            assert(search_calls == 0);
        }
        static void check_fallback_reopen_success_and_failure() {
            reset_controls();
            index_counts[0] = 1;
            index_results[0] = 0;
            Text reopened;
            strcpy(reopened.file_name_, "fallback.txt");
            assert(reopened.open(0, "OPTIONS") == 0);
            assert(strcmp(last_opened_name, "fallback.txt") == 0);
            assert(strcmp(last_opened_mode, "rt") == 0);
            assert(strcmp(reopened.file_path_, "C:\\Data\\rules.txt") == 0);
            assert(reopened.text_file_ && reopened.close_calls == 0);
            assert(search_calls == 1 && strcmp(searched_file, "fallback.txt") == 0);
            assert(strcmp(searched_section, "OPTIONS") == 0);
            assert(strcmp(reopened.buffer_get_, "#OPTIONS") == 0);
            assert(reopened.current_pos_ == reopened.buffer_get_);
            reopened.close();

            reset_controls();
            open_fails = 1;
            Text failed_reopen;
            strcpy(failed_reopen.file_name_, "fallback.txt");
            strcpy(failed_reopen.file_path_, "unchanged");
            assert(failed_reopen.open(0, "OPTIONS") == TRUE_VALUE);
            assert(strcmp(last_opened_name, "fallback.txt") == 0);
            assert(strcmp(last_opened_mode, "rt") == 0);
            assert(failed_reopen.close_calls == 0 && !failed_reopen.text_file_);
            assert(strcmp(failed_reopen.file_path_, "unchanged") == 0);
            assert(search_calls == 0 && read_calls == 0);
        }
        static void check_header_precedes_mutating_index_search() {
            reset_controls();
            index_counts[0] = 1;
            index_results[0] = 0;
            Text aliased;
            char section[] = "OPTIONS";
            section_to_mutate = section;
            assert(aliased.open("rules", section) == 0);
            assert(strcmp(section, "MUTATED") == 0);
            assert(search_calls == 1 && searched_section == section);
            assert(strcmp(aliased.buffer_get_, "#OPTIONS") == 0);
            assert(aliased.current_pos_ == aliased.buffer_get_);
            aliased.close();
        }
        static void check_existing_stream_and_seek() {
            reset_controls();
            index_counts[0] = 1;
            index_results[0] = 0;
            Text existing;
            existing.text_file_ = tmpfile();
            assert(existing.text_file_);
            fwrite(file_contents, 1, strlen(file_contents), existing.text_file_);
            rewind(existing.text_file_);
            strcpy(existing.file_name_, "already-open.txt");
            strcpy(existing.file_path_, "keep existing path");
            assert(existing.open(0, "OPTIONS") == 0);
            assert(last_opened_name == 0 && existing.close_calls == 0);
            assert(strcmp(existing.file_path_, "keep existing path") == 0);
            assert(seek_calls == 1 && log_calls == 1 && rewind_calls == 0);
            assert(seek_offset == 0);
            assert(strcmp(searched_file, "already-open.txt") == 0);
            existing.close();
        }
        static void check_eof_rewind_then_close() {
            reset_controls();
            file_contents = "no section here\n";
            index_counts[0] = 1;
            Text missing;
        missing.text_file_ = tmpfile();
        assert(missing.text_file_);
        fwrite(file_contents, 1, strlen(file_contents), missing.text_file_);
        rewind(missing.text_file_);
        while (fgetc(missing.text_file_) != EOF) {}
        assert(feof(missing.text_file_));
        strcpy(missing.file_name_, "existing.txt");
        assert(missing.open(0, "MISSING") == TRUE_VALUE);
        assert(seek_calls == 0 && log_calls == 0);
            assert(rewind_calls == 1);
            assert(missing.close_calls == 1 && !missing.text_file_);
        }
        static void check_zero_counts_and_all_misses_do_not_seek() {
            reset_controls();
            file_contents = "no section here\n";
            Text no_entries;
            assert(no_entries.open("rules", "MISSING") == TRUE_VALUE);
            assert(search_calls == 0 && count_reads[0] == 1 && count_reads[1] == 1);
            assert(count_reads[2] == 1 && count_reads[3] == 1);
            assert(log_calls == 0 && seek_calls == 0);
            assert(no_entries.close_calls == 2 && !no_entries.text_file_);

            reset_controls();
            file_contents = "no section here\n";
            index_counts[0] = 1;
            index_counts[2] = 1;
            index_results[0] = -7;
            index_results[2] = -1;
            Text all_miss;
            assert(all_miss.open("rules", "MISSING") == TRUE_VALUE);
            assert(search_calls == 2 && search_slots[0] == 0 && search_slots[1] == 2);
            assert(log_calls == 0 && seek_calls == 0);
            assert(count_reads[0] == 1 && count_reads[1] == 1);
            assert(count_reads[2] == 1 && count_reads[3] == 1);
        }
        static void check_negative_result_then_later_index_hit() {
            reset_controls();
            index_counts[1] = 1;
            index_counts[2] = 1;
            index_counts[3] = 1;
            index_results[1] = -7;
            index_results[2] = 0;
            index_results[3] = 4;
            Text text;
            assert(text.open("rules", "OPTIONS") == 0);
            assert(search_calls == 2 && search_slots[0] == 1 && search_slots[1] == 2);
            assert(count_reads[0] == 1 && count_reads[1] == 1 && count_reads[2] == 1);
            assert(count_reads[3] == 0);
            assert(log_calls == 1 && seek_calls == 1 && seek_offset == 0);
            text.close();
        }
        int main() {
            check_filename_and_section();
            check_null_section_and_open_failure();
            check_fallback_reopen_success_and_failure();
            check_header_precedes_mutating_index_search();
            check_existing_stream_and_seek();
            check_eof_rewind_then_close();
            check_zero_counts_and_all_misses_do_not_seek();
            check_negative_result_then_later_index_hit();
            return 0;
        }
        """)


def test_text_open_uses_original_copy_append_operations_and_order() -> None:
    body = text_open_body(SOURCE.read_text())
    assert "strcpy(file_name_, src_file_id);" in body
    assert 'strcat(file_name_, ".txt");' in body
    assert body.count("strcpy(file_path_, FilefindPath.last_path);") == 2
    assert 'strcpy(sect_header, "#");' in body
    assert "strcat(sect_header, section_id);" in body
    assert "strcpy_s" not in body and "strcat_s" not in body
    assert body.index("strcpy(file_name_, src_file_id);") < body.index("strchr(file_name_, '.')")
    assert body.index("strchr(file_name_, '.')") < body.index('strcat(file_name_, ".txt");')
    assert body.index("strcpy(sect_header, \"#\");") < body.index("strcat(sect_header, section_id);")
    assert body.index("strcat(sect_header, section_id);") < body.index("do {")
    assert body.index("strcat(sect_header, section_id);") < body.index(
        "int seek_addr = text_search_index(file_name_, section_id);")
    assert "TxtIndex[i].search_index(file_name_, section_id)" not in body


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_extracted_text_open_preserves_stream_and_parser_control_flow(tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    source = fixture(
        text_open_body(SOURCE.read_text()),
        text_search_body((ROOT / "src/textindex.h").read_text()),
    )
    cpp = tmp_path / "text_open.cpp"
    executable = tmp_path / "text_open"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True)
    assert ran.returncode == 0, ran.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_text_search_helper_rejects_wrong_miss_sentinel_and_early_exit(
    tmp_path: Path, compiler_name: str,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = text_search_body((ROOT / "src/textindex.h").read_text())
    sentry = "return -1;"
    assert body.count(sentry) == 1
    sentinel_mutant = body.replace(sentry, "return 0;", 1)
    break_clause = "if (address >= 0) {\n                return address;\n            }"
    assert body.count(break_clause) == 1
    early_exit_mutant = body.replace(break_clause, "return address;", 1)
    open_body = text_open_body(SOURCE.read_text())
    helper_call = "int seek_addr = text_search_index(file_name_, section_id);"
    assert open_body.count(helper_call) == 1
    for label, mutant in (("zero-sentinel", sentinel_mutant), ("early-exit", early_exit_mutant)):
        source = fixture(open_body, mutant)
        cpp = tmp_path / f"text_open_{label}.cpp"
        executable = tmp_path / f"text_open_{label}"
        cpp.write_text(source)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
            capture_output=True, text=True,
        )
        assert built.returncode == 0, built.stderr
        ran = subprocess.run([str(executable)], capture_output=True, text=True)
        assert ran.returncode != 0, f"{label} mutant unexpectedly passed all controls"


def test_pinned_original_has_the_five_ordinary_call_witnesses() -> None:
    pytest.importorskip("capstone")
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    image = PeImage(image_path.read_bytes(), PIN)
    instructions = Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x005FDA00, 0x005FDC04)), 0x005FDA00
    )
    calls = {
        instruction.address: int(instruction.op_str, 16)
        for instruction in instructions
        if instruction.mnemonic == "call" and instruction.op_str.startswith("0x")
    }
    assert {address: calls.get(address) for address in (0x005FDA19, 0x005FDAC8, 0x005FDAE6)} == {
        0x005FDA19: 0x00645460,
        0x005FDAC8: 0x00645460,
        0x005FDAE6: 0x00645460,
    }
    assert {address: calls.get(address) for address in (0x005FDA33, 0x005FDAF1)} == {
        0x005FDA33: 0x00645470,
        0x005FDAF1: 0x00645470,
    }
    by_address = {instruction.address: instruction for instruction in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x005FDA00, 0x005FDC04)), 0x005FDA00
    )}
    # The filename is this+0; append the suffix only when strchr finds no dot.
    for address, mnemonic, operands in (
        (0x005FDA03, "mov", "eax, dword ptr [esp + 0x58]"),
        (0x005FDA0F, "mov", "esi, ecx"),
        (0x005FDA17, "push", "eax"),
        (0x005FDA18, "push", "esi"),
        (0x005FDA1E, "push", "0x2e"),
        (0x005FDA20, "push", "esi"),
        (0x005FDA21, "call", "0x645dd0"),
        (0x005FDA29, "test", "eax, eax"),
        (0x005FDA2B, "jne", "0x5fda3b"),
        (0x005FDA2D, "push", "0x696e68"),
        (0x005FDA32, "push", "esi"),
    ):
        instruction = by_address[address]
        assert (instruction.mnemonic, instruction.op_str) == (mnemonic, operands)
    assert image.read(Span(0x00696E68, 0x00696E6D)) == b".txt\0"
    # Null input selects either an existing stream or the fallback reopen.
    assert (by_address[0x005FDA0D].mnemonic, by_address[0x005FDA0D].op_str) == ("cmp", "eax, edi")
    assert (by_address[0x005FDA15].mnemonic, by_address[0x005FDA15].op_str) == ("je", "0x5fda86")
    assert (by_address[0x005FDA86].mnemonic, by_address[0x005FDA86].op_str) == ("cmp", "dword ptr [esi + 0x154], edi")
    assert (by_address[0x005FDA8C].mnemonic, by_address[0x005FDA8C].op_str) == ("je", "0x5fda98")
    assert (by_address[0x005FDA98].mnemonic, by_address[0x005FDA98].op_str) == ("push", "0x696e80")
    assert (by_address[0x005FDA9D].mnemonic, by_address[0x005FDA9D].op_str) == ("push", "esi")
    assert (by_address[0x005FDA9E].mnemonic, by_address[0x005FDA9E].op_str) == ("call", "0x634bb0")
    # The fallback shares its file-path copy with the source-file path.
    assert (by_address[0x005FDABF].mnemonic, by_address[0x005FDABF].op_str) == ("lea", "ecx, [esi + 0x50]")
    assert (by_address[0x005FDAC2].mnemonic, by_address[0x005FDAC2].op_str) == ("push", "0x9b8398")
    assert (by_address[0x005FDAC7].mnemonic, by_address[0x005FDAC7].op_str) == ("push", "ecx")
    assert (by_address[0x005FDAE0].mnemonic, by_address[0x005FDAE0].op_str) == ("push", "0x696e84")
    assert (by_address[0x005FDAE5].mnemonic, by_address[0x005FDAE5].op_str) == ("push", "edx")
    assert (by_address[0x005FDAEF].mnemonic, by_address[0x005FDAEF].op_str) == ("push", "ebp")
    assert (by_address[0x005FDAF0].mnemonic, by_address[0x005FDAF0].op_str) == ("push", "eax")
    assert (by_address[0x005FDAE0].mnemonic, by_address[0x005FDAE0].op_str) == ("push", "0x696e84")
    assert (by_address[0x005FDAE5].mnemonic, by_address[0x005FDAE5].op_str) == ("push", "edx")
    assert (by_address[0x005FDAE6].mnemonic, by_address[0x005FDAE6].op_str) == ("call", "0x645460")
    assert (by_address[0x005FDAEF].mnemonic, by_address[0x005FDAEF].op_str) == ("push", "ebp")
    assert (by_address[0x005FDAF0].mnemonic, by_address[0x005FDAF0].op_str) == ("push", "eax")
    assert (by_address[0x005FDAF1].mnemonic, by_address[0x005FDAF1].op_str) == ("call", "0x645470")
    assert (by_address[0x005FDAF9].mnemonic, by_address[0x005FDAF9].op_str) == ("mov", "ebx, 0x9b7d08")
    assert (by_address[0x005FDB0A].mnemonic, by_address[0x005FDB0A].op_str) == ("call", "0x5fe120")
    assert 0x005FDAE6 < 0x005FDAF1 < 0x005FDAF9 < 0x005FDB0A
