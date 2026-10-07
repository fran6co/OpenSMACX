from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SLOTS = (
    "create_device", "delete_device", "unk_slot2", "init", "release",
    "get_ndevices", "select", "get_description",
)


def extract_braced(text: str, start: int) -> str:
    opening = text.index("{", start)
    depth = 0
    state = "code"
    escaped = False
    i = opening
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""
        if state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        else:
            if ch == "/" and nxt == "/":
                state = "line"
                i += 1
            elif ch == "/" and nxt == "*":
                state = "block"
                i += 1
            elif ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return text[opening + 1:i]
        i += 1
    raise AssertionError("unterminated C++ brace scope")


def class_body(text: str, name: str) -> str:
    match = re.search(rf"\bclass\s+{re.escape(name)}\b[^;{{]*{{", text)
    assert match, f"class {name} not found"
    return extract_braced(text, match.start())


def method_declarations(body: str) -> dict[str, tuple[str, str, bool, bool]]:
    methods: dict[str, tuple[str, str, bool, bool]] = {}
    for line in body.splitlines():
        line = line.split("//", 1)[0].strip()
        match = re.fullmatch(
            r"(virtual\s+)?(int|void)\s+(\w+)\s*\(([^;{}]*)\)\s*(=\s*0)?\s*;",
            line,
        )
        if match and match.group(3) in SLOTS:
            assert match.group(3) not in methods, f"duplicate virtual slot {match.group(3)}"
            methods[match.group(3)] = (
                match.group(2), match.group(4).strip(), bool(match.group(1)),
                bool(match.group(5)),
            )
    return methods


def erase_parameter_names(parameters: str) -> str:
    if not parameters:
        return ""
    type_words = {"unsigned", "signed", "int", "long", "short", "char",
                  "void", "float", "double", "bool"}
    result = []
    for parameter in parameters.split(","):
        parameter = parameter.strip()
        parameter = re.sub(r"\*\s*[A-Za-z_]\w*$", "*", parameter)
        match = re.search(r"\s+([A-Za-z_]\w*)$", parameter)
        if match and match.group(1) not in type_words:
            parameter = parameter[:match.start()].rstrip()
        result.append(parameter)
    return ", ".join(result)


def sound_headers():
    sound = (ROOT / "src/sounddevice.h").read_text()
    wave = (ROOT / "src/wave_device.h").read_text()
    return sound, wave


def interface_model(sound: str, wave: str):
    base = method_declarations(class_body(sound, "SoundDeviceBase"))
    owners = {
        "Midi_Device": method_declarations(class_body(sound, "Midi_Device")),
        "Wave_In_Device": method_declarations(class_body(sound, "Wave_In_Device")),
        "Wave_Device": method_declarations(class_body(wave, "Wave_Device")),
    }
    return base, owners


def type_errors(base, owners):
    errors = []
    if list(base) != list(SLOTS):
        errors.append("base virtual slot order/set differs")
    for slot in SLOTS:
        if slot in base and not base[slot][2]:
            errors.append(f"base {slot} is not declared virtual")
        if slot in base and not base[slot][3]:
            errors.append(f"base {slot} is not pure virtual")
    for owner, methods in owners.items():
        for slot in SLOTS:
            if slot not in methods:
                errors.append(f"{owner} missing {slot}")
            elif slot in base and (
                methods[slot][0], erase_parameter_names(methods[slot][1])
            ) != (
                base[slot][0], erase_parameter_names(base[slot][1])
            ):
                errors.append(f"{owner} prototype mismatch at {slot}")
    return errors


def generated_cpp(base, owners):
    def decl(name, item, body):
        result, params, _, _ = item
        params = erase_parameter_names(params)
        tail = "return;" if result == "void" else f"return {body};"
        return f"  {result} {name}({params}) override {{ {tail} }}"

    lines = [
        "#include <cassert>",
        "#include <type_traits>",
        "struct ControlSoundDeviceBase {",
    ]
    for slot in SLOTS:
        result, params, _, _ = base[slot]
        lines.append(
            f"  virtual {result} {slot}({erase_parameter_names(params)}) = 0;"
        )
    lines.append("protected: ~ControlSoundDeviceBase() {};")
    lines.append("};")
    ids = {"Midi_Device": 1, "Wave_In_Device": 2, "Wave_Device": 3}
    for owner, methods in owners.items():
        ctrl = "Control_" + owner
        lines.append(f"struct {ctrl} : ControlSoundDeviceBase {{")
        for index, slot in enumerate(SLOTS):
            lines.append(decl(slot, methods[slot], ids[owner] * 100 + index))
        lines.append("};")
        lines.append(
            f"static_assert(!std::is_abstract<{ctrl}>::value, \"{owner} concrete\");"
        )
    lines.extend([
        "int main() {",
        "  Control_Midi_Device midi; Control_Wave_In_Device wave_in; Control_Wave_Device wave;",
        "  ControlSoundDeviceBase *items[] = {&midi, &wave_in, &wave};",
        "  assert(items[0]->select(0) == 106 && items[0]->get_description(0, 0, 0) == 107);",
        "  assert(items[1]->select(0) == 206 && items[1]->get_description(0, 0, 0) == 207);",
        "  assert(items[2]->select(0) == 306 && items[2]->get_description(0, 0, 0) == 307);",
        "  assert(items[0]->create_device(0) == 100 && items[2]->create_device(0) == 300);",
        "  return 0;",
        "}",
    ])
    return "\n".join(lines) + "\n"


