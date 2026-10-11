"""Complete pinned CRT bodies; unknown references are not dependency admission."""
from collections import Counter
from dataclasses import replace
import csv
import hashlib
import os
from pathlib import Path
import subprocess

from capstone import Cs, CS_ARCH_X86, CS_MODE_32, CS_GRP_JUMP
from capstone.x86 import X86_OP_IMM
import pytest

from match_recovery.binary import RelocationKind, compare_bound_code
from match_recovery.coff_archive import CoffArchive
from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_text
from test_crt_eight_entry_bindings_oct10 import raw_offset, repin

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "src/vendor/vc6-runtime.recovery.h"
BASE = "54c9d27ec"
EXPECTED = (
    (0x644E21, 0x644EC6, "_doexit", "crt0dat", 12,
     "48be3defdca3560f39e7d593e4dd110cb2977874227eae911a707868f722b961"),
    (0x6461A7, 0x64628F, "__fread_lk", "fread", 2,
     "47c22a066b67d5016169be5db695de59e81571ebdd95c7c5b25846b21fb2099f"),
    (0x646ED0, 0x646F5D, "_calloc", "calloc", 1,
     "955dd6918e98e24016fc53bd1d21a4bf3dae3b73407ce3ec6b16e84f422c5fb9"),
    (0x647067, 0x6470A6, "__endthread", "thread", 5,
     "5cce4c839d1764f275e326b85b388f98e40622a9f2f2030843082c9023b2e596"),
    (0x647815, 0x647851, "_fprintf", "fprintf", 1,
     "c156b413e757a9dc6a21700eb0efbb1fdf0bb13973c9bf1e787dd13b4bd37c6d"),
    (0x64F013, 0x64F09C, "___crtMessageBoxA", "crtmbox", 2,
     "bce736f205a9f942f8dd695a63e9a4100046e168ca6f1f2b84a431b4e64c24c4"),
)
ADDRESSES = {a for a, *_ in EXPECTED}
UNKNOWN = {
    "_doexit": {"__C_Exit_Done", "__C_Termination_Done", "__exitflag",
                "___onexitbegin", "___onexitend", "___xp_z", "___xp_a",
                "___xt_z", "___xt_a", "__imp__GetCurrentProcess@0",
                "__imp__TerminateProcess@8", "__imp__ExitProcess@4"},
    "__fread_lk": {"_memcpy"},
    "_calloc": {"___sbh_threshold", "__crtheap", "__newmode"},
    "__endthread": {"__FPmtterm", "__amsg_exit", "__imp__CloseHandle@4",
                    "__imp__ExitThread@4"},
    "_fprintf": {"__output"},
    "___crtMessageBoxA": {
        "_?pfnMessageBoxA@?1??__crtMessageBoxA@@9@9",
        "_?pfnGetActiveWindow@?1??__crtMessageBoxA@@9@9",
        "_?pfnGetLastActivePopup@?1??__crtMessageBoxA@@9@9",
        "??_C@_0L@HKL@user32?4dll?$AA@", "??_C@_0M@PKCK@MessageBoxA?$AA@",
        "??_C@_0BA@GILI@GetActiveWindow?$AA@",
        "??_C@_0BD@NJFP@GetLastActivePopup?$AA@",
        "__imp__LoadLibraryA@4", "__imp__GetProcAddress@8"},
}


def unit(stem):
    return "build/intel/mt_obj/" + stem + ".obj"


def prior_text():
    return subprocess.check_output(["git", "show", BASE + ":" + SOURCE],
                                   cwd=ROOT, text=True)


def row(report, address):
    return next(r for r in report["functions"] if int(r["address"], 16) == address)


