"""v2 EH cleanup fragments: source relation and parent-derived measurement."""

import hashlib
import struct
import sys
from dataclasses import replace
from pathlib import Path

import pytest

ROOT_DIR = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT_DIR / "packages/match-recovery/tests"))
sys.path.insert(0, str(ROOT_DIR / "packages/match-recovery/tests/source"))

from match_recovery.application_binary import (select_bindings,  # noqa: E402
                                               select_fragment_bindings)
from match_recovery.match_check import check_matches  # noqa: E402
from match_recovery.partition import Candidate  # noqa: E402
from match_recovery.model import Origin, Span  # noqa: E402
from match_recovery.source import read_text, writer  # noqa: E402
from match_recovery.source.asm import (AsmComparison, ResolvedEHFragmentGroup,  # noqa: E402
                                       compare_unit, resolve_fragment_pairs)
from match_recovery.source.model import (FRAGMENT_EH_CLEANUP, Tier,  # noqa: E402
                                         fragment_fact)
from match_recovery.source.recording import record_claims  # noqa: E402
from test_claim_integrity import configuration  # noqa: E402
from test_eh_regions import ROOT as EH_SYMBOL, _coff, _pe  # noqa: E402
from test_match_check import database as configure_database  # noqa: E402
from test_recording import compiler as configure_compiler, project as match_base_project  # noqa: E402,F401


PARENT = 0x00401000
PRIMARY = (0x00401000, 0x00401016)
# The two-action fixture: actions at +0 (8 bytes) and +8 (14 bytes), then the
# 10-byte handler stub at +22, as `_pe(cold_size=32)` lays them out.
ACTIONS = ((0x00401100, 0x00401108), (0x00401108, 0x00401116))
STUB = (0x00401116, 0x00401120)
UNIT = Path("src/owner.cpp")


def directive(*, symbol=EH_SYMBOL, address="0x00401000", kind="eh_cleanup", version="v2"):
    kind_field = f'"kind":"{kind}",' if kind is not None else ""
    return (f'// RECOVERY_FRAGMENT: {version} {{"evidence":["pinned:funcinfo-unwind-row"],'
            f'{kind_field}"parent_address":"{address}","parent_symbol":"{symbol}",'
            '"reason":"generated EH unwind action of the parent"}\n')


def child(address, span, *, binding=True, relation=None):
    text = f"// ORIGINAL: 0x{address:08X} sub_{address:x} 0x{span[0]:08X}-0x{span[1]:08X} FILE\n"
    if binding:
        text += ('// RECOVERY_BINDING: v1 {"evidence": ["pinned:unwind row"], '
                 '"reason": "generated cleanup of a real local"}\n')
    text += relation if relation is not None else directive()
    return text + "// kind      game\n"


def parent(spans=(PRIMARY, STUB), *, symbol=EH_SYMBOL, address=PARENT, mode="FILE"):
    rendered = ";".join(f"0x{low:08X}-0x{high:08X}" for low, high in spans)
    keyword = f" {mode}" if mode else ""
    return (f"// ORIGINAL: 0x{address:08X} owner {rendered}{keyword}\n"
            f"// symbol {symbol}\nvoid owner() {{}}\n")


def source(*, children=ACTIONS, parent_text=None):
    # Children precede their parent, as the catalogue left them.
    text = "".join(child(low, (low, high), binding=index == 0)
                   for index, (low, high) in enumerate(children))
    return text + (parent(spans=(PRIMARY, (children[-1][1], STUB[1])))
                   if parent_text is None else parent_text)


def test_reader_binds_children_to_a_later_two_span_parent_after_their_binding():
    rows = read_text(source(), UNIT)
    first, second, owner = rows
    assert owner.function_fragment is None
    for row in (first, second):
        assert row.function_fragment.kind == FRAGMENT_EH_CLEANUP
        assert row.function_fragment.parent_address == PARENT
        assert row.function_fragment.parent_symbol == EH_SYMBOL
        assert row.kind == "game"
    assert fragment_fact(first.function_fragment)["kind"] == FRAGMENT_EH_CLEANUP


