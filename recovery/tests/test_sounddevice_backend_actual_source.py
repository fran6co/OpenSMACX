from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery import binding_source
from match_recovery.source import read_file
from match_recovery.source.model import SourceRole, State


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/sounddevice.cpp").is_file()
)
ADDRESSES = (0x004C5A80, 0x004C5AA0)


def extract_braced(text: str, start: int) -> str:
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
    raise AssertionError("unterminated C++ class")


def declarations(body: str) -> list[tuple[str, str, str]]:
    found = []
    for line in body.splitlines():
        line = line.split("//", 1)[0].strip()
        match = re.fullmatch(
            r"virtual\s+(int|void)\s+(\w+)\s*\(([^;{}]*)\)\s*=\s*0\s*;",
            line,
        )
        if match:
            found.append((match.group(1), match.group(2), match.group(3).strip()))
    return found


def source_records():
    path = ROOT / "src/sounddevice.cpp"
    records = {record.address: record for record in read_file(path)}
    return {address: records[address] for address in ADDRESSES}


def dispatch_surface_is_valid(header: str, cpp: str) -> bool:
    """Check the narrow source view, not the external DLL's full ABI."""
    declaration = re.search(r"class\s+WaveInDeviceDispatchView\b[^;{]*{", header)
    if not declaration:
        return False
    slots = declarations(extract_braced(header, declaration.start()))
    expected = [
        ("void", f"ordinal_placeholder_{index:02}", "")
        for index in range(24)
    ] + [
        ("int", "set_codec", "unsigned long codec"),
        ("int", "set_vxw_key", "signed char *key"),
    ]
    if (slots != expected or "sizeof(WaveInDeviceDispatchView)" in header
            or "SoundDeviceBase *device_14_;" not in header):
        return False
    for signature, call in (
        ("int Wave_In_Device::set_codec(unsigned long codec)",
         "return delegate->set_codec(codec);"),
        ("int Wave_In_Device::set_vxw_key(signed char *key)",
         "return delegate->set_vxw_key(key);"),
    ):
        start = cpp.find(signature)
        if start < 0:
            return False
        try:
            body = extract_braced(cpp, start)
        except AssertionError:
            return False
        if "reinterpret_cast<WaveInDeviceDispatchView *>(device_14_)" not in body:
            return False
        if "if (delegate == 0)" not in body or "return 0x13;" not in body:
            return False
        if call not in body:
            return False
    return True


def mock_methods(slots: list[tuple[str, str, str]]) -> str:
    # Host-only mock implements the source view's placeholder surface. It is
    # not evidence for the external DLL's signatures, size, or vtable contract.
    lines = ["struct FakeWaveInBackend : WaveInDeviceDispatchView {"]
    for result, name, _parameters in slots:
        if name == "set_codec":
            lines.append(
                "  int set_codec(unsigned long value) override { "
                "codec = value; return 47; }"
            )
        elif name == "set_vxw_key":
            lines.append(
                "  int set_vxw_key(signed char *value) override { "
                "key = value; return -9; }"
            )
        elif result == "void":
            lines.append(f"  void {name}() override {{}}")
        else:
            lines.append(f"  int {name}() override {{ return 0; }}")
    lines.extend([
        "  unsigned long codec = 0;",
        "  signed char *key = 0;",
        "};",
    ])
    return "\n".join(lines)