@pytest.fixture(scope="module")
def inputs():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE",
                                     ROOT / ".opensmacx/game/terranx_original.exe"))
    if "OPENSMACX_IMAGE" in os.environ:
        assert image_path.is_file(), "explicit original image is unavailable"
    archive_path = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")
    if not image_path.is_file() or not archive_path.is_file():
        pytest.skip("external pinned original and runtime archive required")
    manifest = parse_component((ROOT / SOURCE).read_text())
    image, archive = image_path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == manifest.bindings[0].origin.image_sha256
    assert hashlib.sha256(archive).hexdigest() == manifest.archive_sha256
    return manifest, image, archive, read_imports(ROOT), image_path


def verify(inputs, manifest=None, image=None):
    m, i, archive, imports, _ = inputs
    return verify_toolchain(m if manifest is None else manifest,
                            i if image is None else image, archive,
                            import_bindings=imports)


def physical(inputs, stem, symbol):
    obj = Coff(CoffArchive(inputs[2]).member(unit(stem)))
    return obj, obj.function(symbol, comdat=False)


def test_only_six_bindings_added_with_all_previous_source_preserved():
    old = prior_text()
    text = (ROOT / SOURCE).read_text()
    before, after = parse_component(old), parse_component(text)
    assert parse_component(render_component(after)) == after
    additions = tuple(b for b in after.bindings if b.origin.address in ADDRESSES)
    assert [(b.origin.address, b.spans, b.symbol, b.unit) for b in additions] == [
        (a, (Span(a, e),), s, unit(u)) for a, e, s, u, *_ in EXPECTED]
    assert replace(after, bindings=tuple(b for b in after.bindings
                   if b.origin.address not in ADDRESSES)) == before
    assert [replace(r, line=1, region="") for r in read_text(text, ROOT / SOURCE)] == [
        replace(r, line=1, region="") for r in read_text(old, ROOT / SOURCE)]
    # Exact original source, including imports and historical annotations, survives.
    lines = [line for line in text.splitlines() if any(
        '"origin":"0x%08X"' % a in line for a in ADDRESSES)]
    assert len(lines) == len(EXPECTED)
    for line in lines:
        text = text.replace(line + "\n", "", 1)
    last = next(line for line in reversed(old.splitlines())
                if '"origin":' in line and '"unit":' in line)
    text = text.replace(last + ",\n", last + "\n", 1)
    assert text == old
    for b in additions:
        assert "final-link retention/allocation" in b.evidence
        assert "runtime equivalence are unproved" in b.evidence
    intervals = sorted((s.start, s.end) for b in after.bindings for s in b.spans)
    assert all(end <= start for (_, end), (start, _) in zip(intervals, intervals[1:]))


def test_full_archive_census_unique_exclusive_functions_and_body_shape(inputs):
    manifest, original, archive, _, _ = inputs
    members = CoffArchive(archive)
    definitions = {s: [] for _, _, s, *_ in EXPECTED}
    # Inventory every physical member: a chosen member alone cannot prove uniqueness.
    for name, candidates in members.members.items():
        for data in candidates:
            try:
                obj = Coff(data)
            except ValueError:
                continue
            for symbol in obj.symbols.values():
                if (symbol.name in definitions and symbol.section > 0
                        and symbol.type & 0x20 and symbol.storage in (2, 3)):
                    definitions[symbol.name].append((name, symbol.section, symbol.value))
    image = PeImage(original, manifest.bindings[0].origin.image_sha256)
    for address, end, name, stem, section, digest in EXPECTED:
        assert definitions[name] == [(unit(stem), section, 0)]
        data = members.member(unit(stem))
        assert hashlib.sha256(data).hexdigest() == digest
        obj = Coff(data)
        function = obj.function(name, comdat=False)
        assert function.section == section and function.value == 0
        assert obj.code_sections(function) == (section,)
        body, relocations, alignment = obj.subject(function)
        assert len(body) == end - address
        span = Span(address, end)
        sites = frozenset(r.offset for r in relocations
                          if r.kind == RelocationKind.ABSOLUTE32)
        assert image.absolute_sites(span) == sites
        proof = compare_bound_code(span, image.read(span), body, relocations, {},
                                   original_absolute_sites=sites, alignment=alignment)
        assert proof["boundary"] == "pass" and proof["alignment_fill"] == 0
        assert not proof["differing_offsets"]
        assert proof["result"] == proof["references"] == "unknown"