@pytest.mark.parametrize(("text", "match"), [
    # A gap between the two actions.
    (child(0x00401100, (0x00401100, 0x00401107)) + child(0x00401108, ACTIONS[1])
     + parent(), "tile"),
    # The last action does not reach the parent's cold span.
    (child(0x00401100, ACTIONS[0]) + parent(), "end at the parent's cold span"),
    (source(parent_text=parent(symbol="?other@@YAXXZ")), "symbol"),
    (source(parent_text=parent(spans=(PRIMARY,))), "primary and one later cold"),
    (source(parent_text=parent() + parent(symbol="?twin@@YAXXZ")), "one non-fragment"),
    (source(parent_text=""), "one non-fragment"),
    (source(parent_text=parent(mode="EXCLUDED S1")), "emitted body"),
    (child(0x00401100, ACTIONS[0], relation=directive(kind="terminal")) + parent(), "eh_cleanup"),
    (child(0x00401100, ACTIONS[0], relation=directive(kind=None)) + parent(), "unexpected fields"),
    # v1 keeps its rule: the directive must immediately follow the marker.
    (child(0x00401100, ACTIONS[0], relation=directive(kind=None, version="v1")) + parent(),
     "immediately follow"),
])
def test_reader_refuses_unproven_eh_cleanup_groups(text, match):
    with pytest.raises(ValueError, match=match):
        read_text(text, UNIT)


def test_reader_refuses_different_compile_context_metadata():
    text = source().replace("// kind      game\n// ORIGINAL: 0x00401108",
                            "// compile_context src/other.cpp\n// ORIGINAL: 0x00401108")
    with pytest.raises(ValueError, match="compile_context"):
        read_text(text, UNIT)


def test_writer_roundtrips_and_keeps_the_directive_below_the_binding():
    text = source()
    rows = read_text(text, UNIT)
    assert writer.write_bytes(text.encode(), rows) == text.encode()
    rows[0].function_fragment = replace(rows[0].function_fragment, reason="respelled reason")
    rewritten = writer.write(text, rows).splitlines()
    marker = rewritten.index(next(line for line in rewritten if "ORIGINAL: 0x00401100" in line))
    assert rewritten[marker + 1].startswith("// RECOVERY_BINDING: v1 ")
    assert rewritten[marker + 2].startswith("// RECOVERY_FRAGMENT: v2 ")
    assert read_text("\n".join(rewritten) + "\n", UNIT)[0].function_fragment.reason == "respelled reason"


def test_writer_removal_takes_binding_and_directive_and_protects_the_parent():
    text = source()
    rows = read_text(text, UNIT)
    with pytest.raises(ValueError, match="fragment parent"):
        writer.remove(text, [rows[2]])
    removed = writer.remove(text, [rows[0]])
    assert "RECOVERY_BINDING" not in removed
    assert removed.count("RECOVERY_FRAGMENT") == 1
    assert "0x00401100 sub_" not in removed


def test_partition_accepts_only_the_eh_cleanup_kind():
    rows = read_text(source(), UNIT)
    fact = fragment_fact(rows[0].function_fragment)
    origin = Origin("0" * 64, "pe:va", ACTIONS[0][0])
    Candidate(origin, (Span(*ACTIONS[0]),), UNIT.as_posix(), 1, False, "sub", function_fragment=fact)
    with pytest.raises(ValueError, match="function-fragment"):
        Candidate(origin, (Span(*ACTIONS[0]),), UNIT.as_posix(), 1, False, "sub",
                  function_fragment={**fact, "kind": "terminal"})


def key(row):
    return row.path.resolve(), row.address, row.line


