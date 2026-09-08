"""Invocation-local historical claim publication, not recovery acceptance.

The coordinator supplies measurement and complete input validation. No report
file can be imported as evidence. The source writer only changes claim tokens;
historical notes, including failed experiments, are not current verdicts and
are neither deleted on a match nor reclassified on a regression.
"""

from collections import defaultdict
from dataclasses import dataclass, replace
from pathlib import Path

from .model import DecompilationState, State, Tier
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
    for key, item in zip(keys, selected):
        native = lossless[key]
        if native.name != item.name or native.symbol != item.symbol:
            raise ValueError("source symbol cannot be measured losslessly")
        groups[item.path].append(native)
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
    measured, unresolved, regressed, updated = [], [], [], []
    for key in keys:
        item, result = by_key[key]
        if isinstance(result, str):
            unresolved.append((item, result))
            continue
        if not result.verdict.scoreable or result.verdict is Tier.NO_COMPILE:
            unresolved.append((item, f"{result.verdict}: {result.diagnostic}"))
            continue
        measured.append((item, result))
        if result.verdict is Tier.BYTE_EXACT:
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
