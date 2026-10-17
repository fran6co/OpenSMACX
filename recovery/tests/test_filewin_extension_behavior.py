import pathlib
import shutil
import subprocess
import tempfile

import pytest


PROJECT_ROOT = pathlib.Path(__file__).parents[2]


def extract_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


HOST_PREFIX = r"""
#include <stddef.h>
#include <string.h>

static int observed_strlen_calls;
static int observed_strcat_calls;
static size_t observed_last_strlen;

static size_t traced_strlen(const char *text) {
    ++observed_strlen_calls;
    size_t length = 0;
    while (text[length] != '\0') {
        ++length;
    }
    observed_last_strlen = length;
    return length;
}

static char *traced_strcat(char *destination, const char *source) {
    ++observed_strcat_calls;
    // strcat(destination, destination) has undefined behavior even when the
    // byte has just been cleared. Model that observed call safely instead of
    // invoking overlapping libc strcat in this host test.
    if (destination == source) {
        return destination;
    }
    size_t end = 0;
    while (destination[end] != '\0') {
        ++end;
    }
    size_t index = 0;
    do {
        destination[end + index] = source[index];
    } while (source[index++] != '\0');
    return destination;
}

#define strlen traced_strlen
#define strcat traced_strcat

class FileWin {
public:
    int add_def_ext(char *extension);
    int extension_count_;
    // The sixth row safely models the adjacent bytes reached by the original
    // exact `count == 4` check if corrupted state supplies count == 5.
    char extension_filters_[6][5];
};

class AlphaSave {
public:
    int add_default_extension(char *extension);
    FileWin file_win_;
};

#define CHECK(condition) do { if (!(condition)) return __LINE__ % 200 + 1; } while (0)
"""


HOST_MAIN = r"""
int main() {
    FileWin null_case = {};
    null_case.extension_count_ = 2;
    strcpy(null_case.extension_filters_[2], "KEEP");
    int strlen_before = observed_strlen_calls;
    int strcat_before = observed_strcat_calls;
    CHECK(null_case.add_def_ext(0) == 0x10);
    CHECK(null_case.extension_count_ == 2);
    CHECK(strcmp(null_case.extension_filters_[2], "KEEP") == 0);
    CHECK(observed_strlen_calls == strlen_before);
    CHECK(observed_strcat_calls == strcat_before);

    FileWin rejected = {};
    rejected.extension_count_ = 1;
    strcpy(rejected.extension_filters_[1], "OLD");
    CHECK(rejected.add_def_ext((char *)"TOOLONG") == 3);
    CHECK(rejected.extension_count_ == 1);
    CHECK(rejected.extension_filters_[1][0] == '\0');
    CHECK(observed_last_strlen == 7);

    FileWin slots = {};
    const char *values[4] = {"FOUR", "A", "MAP", "SC"};
    for (int index = 0; index < 4; ++index) {
        CHECK(slots.extension_count_ == index);
        CHECK(slots.add_def_ext((char *)values[index]) == 0);
        CHECK(slots.extension_count_ == index + 1);
        CHECK(strcmp(slots.extension_filters_[index], values[index]) == 0);
    }
    CHECK(strcmp(slots.extension_filters_[0], "FOUR") == 0);

    FileWin full = {};
    full.extension_count_ = 4;
    strcpy(full.extension_filters_[0], "ONE");
    strcpy(full.extension_filters_[1], "TWO");
    strcpy(full.extension_filters_[2], "THR");
    strcpy(full.extension_filters_[3], "FOU");
    strlen_before = observed_strlen_calls;
    strcat_before = observed_strcat_calls;
    CHECK(full.add_def_ext(0) == 1);
    CHECK(full.add_def_ext((char *)"NEW") == 1);
    CHECK(full.extension_count_ == 4);
    CHECK(strcmp(full.extension_filters_[0], "ONE") == 0);
    CHECK(strcmp(full.extension_filters_[1], "TWO") == 0);
    CHECK(strcmp(full.extension_filters_[2], "THR") == 0);
    CHECK(strcmp(full.extension_filters_[3], "FOU") == 0);
    CHECK(observed_strlen_calls == strlen_before);
    CHECK(observed_strcat_calls == strcat_before);

    FileWin overfull = {};
    overfull.extension_count_ = 5;
    CHECK(overfull.add_def_ext((char *)"X") == 0);
    CHECK(overfull.extension_count_ == 6);
    CHECK(strcmp(overfull.extension_filters_[5], "X") == 0);

    FileWin alias = {};
    strcpy(alias.extension_filters_[0], "SELF");
    observed_last_strlen = 99;
    CHECK(alias.add_def_ext(alias.extension_filters_[0]) == 0);
    CHECK(observed_last_strlen == 0);
    CHECK(alias.extension_count_ == 1);
    CHECK(alias.extension_filters_[0][0] == '\0');

    AlphaSave dialog = {};
    CHECK(dialog.add_default_extension((char *)"SAV") == 0);
    CHECK(dialog.file_win_.extension_count_ == 1);
    CHECK(strcmp(dialog.file_win_.extension_filters_[0], "SAV") == 0);
    return 0;
}
"""