def test_resolver_returns_one_ordered_group_for_any_selected_member():
    rows = read_text(source(), UNIT)
    groups, refused = resolve_fragment_pairs(rows, [rows[1]])
    assert not refused
    group, = groups
    assert isinstance(group, ResolvedEHFragmentGroup)
    assert group.parent == rows[2] and group.children == (rows[0], rows[1])
    assert group.measured_parent().image_spans == (PRIMARY, (ACTIONS[0][0], STUB[1]))
    assert rows[2].image_spans == (PRIMARY, STUB)          # source span unchanged


@pytest.mark.parametrize("damage", ["seed", "component", "duplicate"])
def test_resolver_refuses_competing_or_unsupported_parents(damage, tmp_path):
    rows = read_text(source(), UNIT)
    records = list(rows)
    seed = component = frozenset()
    if damage == "seed":
        seed = {key(rows[2])}
    elif damage == "component":
        component = frozenset({key(rows[2])})
    else:
        records += read_text(parent(), tmp_path / "other.cpp")
    groups, refused = resolve_fragment_pairs(
        records, [rows[0]], component_keys=component, is_seed=lambda row: key(row) in seed)
    assert not groups and key(rows[0]) in refused


def measured(tmp_path, selection, *, compiled=None, groups=None):
    path = tmp_path / "owner.cpp"
    path.write_text(source())
    image = tmp_path / "original.exe"
    image.write_bytes(_pe(cold_size=32))
    rows = read_text(path.read_text(), path)
    selected = [rows[index] for index in selection]
    if groups == "forged":
        groups = (ResolvedEHFragmentGroup(rows[2], (rows[0], rows[1]), object()),)
    elif groups is None:
        groups, refused = resolve_fragment_pairs(rows, selected)
        assert not refused
    calls = []

    def compiler(*args):
        calls.append(args)
        return compiled if compiled is not None else _coff(cold_size=32)

    unit = compare_unit(selected, image, path, [], ("trial",), compiler=compiler,
                        fragment_pairs=groups)
    return {row.address: result for row, result in unit.results}, len(calls)


def change_code(raw, section, offset, value):
    changed = bytearray(raw)
    pointer, = struct.unpack_from("<I", raw, 20 + (section - 1) * 40 + 20)
    changed[pointer + offset] = value
    return bytes(changed)


def test_exact_parent_over_combined_extent_makes_every_child_exact(tmp_path):
    results, compiles = measured(tmp_path, [0, 1, 2])
    assert compiles == 1
    owner = results[PARENT]
    assert owner.verdict is Tier.BYTE_EXACT
    # 22 primary bytes plus the complete 32-byte combined cold region.
    assert owner.compared_bytes + owner.masked_bytes == 22 + 32
    assert "0x00401100" in owner.diagnostic
    for low, high in ACTIONS:
        assert results[low].verdict is Tier.BYTE_EXACT
        assert results[low].compared_bytes == high - low
        assert "EH cleanup fragment" in results[low].diagnostic


def test_selected_child_alone_compiles_its_parent_as_support(tmp_path):
    results, compiles = measured(tmp_path, [1])
    assert list(results) == [ACTIONS[1][0]] and compiles == 1
    assert results[ACTIONS[1][0]].verdict is Tier.BYTE_EXACT


def test_parent_difference_is_a_child_mismatch_never_a_standalone_match(tmp_path):
    # Damage a byte inside the FIRST action; the second action's own slice
    # is still identical, yet it must not be reported as matching.
    damaged = change_code(_coff(cold_size=32), 2, 2, 0xE0)
    results, _ = measured(tmp_path, [0, 1, 2], compiled=damaged)
    assert results[PARENT].verdict not in (Tier.BYTE_EXACT, Tier.REFUSED, Tier.NO_COMPILE)
    for low, _high in ACTIONS:
        assert results[low].verdict is Tier.MISMATCH
        assert "parent is" in results[low].diagnostic


