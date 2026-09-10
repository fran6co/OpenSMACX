"""Private retention controls with portable synthetic COFF, never a toolchain."""

from dataclasses import replace
import re
import struct

import pytest

from match_recovery.bound_objects import FunctionBinding
from match_recovery.function_witness import _escape, retained_definition
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff


def object_bytes(functions, undefined=()):
    """Each (name, bytes, relocations) owns one exclusive external COMDAT."""
    indices = {name: index * 3 + 2 for index, (name, _, _) in enumerate(functions)}
    indices.update({name: len(functions) * 3 + index for index, name in enumerate(undefined)})
    strings, symbols, headers, data = bytearray(), bytearray(), [], bytearray()

    def name_bytes(name):
        raw = name.encode("ascii")
        if len(raw) <= 8:
            return raw.ljust(8, b"\0")
        offset = len(strings) + 4
        strings.extend(raw + b"\0")
        return struct.pack("<II", 0, offset)

    start = 20 + len(functions) * 40
    for number, (name, code, relocations) in enumerate(functions, 1):
        raw_at = start + len(data)
        data.extend(code)
        reloc_at = start + len(data) if relocations else 0
        for offset, target, kind in relocations:
            data.extend(struct.pack("<IIH", offset, indices[target], kind))
        headers.append(b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code), raw_at,
                       reloc_at, 0, len(relocations), 0, 0x60501020))
        symbols.extend(b".text\0\0\0" + struct.pack("<IhHBB", 0, number, 0, 3, 1))
        symbols.extend(struct.pack("<IHHIHB3x", len(code), len(relocations), 0, 0, 0, 2))
        symbols.extend(name_bytes(name) + struct.pack("<IhHBB", 0, number, 0x20, 2, 0))
    for name in undefined:
        symbols.extend(name_bytes(name) + struct.pack("<IhHBB", 0, 0, 0x20, 2, 0))
    return (struct.pack("<HHIIIHH", 0x14C, len(functions), 0, start + len(data),
                        len(symbols) // 18, 0, 0) + b"".join(headers) + data + symbols
            + struct.pack("<I", len(strings) + 4) + strings)


BASE = object_bytes([("_other", b"\xc3", ())])


class Compiler:
    def __init__(self, root, symbol="sample", fault=None):
        self.root, self.symbol, self.fault = root, symbol, fault
        self.calls = []

    def overlay(self, source, replacements, command, flags):
        self.calls.append((source, replacements.copy(), command, flags))
        unit = source.relative_to(self.root).as_posix()
        body = replacements[unit]
        match = re.search(rb'extern "C" int __cdecl (\w+)\(\)', body)
        if self.fault == "compile-error":
            raise ValueError("compiler refused")
        if self.fault == "malformed":
            return b"not COFF"
        if not match:
            return object_bytes([("_other", b"\xcc", ())]) if self.fault == "replica" else BASE
        if self.fault == "ignore-overlay":
            return BASE
        header = next(value for key, value in replacements.items() if key != unit)
        if self.fault == "mutate-source":
            (self.root / "code/model.h").write_bytes(b"user edit\n")
        probe = "_" + match[1].decode()
        sink = "_" + re.search(rb"template<class T> void (\w+)\(T\);", body)[1].decode()
        target = self.symbol if self.fault != "wrong-target" else "_wrong"
        value = 1
        if b"for (;;)" in header and self.fault != "ignore-target-damage":
            value = 9
        probe_value = int(b"); return 1;" in body)
        if self.fault == "ignore-probe-damage":
            probe_value = 0
        if self.fault == "target-sensitive-to-probe":
            value += probe_value
        if self.fault == "unstable" and len(self.calls) == 3:
            value = 4
        target_code = b"\xb8" + struct.pack("<I", value) + b"\xc3"
        probe_code = b"\x68" + bytes(4) + b"\xe8" + bytes(4) + b"\xb8" + struct.pack("<I", probe_value) + b"\xc3"
        relocs = [(1, target, 6), (6, sink, 20)]
        if self.fault == "missing-target-reference":
            relocs = relocs[1:]
        if self.fault == "missing-sink-reference":
            relocs = relocs[:1]
        functions = [("_other", b"\xc3", ()), (target, target_code, ()),
                     (probe, probe_code, relocs)]
        return object_bytes(functions, [sink])


@pytest.fixture
def inputs(tmp_path):
    (tmp_path / "code").mkdir()
    (tmp_path / "code/owner.cc").write_bytes(b'#include "model.h"\n')
    (tmp_path / "code/model.h").write_bytes(b"inline int sample() { return 1; }\n")
    binding = FunctionBinding(Origin("a" * 64, "pe:va", 0x401000),
                              (Span(0x401000, 0x401006),), "sample", "code/owner.cc")
    return tmp_path, binding


def run(inputs, compiler=None, **kwargs):
    root, binding = inputs
    arguments = dict(root=root, binding=binding, body_path="code/model.h", baseline=Coff(BASE),
                     compiler=compiler or Compiler(root, binding.symbol), command=("compiler",),
                     flags=("/O2",), symbol_format="none")
    arguments.update(kwargs)
    return retained_definition(**arguments)


def test_controlled_retention_is_transient_and_diagnostic(inputs):
    root, binding = inputs
    original = {path: path.read_bytes() for path in (root / "code").iterdir()}
    compiler = Compiler(root)
    obj, report = run(inputs, compiler)
    assert isinstance(obj, Coff) and obj.function(binding.symbol)
    assert report["result"] == "pass" and report["compile_attempts"] == 5
    assert report["product_emission"] == "not_emitted" and report["product_emitted"] is False
    assert all(report[key] == "pass" for key in ("replica_control", "repeat_control", "target_body_control", "probe_control"))
    assert report["diagnostic_only"] and report["never_link"]
    assert not report["persisted"] and not report["ready_for_collection"]
    assert all(path.read_bytes() == raw for path, raw in original.items())
    assert all(call[2:] == (("compiler",), ("/O2",)) for call in compiler.calls)
    probe = compiler.calls[1][1][binding.unit]
    assert probe.startswith(original[root / binding.unit])
    assert b"dllexport" not in probe and b"#ifdef sample" in probe and b"(&sample)" in probe


@pytest.mark.parametrize("symbol,source,expression", [
    ("?value@Widget@@QAEHXZ", b"class Widget { public: int value() { return 1; } };", "Widget::value"),
    ("?value@Widget@@SAHXZ", b"class Widget { public: static int value() { return 1; } };", "Widget::value"),
    ("?sample@@YAHXZ", b"inline int sample() { return 1; }", "sample"),
])
def test_supported_msvc_ordinary_functions(inputs, symbol, source, expression):
    root, binding = inputs
    (root / "code/model.h").write_bytes(source)
    binding = replace(binding, symbol=symbol)
    obj, report = run((root, binding), symbol_format="msvc")
    assert obj is not None and report["address_expression"] == expression


@pytest.mark.parametrize("fault,attempts", [
    ("compile-error", 1), ("malformed", 1), ("replica", 1), ("ignore-overlay", 2),
    ("wrong-target", 2), ("missing-target-reference", 2), ("missing-sink-reference", 2),
    ("unstable", 3), ("ignore-target-damage", 4), ("ignore-probe-damage", 5),
    ("target-sensitive-to-probe", 5), ("mutate-source", 2),
])
def test_failed_controls_never_return_a_probe_object(inputs, fault, attempts):
    obj, report = run(inputs, Compiler(inputs[0], fault=fault))
    assert obj is None and report["result"] == "unknown" and report["reason"]
    assert report["compile_attempts"] == attempts and report["product_emitted"] is False
    if fault == "mutate-source":
        assert (inputs[0] / "code/model.h").read_bytes() == b"user edit\n"


@pytest.mark.parametrize("symbol", ["?value@Widget@@AAEHXZ", "?value@Widget@@UAEHXZ",
                                   "??0Widget@@QAE@XZ", "??HWidget@@QAEHH@Z"])
def test_nonpublic_virtual_and_generated_members_refused_before_compile(inputs, symbol):
    root, binding = inputs
    obj, report = run((root, replace(binding, symbol=symbol)), symbol_format="msvc")
    assert obj is None and report["compile_attempts"] == 0


@pytest.mark.parametrize("contents", [
    b"inline int sample(int x) { return x; } inline int sample() { return 1; }",
    b"inline int sample();", b"inline int sample() {\n#ifdef X\nreturn 1;\n#endif\n}",
    b"inline int sample() { return 1; } /* unterminated",
])
def test_unsupported_or_ambiguous_source_refused(inputs, contents):
    (inputs[0] / "code/model.h").write_bytes(contents)
    obj, report = run(inputs)
    assert obj is None and report["compile_attempts"] == 0


@pytest.mark.parametrize("kind", ["defined", "weak", "common", "absolute", "unsupported-type", "duplicate"])
def test_only_established_nonemission_is_eligible(inputs, kind):
    if kind == "defined":
        baseline = Coff(object_bytes([("sample", b"\xc3", ())]))
    else:
        baseline = Coff(object_bytes([("_other", b"\xc3", ())], ["sample"]))
        entry = next(s for s in baseline.symbols.values() if s.name == "sample")
        changes = {"weak": {"storage": 105}, "common": {"value": 4}, "absolute": {"section": -1},
                   "unsupported-type": {"type": 4}, "duplicate": {}}[kind]
        baseline.symbols[entry.index] = replace(entry, **changes)
        if kind == "duplicate":
            baseline.symbols[entry.index + 1] = replace(entry, index=entry.index + 1)
    obj, report = run(inputs, baseline=baseline)
    assert obj is None and report["compile_attempts"] == 0
    assert report["product_emitted"] is None and "product_emission" not in report


def test_ordinary_undefined_external_is_an_absent_body(inputs):
    baseline = Coff(object_bytes([("_other", b"\xc3", ())], ["sample"]))
    # A replica lacking that undefined reference must still be refused.
    obj, report = run(inputs, baseline=baseline)
    assert obj is None and report["product_emitted"] is False
    assert report["compile_attempts"] == 1 and "replica differs" in report["reason"]


@pytest.mark.parametrize("path", ["code/../outside.h", "../outside.h", "/outside.h", "code/missing.h", "code/owner.cc"])
def test_invalid_explicit_header_refused(inputs, path):
    obj, report = run(inputs, body_path=path)
    assert obj is None and report["compile_attempts"] == 0


def test_symlink_header_refused(inputs):
    root, _ = inputs
    (root / "code/link.h").symlink_to(root / "code/model.h")
    obj, report = run(inputs, body_path="code/link.h")
    assert obj is None and report["compile_attempts"] == 0


def test_configured_unusual_source_suffixes_are_not_guessed(inputs):
    root, binding = inputs
    (root / "code/owner.cc").rename(root / "code/owner.C")
    (root / "code/model.h").rename(root / "code/model.ipp")
    obj, report = run((root, replace(binding, unit="code/owner.C")), body_path="code/model.ipp")
    assert obj is not None and report["result"] == "pass"


@pytest.mark.parametrize("flags", [("/Yuheader",), ("/Ycheader",), ("/Fpstate",), ("@arguments",), ("",), (5,)])
def test_standalone_tokenized_recipe_required(inputs, flags):
    obj, report = run(inputs, flags=flags)
    assert obj is None and report["compile_attempts"] == 0


def test_missing_overlay_support_refused(inputs):
    obj, report = run(inputs, compiler=object())
    assert obj is None and report["compile_attempts"] == 0


def test_unsupported_symbol_format_refused(inputs):
    obj, report = run(inputs, symbol_format="guess")
    assert obj is None and report["compile_attempts"] == 0


def test_probe_cannot_borrow_address_evidence_from_same_named_data():
    obj = Coff(object_bytes([("target", b"\xc3", ()),
                             ("probe", bytes(8), ((0, "target", 6), (4, "sink", 20)))], ["sink"]))
    target = obj.function("target")
    duplicate = max(obj.symbols) + 1
    obj.symbols[duplicate] = replace(target, index=duplicate, type=0)
    section = obj.sections[1]
    obj.sections = (obj.sections[0], replace(section, relocations=((0, duplicate, 6), (4, 6, 20))))
    with pytest.raises(ValueError, match="target|exact|ambiguous"):
        _escape(obj, "probe", "target", "sink")


@pytest.mark.parametrize("target_kind,sink_kind", [(0, 20), (6, 0), (20, 20), (6, 6)])
def test_probe_requires_real_address_and_call_relocations(target_kind, sink_kind):
    obj = Coff(object_bytes([("target", b"\xc3", ()),
                             ("probe", bytes(8), ((0, "target", target_kind), (4, "sink", sink_kind)))], ["sink"]))
    with pytest.raises(ValueError, match="target|sink|address|relocation"):
        _escape(obj, "probe", "target", "sink")


@pytest.mark.parametrize("sink_name", ["?sink@@YAXP8Widget@@AEPADXZ@Z", "?sink@@YAXP6AHXZ@Z"])
def test_vc6_template_sink_has_bounded_ordinary_free_function_spelling(sink_name):
    obj = Coff(object_bytes([("target", b"\xc3", ()),
                             ("probe", bytes(8), ((0, "target", 6), (4, sink_name, 20)))], [sink_name]))
    _escape(obj, "probe", "target", "sink")


@pytest.mark.parametrize("sink_name", [
    "?sink_extra@@YAXP8Widget@@AEPADXZ@Z", "?sink@Namespace@@YAXP8Widget@@AEPADXZ@Z",
    "?sink@@QAXP8Widget@@AEPADXZ@Z", "?sink@@YAXH@Z", "?sink@@YAXP8Widget@@AEPADXZ",
    "?sink@@YAXP8Widget@@AEPADXZ@Zjunk",
])
def test_ordinary_sink_name_does_not_admit_similar_scopes_or_unsupported_suffixes(sink_name):
    obj = Coff(object_bytes([("target", b"\xc3", ()),
                             ("probe", bytes(8), ((0, "target", 6), (4, sink_name, 20)))], [sink_name]))
    with pytest.raises(ValueError, match="sink"):
        _escape(obj, "probe", "target", "sink")


def test_ordinary_sink_still_requires_call_relocation():
    sink = "?sink@@YAXP8Widget@@AEPADXZ@Z"
    obj = Coff(object_bytes([("target", b"\xc3", ()),
                             ("probe", bytes(8), ((0, "target", 6), (4, sink, 0)))], [sink]))
    with pytest.raises(ValueError, match="sink"):
        _escape(obj, "probe", "target", "sink")


@pytest.mark.parametrize("target_offset,sink_offset", [(5, 4), (0, 5), (100, 4), (0, 100)])
def test_relocations_outside_probe_code_cannot_prove_address_escape(target_offset, sink_offset):
    obj = Coff(object_bytes([("target", b"\xc3", ()),
                             ("probe", bytes(8), ((target_offset, "target", 6), (sink_offset, "sink", 20)))], ["sink"]))
    with pytest.raises(ValueError, match="target|sink|relocation"):
        _escape(obj, "probe", "target", "sink")


def test_ambiguous_sink_instances_refused():
    first, second = "?sink@@YAXP8Widget@@AEPADXZ@Z", "?sink@@YAXP6AHXZ@Z"
    obj = Coff(object_bytes([("target", b"\xc3", ()),
                             ("probe", bytes(8), ((0, "target", 6), (4, first, 20)))], [first, second]))
    with pytest.raises(ValueError, match="sink"):
        _escape(obj, "probe", "target", "sink")


@pytest.mark.parametrize("code,relocations", [
    (b"\x01" + bytes(7), ((0, "target", 6), (4, "sink", 20))),
    (bytes(4) + b"\x01" + bytes(3), ((0, "target", 6), (4, "sink", 20))),
    (bytes(8), ((0, "target", 6), (0, "sink", 20))),
    (bytes(8), ((0, "target", 6), (2, "sink", 20))),
])
def test_nonzero_addends_and_overlapping_relocations_refused(code, relocations):
    obj = Coff(object_bytes([("target", b"\xc3", ()), ("probe", code, relocations)], ["sink"]))
    with pytest.raises(ValueError, match="relocation"):
        _escape(obj, "probe", "target", "sink")