def test_all_three_actual_headers_have_one_exact_eight_slot_interface():
    sound, wave = sound_headers()
    base, owners = interface_model(sound, wave)
    assert not type_errors(base, owners)
    for owner in owners:
        assert f"class {owner} : public SoundDeviceBase" in (sound if owner != "Wave_Device" else wave)
    base_text = class_body(sound, "SoundDeviceBase")
    assert "virtual ~SoundDeviceBase" not in base_text


def test_actual_shared_fields_and_lifecycle_order_are_consistent():
    sound, wave = sound_headers()
    base_text = class_body(sound, "SoundDeviceBase")
    expected_fields = (
        "uint32_t field_4_;", "uint32_t volume_8_;", "uint32_t field_C_;",
        "uint32_t field_10_;", "SoundDeviceBase *device_14_;", "uint32_t field_18_;",
        "uint32_t field_1C_;",
    )
    assert "SoundDeviceBase *device_14_;" in base_text
    positions = [base_text.index(field) for field in expected_fields]
    assert positions == sorted(positions)
    for owner, text in (("Midi_Device", sound), ("Wave_In_Device", sound),
                        ("Wave_Device", wave)):
        body = class_body(text, owner)
        for field in expected_fields:
            field_name = field.split()[-1].rstrip(";").lstrip("*")
            declaration = rf"^\s*(?:uint32_t\s+|(?:void|SoundDeviceBase)\s*\*\s*){re.escape(field_name)}\s*;"
            assert not re.search(declaration, body, re.MULTILINE), (
                f"{owner} duplicates shared {field_name}"
            )

    sound_cpp = (ROOT / "src/sounddevice.cpp").read_text()
    wave_cpp = (ROOT / "src/wave_device.cpp").read_text()
    base_ctor = extract_braced(sound_cpp, sound_cpp.index("SoundDeviceBase::SoundDeviceBase()"))
    base_dtor = extract_braced(sound_cpp, sound_cpp.index("SoundDeviceBase::~SoundDeviceBase()"))
    wave_ctor = extract_braced(wave_cpp, wave_cpp.index("Wave_Device::Wave_Device()"))
    wave_dtor = extract_braced(wave_cpp, wave_cpp.index("Wave_Device::~Wave_Device()"))
    for statement in (
        "memset(&field_4_, 0, 4);", "field_C_ = 0;", "field_18_ = 0;",
        "field_1C_ = 0;", "field_10_ = 0;", "device_14_ = nullptr;",
        "volume_8_ = 0x7F;",
    ):
        assert statement in base_ctor
    assert "if (device)" in base_dtor and "vtable_slot<device_release_fn>(device, 0x10)(device);" in base_dtor
    assert "VectorCtorIterator" not in wave_ctor
    assert "field_20_ = 0" in wave_ctor
    assert "VectorDtorIterator" not in wave_dtor
    wave_header = (ROOT / "src/wave_device.h").read_text()
    assert "WaveControlGroup groups_[16]" in wave_header
    assert "WaveControlGroup();" in wave_header
    assert "~WaveControlGroup();" in wave_header
    assert "WaveControlGroup::WaveControlGroup()" in wave_cpp
    assert "WaveControlGroup::~WaveControlGroup()" in wave_cpp
    assert "device_release_fn" not in wave_dtor
    assert "Midi_Device::Midi_Device() {\n}" in sound_cpp
    assert "Wave_In_Device::Wave_In_Device() {\n}" in sound_cpp
    assert "Midi_Device::~Midi_Device() {\n}" in sound_cpp
    assert "Wave_In_Device::~Wave_In_Device() {\n}" in sound_cpp


def test_host_compiler_checks_prototypes_concreteness_and_dispatch(tmp_path):
    compiler = shutil.which("c++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    base, owners = interface_model(*sound_headers())
    errors = type_errors(base, owners)
    assert not errors, "; ".join(errors)
    # The generated harness uses source-extracted signatures but no target
    # size assertions; it checks C++ override/concreteness/dispatch semantics.
    source = tmp_path / "sound_interface_control.cpp"
    source.write_text(generated_cpp(base, owners))
    binary = tmp_path / "sound_interface_control"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)


def test_source_prototype_mutant_is_rejected():
    sound, wave = sound_headers()
    mutant = wave.replace(
        "int select(unsigned long a1);", "int select(unsigned int a1);", 1
    )
    assert mutant != wave
    base, owners = interface_model(sound, mutant)
    errors = type_errors(base, owners)
    assert "Wave_Device prototype mismatch at select" in errors
