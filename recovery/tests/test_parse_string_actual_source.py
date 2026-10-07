"""Host controls that compile parse_string and its helpers from installed source."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile


def repository_root():
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src/general.cpp").is_file():
            return candidate
    raise AssertionError("could not locate OpenSMACX source root")


ROOT = repository_root()
SOURCE = ROOT / "src/general.cpp"


def extract_function(source, signature):
    matches = list(re.finditer(signature, source))
    assert matches, f"missing installed function matching {signature!r}"
    match = None
    opening = -1
    for candidate in matches:
        paren = source.find("(", candidate.start())
        depth = 0
        close = -1
        for position in range(paren, len(source)):
            if source[position] == "(":
                depth += 1
            elif source[position] == ")":
                depth -= 1
                if depth == 0:
                    close = position + 1
                    break
        if close < 0:
            continue
        position = close
        while position < len(source) and source[position].isspace():
            position += 1
        if source.startswith("const", position):
            position += len("const")
            while position < len(source) and source[position].isspace():
                position += 1
        if position < len(source) and source[position] == "{":
            match = candidate
            opening = position
            break
    assert match is not None, f"no installed function definition matching {signature!r}"
    depth = 0
    quote = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and next_char == "/":
                block_comment = False
                index += 1
        elif quote is not None:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and next_char == "/":
            line_comment = True
            index += 1
        elif char == "/" and next_char == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():index + 1]
        index += 1
    raise AssertionError(f"unterminated installed function {signature!r}")


PREAMBLE = r"""
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
typedef char *LPSTR;
typedef const char *LPCSTR;
#define MEASURED
#define __cdecl
struct char256 { char str[256]; };
static int ParseNumTable[10];
static int ParseStrPlurality[10];
static int ParseStrGender[10];
static char256 ParseStrBuffer[10];
static uint32_t Language;
static char findnum_digits[] = "09";
static void strcat_s(char *dst, size_t, const char *src) { std::strcat(dst, src); }
static void _itoa_s(int value, char *dst, size_t, int radix) {
    // The project's VC6 shim calls unbounded _itoa; size is not enforced.
    if (radix == 10) std::sprintf(dst, "%d", value);
}
"""


MAIN = r"""
static int run(const char *input, char *output) {
    std::memset(output, 0, 4096);
    return parse_string(const_cast<char *>(input), output);
}