def test_independent_catalogues_and_complete_native_epilogues(inputs):
    manifest, original, _, _, image_path = inputs
    ida_path = image_path.parent / "ida94-functions.csv"
    if not ida_path.is_file():
        pytest.skip("external IDA9.4 catalogue required")
    with ida_path.open() as stream:
        ida = {int(r["address"], 16): r for r in csv.DictReader(stream)}
    with (ROOT / "docs/recovery/ghidra-functions.csv").open() as stream:
        ghidra = {int(r["address"], 16): r for r in csv.DictReader(stream)}
    image = PeImage(original, manifest.bindings[0].origin.image_sha256)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    for address, end, name, *_ in EXPECTED:
        r = ghidra[address]
        assert r["program_sha256"] == manifest.bindings[0].origin.image_sha256
        catalogue_end = {0x644E21: 0x644EC4, 0x647067: 0x6470A4}.get(address, end)
        assert int(ida[address]["end"], 16) == catalogue_end
        assert int(r["end_address"], 16) == catalogue_end
        assert r["body_ranges"] == f"0x{address:08X}-0x{catalogue_end:08X}"
        instructions = list(decoder.disasm(image.read(Span(address, end)), address))
        assert sum(i.size for i in instructions) == end - address
        assert instructions[-1].address + instructions[-1].size == end
        by_address = {i.address: i for i in instructions}
        for instruction in instructions:
            if instruction.group(CS_GRP_JUMP) and instruction.operands[0].type == X86_OP_IMM:
                assert instruction.operands[0].imm in by_address
        if address in (0x644E21, 0x647067):
            register, next_symbol = ("edi", "__lockexit") if address == 0x644E21 else ("esi", "_malloc")
            assert [(i.address, i.mnemonic, i.op_str) for i in instructions[-2:]] == [
                (catalogue_end, "pop", register), (end - 1, "ret", "")]
            assert next(b.origin.address for b in manifest.bindings
                        if b.symbol == next_symbol) == end
        elif name == "_fprintf":
            assert instructions[-1].mnemonic == "ret"
        else:
            assert instructions[-1].mnemonic == "jmp"
            assert any(i.mnemonic == "ret" for i in instructions)


def test_incoming_original_calls_correspond_to_physical_archive_relocations(inputs):
    original = PeImage(inputs[1], inputs[0].bindings[0].origin.image_sha256)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    callers = (
        ("crt0dat", "_exit", 4, 0x644DFF, 9, "_doexit", 0x644E21),
        ("fread", "_fread", 1, 0x646178, 25, "__fread_lk", 0x6461A7),
        ("tidtable", "__getptd", 8, 0x6491C3, 33, "_calloc", 0x646ED0),
        ("thread", "__threadstart@4", 3, 0x646FD4, 89, "__endthread", 0x647067),
        ("crt0msg", "__NMSG_WRITE", 23, 0x64BDF9, 288, "___crtMessageBoxA", 0x64F013),
    )
    for stem, symbol, section, start, offset, target, target_address in callers:
        obj, function = physical(inputs, stem, symbol)
        assert function.section == section and function.value == 0
        assert (offset, target, 20) in [(o, obj.symbols[t].name, k)
                                      for o, t, k in obj.sections[section - 1].relocations]
        site = start + offset - 1
        instruction, = decoder.disasm(original.read(Span(site, site + 5)), site)
        assert (instruction.mnemonic, instruction.op_str) == ("call", hex(target_address))
    # The application caller is independently decoded; no candidate address supplies its target.
    for site in (0x626147, 0x626176):
        instruction, = decoder.disasm(original.read(Span(site, site + 5)), site)
        assert (instruction.mnemonic, instruction.op_str) == ("call", "0x647815")