def compile_and_run(compiler: str, body: str, wrapper: str, directory: pathlib.Path, name: str):
    source = directory / f"{name}.cpp"
    binary = directory / name
    source.write_text(HOST_PREFIX + body + "\n" + wrapper + "\n" + HOST_MAIN)
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=30,
        check=False,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30, check=False)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_filewin_method_and_alphasave_forwarder_with_mutation_controls(compiler):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")

    filewin = (PROJECT_ROOT / "src/jackal/filewin.cpp").read_text()
    alphasave = (PROJECT_ROOT / "src/alphasave.cpp").read_text()
    body = extract_definition(filewin, "int FileWin::add_def_ext(")
    wrapper = extract_definition(alphasave, "int AlphaSave::add_default_extension(")

    mutations = {
        "reject_four": (
            "strlen(extension) > 4",
            "strlen(extension) >= 4",
        ),
        "clear_after_length": (
            "    extension_filters_[extension_count_][0] = '\\0';\n"
            "    if (strlen(extension) > 4) {\n"
            "        return 3;\n"
            "    }",
            "    if (strlen(extension) > 4) {\n"
            "        return 3;\n"
            "    }\n"
            "    extension_filters_[extension_count_][0] = '\\0';",
        ),
        "null_guard_precedence": (
            "    if (extension_count_ == 4) {\n"
            "        return 1;\n"
            "    }\n"
            "    if (extension == 0) {\n"
            "        return 0x10;\n"
            "    }",
            "    if (extension == 0) {\n"
            "        return 0x10;\n"
            "    }\n"
            "    if (extension_count_ == 4) {\n"
            "        return 1;\n"
            "    }",
        ),
        "overfull_guard": (
            "extension_count_ == 4",
            "extension_count_ >= 4",
        ),
        "null_clears_slot": (
            "    if (extension == 0) {\n"
            "        return 0x10;\n"
            "    }",
            "    if (extension == 0) {\n"
            "        extension_filters_[extension_count_][0] = '\\0';\n"
            "        return 0x10;\n"
            "    }",
        ),
        "missing_increment": (
            "    ++extension_count_;",
            "    extension_count_ += 0;",
        ),
    }

    with tempfile.TemporaryDirectory(prefix="filewin-add-ext-source-") as temp:
        directory = pathlib.Path(temp)
        result = compile_and_run(executable, body, wrapper, directory, "actual")
        assert result.returncode == 0, result.stderr

        for name, (before, after) in mutations.items():
            assert body.count(before) == 1, name
            damaged = body.replace(before, after, 1)
            result = compile_and_run(executable, damaged, wrapper, directory, name)
            assert result.returncode != 0, f"behavior checks did not detect {name} mutation"
