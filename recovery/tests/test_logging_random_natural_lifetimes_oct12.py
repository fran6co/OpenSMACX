"""Host witnesses for the original Log and Random global lifetimes."""
from pathlib import Path
import json
import re
import shutil
import subprocess
import textwrap

import pytest
from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/log.h").is_file())
LOG_H = ROOT / "src/log.h"
RANDOM_H = ROOT / "src/random.h"
LOG_CPP = ROOT / "src/log.cpp"
RANDOM_CPP = ROOT / "src/random.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BASELINE = "54738d3db40bc2903e3b2a1e53fc2026f6f86d20"
TASK = ROOT / "recovery/tasks/logging-random-natural-lifetimes-oct12.json"


def class_definition(source, name):
    start = source.index("class " + name + " {")
    depth = 0
    for index in range(source.index("{", start), len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1] + ";"
    raise AssertionError("unterminated class " + name)


def product_global_declaration(source_path, type_name, variable_name):
    lines = [line.split("//", 1)[0].strip()
             for line in source_path.read_text().splitlines()
             if re.match(rf"^{re.escape(type_name)}\s+{re.escape(variable_name)}\b", line)]
    assert len(lines) == 1, (source_path, variable_name, lines)
    return lines[0]


def compile_and_run(tmp_path, source):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    cpp = tmp_path / "lifetime.cpp"
    binary = tmp_path / "lifetime"
    cpp.write_text(source)
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)], capture_output=True, timeout=30)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    return subprocess.run([str(binary)], capture_output=True, timeout=10)


def log_host_source(body):
    return textwrap.dedent(r"""
        #include <cstddef>
        #include <cstdio>
        #include <cstdlib>
        #include <cstring>
        #include <string>
        #include <vector>
        using std::size_t;
        using std::strcat;
        using std::strlen;
        typedef const char *LPCSTR;
        typedef char *LPSTR;
        typedef int BOOL;
        #define MEASURED
        static std::vector<std::string> events;
        static bool allocation_fails;
        static bool open_fails;
        static size_t allocation_size;
        static const char *opened_name;
        static const char *opened_mode;
        static int close_count;
        static int free_count;
        BOOL IsLoggingDisabled = 0;
        void *mem_get(size_t bytes) {
            events.push_back("alloc");
            allocation_size = bytes;
            return allocation_fails ? 0 : std::malloc(bytes);
        }
        FILE *env_open(LPCSTR name, LPCSTR mode) {
            events.push_back("open");
            opened_name = name;
            opened_mode = mode;
            return open_fails ? 0 : reinterpret_cast<FILE *>(1);
        }
        int host_fclose(FILE *) {
            events.push_back("close");
            ++close_count;
            return 0;
        }
        void host_free(void *pointer) {
            events.push_back("free");
            ++free_count;
            std::free(pointer);
        }
        int fprintf_s(FILE *, const char *, ...) { return 0; }
        #define fclose host_fclose
        #define free host_free
        #define private public
    """) + body + "\n"