def test_strict_results_leave_each_unproved_reference_unknown(inputs):
    report = verify(inputs)
    for address, end, symbol, _, section, _ in EXPECTED:
        result = row(report, address)
        assert result["boundary"] == "pass" and result["alignment_fill"] == 0
        assert result["span"] == {"start": address, "end": end}
        assert result["section"] == section and not result["differing_offsets"]
        assert result["result"] == result["bytes"] == result["references"] == "unknown"
        assert {r["symbol"] for r in result["relocations"] if r["result"] == "unknown"} == UNKNOWN[symbol]
        assert all(r["result"] in ("pass", "unknown") for r in result["relocations"])
        assert not result["missing_absolute_relocations"]
        assert not result["extra_absolute_relocations"]
    assert report["result"] == "unknown"
    assert report["application_recoveries"] == 0 and not report["ready_for_collection"]


def test_previous_results_only_improve_at_independently_bound_incoming_targets(inputs):
    before, after = verify(inputs, parse_component(prior_text())), verify(inputs)
    new_symbols = {s for _, _, s, *_ in EXPECTED}
    changes = set()
    for old, current in zip(before["functions"], after["functions"]):
        assert old.keys() == current.keys()
        if old == current:
            continue
        changes.add(old["symbol"])
        assert current["boundary"] == old["boundary"]
        assert current["differing_offsets"] == old["differing_offsets"]
        assert current["result"] != "fail"
        if old["result"] == "pass":
            assert current["result"] == "pass"
        assert old["bytes"] == current["bytes"] or (old["bytes"], current["bytes"]) == ("unknown", "pass")
        assert old["references"] == current["references"] or (old["references"], current["references"]) == ("unknown", "pass")
        for key in old.keys() - {"result", "bytes", "references", "diagnostic", "relocations"}:
            assert old[key] == current[key]
        assert {k: v for k, v in old["diagnostic"].items() if k != "emitted_references"} == {
            k: v for k, v in current["diagnostic"].items() if k != "emitted_references"}
        for x, y in zip(old["relocations"], current["relocations"], strict=True):
            if x != y:
                assert x["symbol"] in new_symbols
                assert x["result"] == "unknown" and y["result"] == "pass"
                assert {k: v for k, v in x.items() if k != "result"} == {k: v for k, v in y.items() if k != "result"}
        for x, y in zip(old["diagnostic"]["emitted_references"], current["diagnostic"]["emitted_references"], strict=True):
            if x != y:
                assert x["symbol"] in new_symbols and x["binding_address"] is None
                assert y["binding_address"] is not None
                assert {k: v for k, v in x.items() if k != "binding_address"} == {k: v for k, v in y.items() if k != "binding_address"}
    assert changes == {"_exit", "__exit", "_fread", "__getptd", "__NMSG_WRITE",
                       "___initstdio", "__beginthread", "__mtinit"}
    assert row(after, 0x646178)["result"] == "pass"
    assert Counter(r["result"] for r in before["functions"]) == {"pass": 110, "unknown": 138}
    assert Counter(r["result"] for r in after["functions"]) == {"pass": 113, "unknown": 141}


@pytest.mark.parametrize("address,end,symbol,stem,section,digest", EXPECTED)
@pytest.mark.parametrize("damage", ("body", "extent", "symbol"))
def test_body_extent_and_symbol_damage_refused(inputs, address, end, symbol, stem, section, digest, damage):
    manifest, image, *_ = inputs
    if damage == "body":
        changed = bytearray(image)
        changed[raw_offset(image, address)] ^= 1
        image = bytes(changed)
        manifest = repin(manifest, image)
    else:
        manifest = replace(manifest, bindings=tuple(
            (replace(b, spans=(Span(address, end - 1),)) if damage == "extent"
             else replace(b, symbol=symbol + "_missing"))
            if b.origin.address == address else b for b in manifest.bindings))
    result = row(verify(inputs, manifest, image), address)
    assert result["result"] == ("unknown" if damage == "symbol" else "fail")


