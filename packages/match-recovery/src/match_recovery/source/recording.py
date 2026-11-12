"""Invocation-local historical claim publication, not recovery acceptance.

The coordinator supplies measurement and complete input validation. No report
file can be imported as evidence. The source writer only changes claim tokens;
historical notes, including failed experiments, are not current verdicts and
are neither deleted on a match nor reclassified on a regression.
"""

from collections import defaultdict
from dataclasses import dataclass, replace
from pathlib import Path

from .model import DecompilationState, Mode, State, Tier
from .reader import read_text
from .writer import write_bytes
from .publication import publish
from .asm import AsmComparison


@dataclass(frozen=True)
class RecordingResult:
    measured: tuple
    unresolved: tuple
    regressed: tuple
    updated: tuple[DecompilationState, ...]
    changed_files: tuple[Path, ...]

    @property
    def exit_code(self) -> int:
        if any(result.verdict is not Tier.BYTE_EXACT for _, result in self.measured):
            return 1
        return 3 if self.unresolved else 0


def record_claims(selected, *, measure, validate, staging_directory: Path) -> RecordingResult:
    """Measure existing mappings and publish only newly proved byte obligations.

    ``measure`` is invoked here, after source capture; it must return exactly
    one (original record, AsmComparison or error string) pair per selection.
    ``validate`` refuses changed complete measurement inputs. Only selected
    source files can be published; no count floor or side ledger is updated.
    No automatic demotion is supported.
    A BYTE_EXACT result is the historical masked comparer, not structural proof.
    """
    selected = tuple(selected)
    keys = [(item.path, item.line) for item in selected]
    if not selected or len(set(keys)) != len(keys):
        raise ValueError("recording requires a nonempty, unique source selection")
    if any(item.mode is Mode.UNBOUND or item.state is State.UNBOUND for item in selected):
        raise ValueError("cannot record an UNBOUND mapping; it has no emitted body to measure")
    expected = {item.path: item.path.read_bytes() for item in selected}
    original, lossless = {}, {}
    for path, raw in expected.items():
        text = raw.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n")
        original.update({(path, item.line): item for item in read_text(text, path)})
        text = raw.decode("utf-8", errors="surrogateescape").replace("\r\n", "\n").replace("\r", "\n")
        lossless.update({(path, item.line): item for item in read_text(text, path)})
    if any(original.get(key) != item for key, item in zip(keys, selected)):
        raise ValueError("source mapping changed before recording")
    if any(item.state is State.PLACEHOLDER for item in selected):
        raise ValueError("cannot record an unfinished placeholder; implement its body and remove the body-slot sentinel first")
    groups = defaultdict(list)
    attached = attached_fragments(lossless.values())
    selected_keys = set(keys)
    # parent key -> its complete attached fragment keys (address order)
    combined = {}
    for key, item in zip(keys, selected):
        native = lossless[key]
        if native.name != item.name or native.symbol != item.symbol:
            raise ValueError("source symbol cannot be measured losslessly")
        if native.function_fragment is not None:
            relation = native.function_fragment
            parents = [row for row in lossless.values()
                       if row.path == native.path and row.function_fragment is None
                       and (row.path, row.address, row.symbol or row.name)
                       == (native.path, relation.parent_address, relation.parent_symbol)]
            if len(parents) != 1 or (parents[0].path, parents[0].line) not in selected_keys:
                raise ValueError(
                    "cannot independently record a non-callable function fragment; record its parent "
                    f"0x{relation.parent_address:08X} {relation.parent_symbol}, which publishes it "
                    "with every attached fragment from one combined measurement")
        else:
            children = attached.get((item.path, native.address, native.symbol or native.name), ())
            if children:
                missing = [row for row in children if (row.path, row.line) not in selected_keys]
                if missing:
                    raise ValueError(
                        "combined-parent recording requires every attached function fragment in the "
                        "selection; missing " + ", ".join(f"0x{row.address:08X}" for row in missing))
                combined[key] = tuple((row.path, row.line) for row in children)
        groups[item.path].append(native)
    fragment_keys = {child for children in combined.values() for child in children}
    if len(fragment_keys) != sum(len(children) for children in combined.values()) or any(
            lossless[key].function_fragment is not None and key not in fragment_keys for key in keys):
        raise ValueError("function fragment ownership is ambiguous; no claims written")
    if any(write_bytes(expected[path], group) != expected[path] for path, group in groups.items()):
        raise ValueError("source annotations require a lossless no-op rewrite before recording")
    validate()
    rows = tuple(measure())
    if len(rows) != len(selected):
        raise ValueError("measurement did not cover the complete recording selection")
    by_key = {}
    for item, result in rows:
        key = (item.path, item.line)
        if (key not in keys or key in by_key or item != original[key]
                or not isinstance(result, (AsmComparison, str))
                or isinstance(result, AsmComparison) and not isinstance(result.verdict, Tier)):
            raise ValueError("measurement returned an unexpected or duplicate source record/verdict")
        by_key[key] = (item, result)
    # Combined parents: one atomic decision over the parent and every
    # attached fragment. Fragment verdicts are only ever derived from the
    # parent's single measurement over the complete combined extent.
    owner_of = {child: parent for parent, children in combined.items() for child in children}
    group_exact, group_refusal = set(), {}
    for parent_key, children in combined.items():
        reason = _combined_refusal(by_key[parent_key], [by_key[child] for child in children])
        if reason is None:
            group_exact.add(parent_key)
        elif reason:
            group_refusal[parent_key] = reason
    measured, unresolved, regressed, updated = [], [], [], []
    for key in keys:
        item, result = by_key[key]
        group = owner_of.get(key, key if key in combined else None)
        if group is not None and group in group_refusal:
            unresolved.append((item, group_refusal[group]))
            continue
        if isinstance(result, str):
            unresolved.append((item, result))
            continue
        if not result.verdict.scoreable or result.verdict is Tier.NO_COMPILE:
            unresolved.append((item, f"{result.verdict}: {result.diagnostic}"))
            continue
        measured.append((item, result))
        if result.verdict is Tier.BYTE_EXACT:
            if group is not None and group not in group_exact:
                raise AssertionError("combined fragment group reached publication without proof")
            after = replace(item, byte_exact=True, semantic=False)
            if after != item:
                updated.append(after)
        elif item.byte_exact:
            regressed.append((item, result))
    grouped = defaultdict(list)
    for item in updated:
        grouped[item.path].append(replace(lossless[(item.path, item.line)], byte_exact=True, semantic=False))
    replacements = {path: write_bytes(expected[path], group) for path, group in grouped.items()}
    changed = publish(expected, replacements, validate=validate, staging_directory=staging_directory)
    return RecordingResult(tuple(measured), tuple(unresolved), tuple(regressed), tuple(updated), changed)