def test_filename_constructor_preserves_original_paths_and_destructor(tmp_path):
    log_class = class_definition(LOG_H.read_text(), "Log")
    checks = r"""
        int main() {
            {
                Log logger(static_cast<LPCSTR>(0));
                if (logger.log_file_ != 0 || !events.empty()) return 1;
            }
            if (free_count != 0) return 2;
            allocation_fails = true;
            {
                Log logger("abc");
                if (logger.log_file_ != 0 || allocation_size != 4) return 3;
                if (events.size() != 1 || events[0] != "alloc") return 4;
            }
            if (free_count != 0) return 5;
            events.clear();
            allocation_fails = false;
            open_fails = false;
            {
                Log logger("abc");
                if (!logger.log_file_ || std::strcmp(logger.log_file_, "abc")) return 6;
                if (opened_name != std::string("abc") || std::strcmp(opened_mode, "wt")) return 7;
                if (events.size() != 3 || events[0] != "alloc" ||
                    events[1] != "open" || events[2] != "close") return 8;
            }
            if (free_count != 1) return 9;
            events.clear();
            open_fails = true;
            {
                Log logger("logfile.txt");
                if (!logger.log_file_ || std::strcmp(logger.log_file_, "logfile.txt")) return 10;
                if (events.size() != 2 || events[0] != "alloc" || events[1] != "open") return 11;
            }
            if (free_count != 2 || close_count != 1) return 12;
            return 0;
        }
    """
    source = log_host_source(log_class + checks)
    result = compile_and_run(tmp_path, source)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_global_log_and_random_objects_own_startup_and_reverse_cleanup(tmp_path):
    log_class = class_definition(LOG_H.read_text(), "Log")
    logging_declaration = product_global_declaration(LOG_CPP, "Log", "Logging")
    source = log_host_source(log_class + textwrap.dedent(r"""
        struct CleanupObserver {
            ~CleanupObserver() { std::fprintf(stderr, "%d\n", free_count); }
        } observer;
    """) + logging_declaration + textwrap.dedent(r"""
        int main() {
            if (!Logging.log_file_ || std::strcmp(Logging.log_file_, "logfile.txt")) return 1;
            if (opened_name != std::string("logfile.txt") || std::strcmp(opened_mode, "wt")) return 2;
            if (events.size() != 3 || events[0] != "alloc" ||
                events[1] != "open" || events[2] != "close") return 3;
            return 0;
        }
    """))
    result = compile_and_run(tmp_path, source)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    assert result.stderr.decode().strip() == "1", "Log global destructor did not release its filename"

    random_class = class_definition(RANDOM_H.read_text(), "Random")
    rand_declaration = product_global_declaration(RANDOM_CPP, "Random", "Rand")
    random_source = textwrap.dedent(r"""
        #include <cstdint>
        #include <cstdio>
        #include <cstring>
        #define MEASURED
    """) + random_class + textwrap.dedent(r"""
        extern Random Rand;
        struct CleanupObserver {
            ~CleanupObserver() {
                uint32_t raw_seed;
                std::memcpy(&raw_seed, static_cast<const void *>(&Rand), sizeof(raw_seed));
                std::fprintf(stderr, "%u\n", raw_seed);
            }
        } observer;
    """) + rand_declaration + textwrap.dedent(r"""
        int main() {
            if (Rand.get_seed() != 0) return 1;
            Rand.reseed(123);
            return Rand.get_seed() == 123 ? 0 : 2;
        }
    """)
    result = compile_and_run(tmp_path, random_source)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    assert result.stderr.decode().strip() == "0", "Random global destructor did not clear its seed"


def test_pinned_lifetime_annotations_match_baseline_parser_metadata(tmp_path):
    task = json.loads(TASK.read_text())
    assert task["base_revision"] == BASELINE
    assert all(subject["image_sha256"] == PIN for subject in task["subjects"])
    expected = {
        0x625700: ("??__ERand@@YAXXZ", ((0x625700, 0x625716),), "game", True),
        0x625720: ("??__FRand@@YAXXZ", ((0x625720, 0x62572B),), "game", True),
        0x625F20: ("??__ELogging@@YAXXZ", ((0x625F20, 0x625F8B),), "game", True),
        0x625F90: ("??__FLogging@@YAXXZ", ((0x625F90, 0x625FAD),), "game", True),
        0x625FC0: ("??0Log@@QAE@PAD@Z", ((0x625FC0, 0x626016),), "game", True),
    }
    assert {subject["address"] for subject in task["subjects"]} == set(expected)

    def metadata(rows):
        return {row.address: (row.name, row.image_spans, row.kind, row.byte_exact)
                for row in rows if row.address in expected}

    baseline_rows = []
    for relative in ("src/log.cpp", "src/random.cpp"):
        text = subprocess.check_output(
            ["git", "-C", str(ROOT), "show", f"{BASELINE}:{relative}"]).decode()
        baseline_path = tmp_path / ("baseline_" + Path(relative).name)
        baseline_path.write_text(text)
        baseline_rows.extend(read_file(baseline_path))
    current_rows = read_file(LOG_CPP) + read_file(RANDOM_CPP)
    assert metadata(baseline_rows) == expected
    assert metadata(current_rows) == expected

    log_header = LOG_H.read_text()
    log_source = LOG_CPP.read_text()
    random_header = RANDOM_H.read_text()
    random_source = RANDOM_CPP.read_text()
    assert "MEASURED Log(LPCSTR input)" in log_header
    ctor = log_header.split("MEASURED Log(LPCSTR input)", 1)[1].split("MEASURED ~Log()", 1)[0]
    assert "is_disabled_" not in ctor
    assert product_global_declaration(LOG_CPP, "Log", "Logging") == 'Log Logging("logfile.txt");'
    assert product_global_declaration(RANDOM_CPP, "Random", "Rand") == "Random Rand;"
    for obsolete in ("log_logging()", "log_logging_exit()", "random_rand()", "random_rand_exit()"):
        assert obsolete not in log_header + log_source + random_header + random_source
    code = re.sub(r"/\*.*?\*/|//[^\n]*", "", log_source, flags=re.S)
    assert "Logging.construct(" not in code