def test_backend_calls_use_the_actual_typed_interface_and_source_bodies(tmp_path):
    compiler = shutil.which("c++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")

    records = source_records()
    assert set(records) == set(ADDRESSES)
    names = {
        0x004C5A80: "?set_codec@Wave_In_Device@@QAEHK@Z",
        0x004C5AA0: "?set_vxw_key@Wave_In_Device@@QAEHPAC@Z",
    }
    for address, record in records.items():
        assert record.path == ROOT / "src/sounddevice.cpp"
        assert record.name == names[address]
        assert record.state is State.IMPLEMENTED
        assert record.source_role is SourceRole.SCAFFOLD
        assert "ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5" in record.source_role_reason
        assert "exports 9/10" in record.source_role_reason
        assert "26-entry" in record.source_role_reason
        assert "preceding ordinal placeholders are not recovered signatures" in record.source_role_reason
        assert "complete external class extent" in record.source_role_reason
        assert record.image_spans == ((address, address + 32),)
        assert not record.byte_exact and not record.semantic
        assert record.region
        assert f"0x{address:08X}" in record.region

    header = (ROOT / "src/sounddevice.h").read_text()
    declaration = re.search(r"class\s+WaveInDeviceDispatchView\b[^;{]*{", header)
    assert declaration, "two-method dispatch view is absent from installed source"
    interface = "class WaveInDeviceDispatchView {" + extract_braced(
        header, declaration.start()
    ) + "};"
    slots = declarations(extract_braced(interface, interface.index("class")))
    assert len(slots) == 26
    assert [name for _result, name, _parameters in slots[:24]] == [
        f"ordinal_placeholder_{index:02}" for index in range(24)
    ]
    assert slots[24] == ("int", "set_codec", "unsigned long codec")
    assert slots[25] == ("int", "set_vxw_key", "signed char *key")
    assert "sizeof(WaveInDeviceDispatchView)" not in header

    cpp_source = (ROOT / "src/sounddevice.cpp").read_text()
    assert dispatch_surface_is_valid(header, cpp_source)
    bindings = binding_source.parse(
        cpp_source, read_file(ROOT / "src/sounddevice.cpp")
    )
    expected_binding_lines = {
        record.line: address for address, record in records.items()
    }
    assert set(bindings) == set(expected_binding_lines)
    for line, binding in bindings.items():
        address = expected_binding_lines[line]
        assert "soundx.dll SHA-256 ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5" in binding["evidence"][0]
        assert "ordinal 9 create_device" in binding["evidence"][0]
        assert "ordinal 10 delete_device" in binding["evidence"][0]
        assert "26-entry table" in binding["evidence"][1]
        assert "entries 24 and 25" in binding["evidence"][1]
        assert f"0x{address:08X}-0x{address + 32:08X}" in binding["evidence"][2]
        assert "complete class extent" in binding["evidence"][4]
        assert "only" in binding["reason"]

    mutations = (
        header.replace("int set_codec(unsigned long codec) = 0;",
                       "int set_vxw_key(signed char *key) = 0;", 1),
        header.replace(
            "virtual int set_codec(unsigned long codec) = 0;\n"
            "  virtual int set_vxw_key(signed char *key) = 0;",
            "virtual int set_vxw_key(signed char *key) = 0;\n"
            "  virtual int set_codec(unsigned long codec) = 0;",
            1,
        ),
        cpp_source.replace("if (delegate == 0)", "if (delegate != 0)", 1),
        cpp_source.replace("delegate->set_codec(codec)",
                           "delegate->set_codec(codec + 1)", 1),
        header.replace("SoundDeviceBase *device_14_;", "void *device_14_;", 1),
        cpp_source.replace("reinterpret_cast<WaveInDeviceDispatchView *>(device_14_)",
                           "static_cast<WaveInDeviceDispatchView *>(device_14_)", 1),
        cpp_source.replace("return delegate->set_codec(codec);",
                           "delegate->set_codec(codec); return 0;", 1),
    )
    for mutant in mutations:
        mutated_header = mutant if ("class WaveInDeviceDispatchView" in mutant
                              or "SoundDeviceBase *device_14_" in mutant
                              or "void *device_14_" in mutant) else header
        mutated_cpp = mutant if ("Wave_In_Device::set_codec" in mutant
                                 or "Wave_In_Device::set_vxw_key" in mutant
                                 or "reinterpret_cast<WaveInDeviceDispatchView" in mutant) else cpp_source
        assert not dispatch_surface_is_valid(mutated_header, mutated_cpp)

    assert re.search(r"int\s+set_codec\(unsigned long\s+codec\)\s*;", header)
    assert re.search(r"int\s+set_vxw_key\(signed char\s*\*\s*key\)\s*;", header)
    owner = """
struct SoundDeviceBase { virtual int init(void *, unsigned long) = 0; };
struct Wave_In_Device {
  SoundDeviceBase *device_14_;
  int set_codec(unsigned long codec);
  int set_vxw_key(signed char *key);
};
"""
    cpp = "\n".join([
        "#include <cassert>", interface, owner, mock_methods(slots),
        records[ADDRESSES[0]].region,
        records[ADDRESSES[1]].region,
        "int main() {",
        "  Wave_In_Device device = {};",
        "  assert(device.set_codec(5) == 0x13);",
        "  signed char key[] = {1, 2, 0};",
        "  assert(device.set_vxw_key(key) == 0x13);",
        "  FakeWaveInBackend backend;",
        "  device.device_14_ = reinterpret_cast<SoundDeviceBase *>(&backend);",
        "  assert(device.set_codec(0x1234) == 47);",
        "  assert(backend.codec == 0x1234);",
        "  assert(device.set_vxw_key(key) == -9);",
        "  assert(backend.key == key);",
        "  return 0;",
        "}",
    ])
    cpp_path = tmp_path / "sounddevice_backend_control.cpp"
    binary = tmp_path / "sounddevice_backend_control"
    cpp_path.write_text(cpp)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp_path), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