def test_unresolved_or_forged_support_never_measures_a_child(tmp_path):
    results, compiles = measured(tmp_path, [0, 2], groups=())
    assert isinstance(results[ACTIONS[0][0]], str) and compiles <= 1
    # Without the group the parent's declared cold span is only its stub.
    assert results[PARENT].verdict is Tier.REFUSED
    results, compiles = measured(tmp_path, [0], groups="forged")
    assert "not resolved by the package" in results[ACTIONS[0][0]] and compiles == 0


def test_recording_refuses_children_and_their_parent(tmp_path):
    path = tmp_path / "owner.cpp"
    path.write_text(source())
    rows = read_text(path.read_text(), path)

    def unexpected():
        raise AssertionError("recording must refuse before measuring")

    for row, match in ((rows[0], "non-callable function fragment"), (rows[2], "attached function fragment")):
        with pytest.raises(ValueError, match=match):
            record_claims([row], measure=unexpected, validate=lambda: None,
                          staging_directory=tmp_path / "stage")


def test_match_check_derives_children_from_the_measured_parent(match_base_project):
    root = match_base_project
    (root / "build").mkdir(exist_ok=True)
    configure_database(root, ["engine/family.cc"])
    configuration(root, lambda value: value.update(mappings={
        "compile_commands": "build/commands.json", "forwarder_macros": []}))
    (root / "engine/family.cc").write_text(source())
    image = root / "original/input.exe"
    image.write_bytes(_pe(cold_size=32))
    (root / "original/PIN").write_text(
        "sha256 " + hashlib.sha256(image.read_bytes()).hexdigest() + "\n")
    configure_compiler(root, raw_object=_coff(cold_size=32))
    report = check_matches(root, Path("original/input.exe"))
    rows = {row["address"]: row for row in report["measurements"]}
    assert rows["0x00401000"]["verdict"] == "BYTE_EXACT"
    assert rows["0x00401100"]["verdict"] == rows["0x00401108"]["verdict"] == "BYTE_EXACT"
    assert report["compile_attempts"] == 1
    configure_compiler(root, raw_object=change_code(_coff(cold_size=32), 2, 2, 0xE0))
    report = check_matches(root, Path("original/input.exe"))
    rows = {row["address"]: row for row in report["measurements"]}
    assert rows["0x00401100"]["verdict"] == rows["0x00401108"]["verdict"] == "MISMATCH"


@pytest.mark.parametrize("selected_member", ["child", "parent"])
def test_strict_application_route_refuses_eh_cleanup_relations(selected_member):
    origin = {"image_sha256": "a" * 64, "address_space": "pe:va"}
    parent_id = Origin("a" * 64, "pe:va", PARENT).entity_id
    child_ids = [Origin("a" * 64, "pe:va", low).entity_id for low, _high in ACTIONS]
    relation = fragment_fact(read_text(source(), UNIT)[0].function_fragment)
    entities = [{"entity_id": parent_id, "origin": {**origin, "address": PARENT}, "conflicts": [],
                 "candidates": [{"seed": False, "component": None, "path": UNIT.as_posix(),
                                 "symbol": EH_SYMBOL,
                                 "spans": [{"start": PRIMARY[0], "end": PRIMARY[1]},
                                           {"start": STUB[0], "end": STUB[1]}]}]}]
    for child_id, (low, high) in zip(child_ids, ACTIONS):
        entities.append({"entity_id": child_id, "origin": {**origin, "address": low}, "conflicts": [],
                         "candidates": [{"seed": False, "component": None, "path": UNIT.as_posix(),
                                         "symbol": f"sub_{low:x}", "spans": [{"start": low, "end": high}],
                                         "function_fragment": relation}]})
    view = {"groups": [{"entities": entities}]}
    selected = frozenset({parent_id if selected_member == "parent" else child_ids[0]})
    bindings, _refused, _externals = select_bindings(view, selected)
    expanded, fragments, support, refused = select_fragment_bindings(view, selected, bindings)
    assert not fragments and not support
    assert all(binding.origin.entity_id != parent_id for binding in expanded)
    row, = refused
    assert row["entity_id"] in selected and "EH cleanup" in row["reason"]