def attached_fragments(records) -> dict:
    """``(path, parent address, parent symbol)`` -> attached fragment rows.

    Fragments are ordered by address. A parent is identified exactly as a
    fragment relation names it: same marker host, address and emitted symbol.
    """
    attached = defaultdict(list)
    for row in records:
        relation = row.function_fragment
        if relation is not None:
            attached[(row.path, relation.parent_address, relation.parent_symbol)].append(row)
    return {key: tuple(sorted(rows, key=lambda row: (row.address, row.line)))
            for key, rows in attached.items()}


def with_attached_fragments(selected, records) -> list:
    """Expand each selected parent with its complete attached fragment set.

    Selected fragments stay selected; ``record_claims`` refuses a fragment
    whose parent is not also selected.
    """
    attached = attached_fragments(records)
    expanded, seen = [], set()
    for row in selected:
        members = (row,) if row.function_fragment is not None else (
            row, *attached.get((row.path, row.address, row.symbol or row.name), ()))
        for member in members:
            if (member.path, member.line) not in seen:
                seen.add((member.path, member.line))
                expanded.append(member)
    return expanded


def _span_bytes(record) -> int:
    return sum(high - low for low, high in record.image_spans)


def _combined_refusal(parent, children) -> str | None:
    """None when the parent's one measurement proves every member exact.

    ``""`` means an ordinary non-exact parent outcome, handled per row (a
    measured miss, a regression, or an unresolved parent). Any other text is
    an incomplete or inconsistent combined measurement: nothing in the group
    is recorded and every member is reported unresolved.
    """
    parent_row, result = parent
    if not isinstance(result, AsmComparison) or result.verdict is not Tier.BYTE_EXACT:
        if any(isinstance(child, AsmComparison) and child.verdict is Tier.BYTE_EXACT
               for _row, child in children):
            return "function fragment reported exact without an exact combined parent measurement"
        return ""
    extent = _span_bytes(parent_row) + sum(_span_bytes(row) for row, _ in children)
    if result.compared_bytes + result.masked_bytes != extent:
        return (f"combined parent measurement covered {result.compared_bytes + result.masked_bytes} "
                f"bytes, not the complete {extent}-byte parent and fragment extent")
    for row, child in children:
        if (not isinstance(child, AsmComparison) or child.verdict is not Tier.BYTE_EXACT
                or child.flags != result.flags):
            return (f"attached function fragment 0x{row.address:08X} is not covered by the "
                    "parent's exact combined measurement")
    return None