@pytest.mark.parametrize("address,offset", ((0x644E21, 2), (0x6461A7, 145),
                         (0x646ED0, 50), (0x647067, 13), (0x647815, 9)))
def test_known_relative_reference_damage_fails(inputs, address, offset):
    manifest, image, *_ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, address + offset)] ^= 1
    changed = bytes(changed)
    result = row(verify(inputs, repin(manifest, changed), changed), address)
    assert result["references"] == result["result"] == "fail"


def test_unknown_memcpy_target_damage_stays_unknown(inputs):
    manifest, image, *_ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, 0x6461A7 + 95)] ^= 1
    changed = bytes(changed)
    result = row(verify(inputs, repin(manifest, changed), changed), 0x6461A7)
    assert result["references"] == result["result"] == "unknown"
    assert next(r["result"] for r in result["relocations"] if r["symbol"] == "_memcpy") == "unknown"


@pytest.mark.parametrize("address,end,symbol,stem,section,digest",
                         [r for r in EXPECTED if r[2] not in ("__fread_lk", "_fprintf")])
def test_omitted_physical_absolute_relocation_detected(inputs, address, end, symbol, stem, section, digest):
    obj, function = physical(inputs, stem, symbol)
    body, relocations, alignment = obj.subject(function)
    original = PeImage(inputs[1], inputs[0].bindings[0].origin.image_sha256)
    removed = next(r for r in relocations if r.kind == RelocationKind.ABSOLUTE32)
    span = Span(address, end)
    proof = compare_bound_code(span, original.read(span), body,
                               tuple(r for r in relocations if r != removed), {},
                               original_absolute_sites=original.absolute_sites(span),
                               alignment=alignment)
    assert proof["boundary"] == "pass"
    assert proof["missing_absolute_relocations"] == [removed.offset]
    assert proof["references"] == proof["result"] == "fail"


@pytest.mark.parametrize("address,end", ((0x644E21, 0x644EC6), (0x647067, 0x6470A6)))
def test_catalogue_truncation_and_terminal_epilogue_damage_refused(inputs, address, end):
    manifest, image, *_ = inputs
    truncated = replace(manifest, bindings=tuple(
        replace(b, spans=(Span(address, end - 2),)) if b.origin.address == address else b
        for b in manifest.bindings))
    result = row(verify(inputs, truncated), address)
    assert result["boundary"] == result["result"] == "fail"
    for site in (end - 2, end - 1):
        changed = bytearray(image)
        changed[raw_offset(image, site)] ^= 1
        changed = bytes(changed)
        result = row(verify(inputs, repin(manifest, changed), changed), address)
        assert result["result"] == "fail"
        assert site - address in result["differing_offsets"]


@pytest.mark.parametrize("symbol,stem", (("_strcat", "strcat"), ("__alloca_probe", "chkstk"),
                         ("_strchr", "strchr"), ("__strcmpi", "stricmp"), ("__CIacos", "acos")))
def test_multi_entry_sections_are_not_promoted_to_functions(inputs, symbol, stem):
    assert symbol not in {b.symbol for b in inputs[0].bindings}
    obj = Coff(CoffArchive(inputs[2]).member(next(
        u for u in CoffArchive(inputs[2]).members if u.endswith("/" + stem + ".obj"))))
    with pytest.raises(ValueError, match="exclusively"):
        obj.function(symbol, comdat=False)


@pytest.mark.parametrize("symbol,provider_count", (("__amsg_exit", 5), ("_fast_error_exit", 4)))
def test_multiple_provider_error_entries_remain_unbound(inputs, symbol, provider_count):
    assert symbol not in {b.symbol for b in inputs[0].bindings}
    providers = []
    for name, members in CoffArchive(inputs[2]).members.items():
        for data in members:
            try:
                obj = Coff(data)
            except ValueError:
                continue
            if any(s.name == symbol and s.section > 0 and s.type & 0x20
                   for s in obj.symbols.values()):
                providers.append(name)
    assert len(providers) == provider_count
