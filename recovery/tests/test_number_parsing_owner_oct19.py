"""Transport and safe behavior controls for the source-owned numeric helpers."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from test_parse_string_actual_source import extract_function


ROOT = Path(__file__).resolve().parents[2]
GENERAL = ROOT / "src/general.cpp"
NUMBER = ROOT / "src/number_parsing.cpp"
GENERAL_H = ROOT / "src/general.h"
BASE = "d62a5691064294364d349a1a794701bb589ab236"
TASK_BASE = "7e4903cff533e303aad48821a36c26c8c5ede2ab"
START = b"/*\nPurpose: Convert the binary string to an integer."
END = b"/*\nPurpose: This handles parsing the input string and storing it in the output."


def _git(path: str) -> bytes:
    result = subprocess.run(
        ["git", "show", f"{BASE}:{path}"], cwd=ROOT, check=True,
        capture_output=True,
    )
    return result.stdout


def _task_git(path: str) -> bytes:
    result = subprocess.run(
        ["git", "show", f"{TASK_BASE}:{path}"], cwd=ROOT, check=True,
        capture_output=True,
    )
    return result.stdout


def _block(data: bytes) -> bytes:
    assert data.count(START) == 1
    assert data.count(END) == 1
    begin = data.index(START)
    end = data.index(END)
    assert begin < end
    return data[begin:end]


def _prefix() -> bytes:
    original = _git("src/general.cpp")
    include = b'#include "stdafx.h"\n'
    assert original.startswith(b"/*") and original.count(include) >= 1
    return original[: original.index(include) + len(include)]


def _assert_transport(general: bytes, numeric: bytes) -> None:
    frozen_general = _git("src/general.cpp")
    original_block = _block(frozen_general)
    assert general.count(START) == 0 and general.count(END) == 1
    number_text = numeric.decode()
    historical_header = _git("src/general.h").decode()
    historical_binary = extract_function(
        historical_header, r"MEASURED\s+inline\s+int\s+__cdecl\s+btoi\s*\(")
    historical_hex = extract_function(
        historical_header, r"MEASURED\s+inline\s+int\s+__cdecl\s+htoi\s*\(")
    binary = extract_function(
        number_text, r"MEASURED\s+int\s+__cdecl\s+btoi\s*\(")
    hexadecimal = extract_function(
        number_text, r"MEASURED\s+int\s+__cdecl\s+htoi\s*\(")
    normalize_inline = lambda body: re.sub(
        r"\bMEASURED\s+inline\s+", "MEASURED ", body, count=1)
    assert binary == normalize_inline(historical_binary)
    assert hexadecimal == normalize_inline(historical_hex)
    assert "MEASURED inline int __cdecl btoi" not in number_text
    assert "MEASURED inline int __cdecl htoi" not in number_text
    header_text = GENERAL_H.read_text(encoding="utf-8")
    assert "MEASURED int __cdecl btoi(LPCSTR str);" in header_text
    assert "MEASURED int __cdecl htoi(LPCSTR str);" in header_text
    assert "MEASURED inline int __cdecl btoi" not in header_text
    assert "MEASURED inline int __cdecl htoi" not in header_text
    assert number_text.index(binary) < number_text.index("int __cdecl stoi(")
    assert number_text.index(hexadecimal) < number_text.index("int __cdecl stoi(")

    expected_numeric = _prefix() + b'#include "general.h"\n\n' + original_block
    for line in re.findall(rb"^// ORIGINAL: [^\n]+$", original_block, re.M):
        assert line in numeric
    assert general.count(b"// ORIGINAL: 0x00628950 ?stoi") == 0
    assert general.count(b"// ORIGINAL: 0x00628B30 ?findnum") == 0
    assert numeric.count(b"// ORIGINAL: 0x006288D0 ?btoi") == 1
    assert numeric.count(b"// ORIGINAL: 0x006288F0 ?htoi") == 1
    assert numeric.count(b"// ORIGINAL: 0x00628950 ?stoi") == 1
    assert numeric.count(b"// ORIGINAL: 0x00628B30 ?findnum") == 1
    assert numeric.count(b"// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628950") == 1
    assert numeric.count(b"// RECOVERY_ASSOCIATED: v1 switch_selector 0x00628950") == 1
    assert numeric.count(b'char findnum_digits[] = "09";  // 0x00670C1C') == 1
    assert b"// TRIED" in numeric and b"// LEVER" in numeric
    assert b"// ORIGINAL: 0x00625880 ?parse_string" in general
    assert b"int __cdecl parse_string" not in numeric
    current_parser = extract_function(general.decode(), r"int\s+__cdecl\s+parse_string\s*\(")
    original_parser = extract_function(_task_git("src/general.cpp").decode(), r"int\s+__cdecl\s+parse_string\s*\(")
    assert current_parser == original_parser, "parse_string body or inline table binding changed"
    expected_text = expected_numeric.decode()
    for signature in (r"int\s+__cdecl\s+stoi\s*\(", r"char\s*\*\s*__cdecl\s+findnum\s*\("):
        assert extract_function(number_text, signature) == extract_function(expected_text, signature)


def test_numeric_owner_transport_and_mapping_damage_controls():
    general = GENERAL.read_bytes()
    numeric = NUMBER.read_bytes()
    _assert_transport(general, numeric)

    damaged = numeric.replace(b"return atoi(str);", b"return 0;", 1)
    assert damaged != numeric
    try:
        _assert_transport(general, damaged)
    except AssertionError:
        pass
    else:
        raise AssertionError("changed source body passed exact owner transport check")

    damaged = numeric.replace(b"BYTE_EXACT", b"MISMATCH", 1)
    try:
        _assert_transport(general, damaged)
    except AssertionError:
        pass
    else:
        raise AssertionError("changed historical claim passed transport check")

    damaged = numeric.replace(b"// RECOVERY_ASSOCIATED: v1 switch_targets", b"// missing", 1)
    try:
        _assert_transport(general, damaged)
    except AssertionError:
        pass
    else:
        raise AssertionError("missing associated switch mapping passed transport check")

    duplicated = numeric + b"\n" + numeric[numeric.index(START):]
    try:
        _assert_transport(general, duplicated)
    except AssertionError:
        pass
    else:
        raise AssertionError("duplicate numeric mappings passed transport check")

    damaged = numeric.replace(b'char findnum_digits[] = "09";', b'char *findnum_digits = "09";', 1)
    try:
        _assert_transport(general, damaged)
    except AssertionError:
        pass
    else:
        raise AssertionError("changed findnum storage representation passed transport check")


HOST = r"""
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <cstring>
#define MEASURED
#define __cdecl
typedef const char *LPCSTR;
typedef char *LPSTR;
"""

MAIN = r"""
int main() {
    assert(btoi("101101") == 45);
    assert(btoi("10tail") == 2);
    assert(htoi("2f") == 47);
    assert(htoi("A0") == 160);
    assert(stoi("123") == 123);
    assert(stoi("0B101") == 5);
    assert(stoi("0b110") == 6);
    assert(stoi("0X2f") == 47);
    assert(stoi("0xA0") == 160);
    assert(stoi("0D42") == 42);
    assert(stoi("0d17") == 17);
    assert(stoi("0") == 0);
    // Keep the original helper's initial decrement within a real array.
    char one[] = "_prefix 42 tail";
    char none[] = "_no digits";
    char empty[] = "_";
    assert(findnum(one + 1) == one + 8);
    assert(findnum(none + 1) == 0);
    assert(findnum(empty + 1) == 0);
    assert(findnum(0) == 0);
}
"""


def test_numeric_owner_actual_bodies_safe_host_controls():
    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "neither GCC nor Clang is installed for host controls"
    number_text = NUMBER.read_text()
    binary = extract_function(number_text, r"MEASURED\s+int\s+__cdecl\s+btoi\s*\(")
    hexadecimal = extract_function(number_text, r"MEASURED\s+int\s+__cdecl\s+htoi\s*\(")
    decimal = extract_function(number_text, r"int\s+__cdecl\s+stoi\s*\(")
    finder = extract_function(number_text, r"char\s*\*\s*__cdecl\s+findnum\s*\(")
    digits, = re.findall(r'^char\s+findnum_digits\[\]\s*=\s*"09";[^\n]*$', number_text, re.M)
    source = HOST + "\n" + binary + "\n" + hexadecimal + "\n" + decimal + "\n" + digits + "\n" + finder + "\n" + MAIN
    with tempfile.TemporaryDirectory(prefix="osmx-number-owner-") as temp:
        src = Path(temp) / "number.cpp"
        exe = Path(temp) / "number"
        src.write_text(source)
        compiled = subprocess.run([compiler, "-std=c++17", "-O0", str(src), "-o", str(exe)], capture_output=True, text=True, timeout=45)
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
        assert ran.returncode == 0, ran.stdout + ran.stderr
