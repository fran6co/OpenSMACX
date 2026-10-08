"""Observed Wave-In extension calls, without a fabricated provider class."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from match_recovery.source.model import SourceRole

ROOT = Path(__file__).resolve().parents[2]
ADDRESSES = (0x004C5A80, 0x004C5AA0)


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth, end = 1, opening + 1
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    return text[start:end]


def test_extension_records_keep_partial_external_contract_obligations():
    source = (ROOT / "src/sounddevice.cpp").read_text()
    records = {r.address: r for r in read_file(ROOT / "src/sounddevice.cpp")}
    header = (ROOT / "src/sounddevice.h").read_text()
    assert "WaveInDeviceDispatchView" not in header
    assert "ordinal_placeholder_" not in header
    assert "void *device_14_;" in header
    for address in ADDRESSES:
        record = records[address]
        assert record.source_role is SourceRole.UNCLASSIFIED
        assert "Wave_In_Device" in record.symbol
        marker = source.index(f"// ORIGINAL: 0x{address:08X}")
        binding = source[marker:source.index("// symbol", marker)]
        assert "SDK/PDB" in binding and "nested object lifetime remain unknown" in binding
        assert record.image_spans == ((address, address + 32),)
        # Coverage classification neither grants nor forbids measured claims.
        assert not record.semantic


def test_external_boundary_has_only_observed_operations_not_prefix_methods():
    abi = (ROOT / "src/soundx_device_abi.h").read_text()
    for expression in ("vtable_method<InitMethod>(device, 0x0C)",
                       "vtable_slot<ReleaseMethod>(device, 0x10)",
                       "vtable_method<CodecMethod>(device, 0x60)",
                       "vtable_method<KeyMethod>(device, 0x64)"):
        assert expression in abi
    assert "virtual " not in abi and "ordinal_placeholder" not in abi
    assert "Partial external SoundX ABI" in abi


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_extension_bodies_and_behavior_damage(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/sounddevice.cpp").read_text()
    codec = definition(source, "int Wave_In_Device::set_codec(unsigned long codec)")
    key = definition(source, "int Wave_In_Device::set_vxw_key(signed char *key)")
    prefix = r"""
#include <cassert>
struct FakeProvider { unsigned long codec = 0; signed char *key = nullptr; };
namespace SoundXDeviceAbi {
// Host adapter stubs, not evidence for the target member-pointer encoding.
int set_codec(void *device, unsigned long codec) {
    if (!device) return 91;
    static_cast<FakeProvider *>(device)->codec = codec; return 47;
}
int set_vxw_key(void *device, signed char *key) {
    if (!device) return 92;
    static_cast<FakeProvider *>(device)->key = key; return -9;
}
}
struct Wave_In_Device {
    void *device_14_ = nullptr;
    int set_codec(unsigned long);
    int set_vxw_key(signed char *);
};
"""
    main = r"""
int main() {
    Wave_In_Device owner;
    signed char key[] = {-128, 2, 0};
    assert(owner.set_codec(5) == 0x13);
    assert(owner.set_vxw_key(key) == 0x13);
    FakeProvider provider;
    owner.device_14_ = &provider;
    assert(owner.set_codec(0xffffffffUL) == 47 && provider.codec == 0xffffffffUL);
    assert(owner.set_vxw_key(key) == -9 && provider.key == key);
}
"""
    candidates = [
        (codec, key),
        (codec.replace("return 0x13;", "return 0x14;", 1), key),
        (codec.replace("set_codec(delegate, codec)", "set_codec(delegate, codec + 1)", 1), key),
        (codec, key.replace("set_vxw_key(delegate, key)", "set_vxw_key(delegate, key + 1)", 1)),
    ]
    for index, (codec_body, key_body) in enumerate(candidates):
        path = tmp_path / f"extension-{index}.cpp"
        path.write_text(prefix + codec_body + "\n" + key_body + main)
        binary = tmp_path / f"extension-{index}"
        compiled = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                                   "-Werror", str(path), "-o", str(binary)],
                                  capture_output=True, text=True)
        assert compiled.returncode == 0, compiled.stderr
        result = subprocess.run([str(binary)], capture_output=True, timeout=10)
        assert (result.returncode == 0) == (index == 0)