int main() {
    ParseNumTable[0] = 123456789;
    ParseNumTable[1] = -42;
    ParseNumTable[2] = 1;
    ParseNumTable[3] = 7;
    ParseStrBuffer[0] = {{0}};
    ParseStrBuffer[1] = {{0}};
    ParseStrBuffer[2] = {{0}};
    ParseStrBuffer[3] = {{0}};
    std::strcpy(ParseStrBuffer[0].str, "alpha");
    std::strcpy(ParseStrBuffer[1].str, "he:she:they");
    std::strcpy(ParseStrBuffer[2].str, "unit:units");
    std::strcpy(ParseStrBuffer[3].str, "beta");

    char out[4096];
    assert(run("literal tail", out) == 0 && !std::strcmp(out, "literal tail"));
    assert(run("A $$ B", out) == 0 && !std::strcmp(out, "A $ B"));
    assert(run("v=$HEX0", out) == 0 && !std::strcmp(out, "v=123456789"));
    assert(run("$NUMBER1", out) == 0 && !std::strcmp(out, "-42"));
    assert(run("$NUM3!", out) == 0 && !std::strcmp(out, "7!"));
    ParseNumTable[4] = INT32_MIN;
    ParseNumTable[5] = INT32_MAX;
    assert(run("$HEX4", out) == 0 && !std::strcmp(out, "-2147483648"));
    assert(run("$NUMBER4", out) == 0 && !std::strcmp(out, "-2147483648"));
    assert(run("$NUM4", out) == 0 && !std::strcmp(out, "-2147483648"));
    assert(run("$HEX5", out) == 0 && !std::strcmp(out, "2147483647"));
    assert(run("$NUMBER5", out) == 0 && !std::strcmp(out, "2147483647"));
    assert(run("$NUM5", out) == 0 && !std::strcmp(out, "2147483647"));
    assert(run("$H0", out) == 0 && !std::strcmp(out, "$alpha"));
    assert(run("$N3", out) == 0 && !std::strcmp(out, "beta"));
    assert(run("$<F:1>", out) == 0 && !std::strcmp(out, "she"));
    assert(run("$<M:1>", out) == 0 && !std::strcmp(out, "he"));
    assert(run("$<N:1>", out) == 0 && !std::strcmp(out, "they"));

    ParseStrGender[2] = 0;
    ParseStrPlurality[2] = 0;
    assert(run("$<2:2>", out) == 0 && !std::strcmp(out, "unit"));
    ParseStrPlurality[2] = 1;
    assert(run("$<2:2>", out) == 0 && !std::strcmp(out, "units"));
    ParseStrPlurality[2] = 0;
    assert(run("$<2:2>", out) == 0 && !std::strcmp(out, "unit"));
    std::strcpy(ParseStrBuffer[0].str, "unit:units");
    assert(run("$<#0:0>", out) == 0 && !std::strcmp(out, "units"));
    std::strcpy(ParseStrBuffer[0].str, "alpha");
    assert(run("$<M:single:plural>", out) == 0 && !std::strcmp(out, "single"));
    assert(run("$<F:a:b:c>", out) == 0 && !std::strcmp(out, "b"));
    assert(run("$<M:a:b:c:d>", out) == 0 && !std::strcmp(out, "a"));
    assert(run("$<F:a:b:c:d>", out) == 0 && !std::strcmp(out, "b"));
    assert(run("$<M2:a:b:c:d>", out) == 0 && !std::strcmp(out, "c"));
    assert(run("$<F2:a:b:c:d>", out) == 0 && !std::strcmp(out, "d"));
    assert(run("$<N2:a:b:c:d>", out) == 0 && !std::strcmp(out, "d"));
    assert(run("$<M:a:b:c:d:e:f>", out) == 0 && !std::strcmp(out, "a"));
    assert(run("$<F:a:b:c:d:e:f>", out) == 0 && !std::strcmp(out, "b"));
    assert(run("$<N:a:b:c:d:e:f>", out) == 0 && !std::strcmp(out, "c"));
    assert(run("$<M2:a:b:c:d:e:f>", out) == 0 && !std::strcmp(out, "d"));
    assert(run("$<F2:a:b:c:d:e:f>", out) == 0 && !std::strcmp(out, "e"));
    assert(run("$<N2:a:b:c:d:e:f>", out) == 0 && !std::strcmp(out, "f"));
    assert(run("A $<missing", out) == 0 && !std::strcmp(out, "A <missing"));
    assert(run("A $<x:0>Z", out) == 0 && !std::strcmp(out, "A Z"));

    Language = 1;
    assert(run("va de arbre", out) == 0 && !std::strcmp(out, "va d'arbre"));
    assert(run("De arbre", out) == 0 && !std::strcmp(out, "D'arbre"));
    assert(run("va de maison", out) == 0 && !std::strcmp(out, "va de maison"));
    Language = 0;
    assert(run("va de arbre", out) == 0 && !std::strcmp(out, "va de arbre"));

    std::strcpy(out, "unchanged");
    assert(parse_string(const_cast<char *>("$HEX:"), out) == 14);
    assert(!std::strcmp(out, "unchanged"));
    std::puts("installed-source parse_string controls passed");
}
"""


def compile_and_run(compiler, parser_override=None, expect_success=True):
    source_text = SOURCE.read_text()
    parser = extract_function(source_text, r"int\s+__cdecl\s+parse_string\s*\(")
    if parser_override is not None:
        parser = parser_override
    finder = extract_function(source_text, r"char\s*\*\s*__cdecl\s+findnum\s*\(")
    purger = extract_function(source_text, r"MEASURED\s+inline\s+LPSTR\s+__cdecl\s+purge_leading\s*\(")
    source = PREAMBLE + "\n" + purger + "\n" + finder + "\n" + parser + "\n" + MAIN
    with tempfile.TemporaryDirectory(prefix="osmx-parse-string-") as temp:
        source_path = Path(temp) / "parse_string.cpp"
        binary_path = Path(temp) / "parse_string"
        source_path.write_text(source)
        compiled = subprocess.run(
            [compiler, "-std=c++17", "-O0", str(source_path), "-o", str(binary_path)],
            capture_output=True,
            text=True,
            timeout=45,
        )
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        ran = subprocess.run([str(binary_path)], capture_output=True, text=True, timeout=10)
        if expect_success:
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "mutated parser unexpectedly passed all controls"


def test_installed_parse_string_actual_source_host_controls():
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "neither GCC nor Clang is installed for host controls"
    for compiler in compilers:
        compile_and_run(compiler)
        source_text = SOURCE.read_text()
        parser = extract_function(source_text, r"int\s+__cdecl\s+parse_string\s*\(")

        no_tail = parser.replace("strcat(output, input);", "/* tail omitted */", 1)
        assert no_tail != parser
        compile_and_run(compiler, no_tail, expect_success=False)

        fixed_gender = parser.replace("alternative = gender;", "alternative = 0;", 1)
        assert fixed_gender != parser
        compile_and_run(compiler, fixed_gender, expect_success=False)

        reversed_axes = parser.replace(
            "alternative = (gender != 0) + (plurality != 0 ? 2 : 0);",
            "alternative = (plurality != 0) + (gender != 0 ? 2 : 0);", 1)
        assert reversed_axes != parser
        compile_and_run(compiler, reversed_axes, expect_success=False)

        no_french = parser.replace("if (Language == 1)", "if (Language == 2)", 1)
        assert no_french != parser
        compile_and_run(compiler, no_french, expect_success=False)
