"""Source-owned interpretations for code-associated original data spans.

These bindings preserve where a mapped code owner references companion data.
They do not establish physical section layout, object ownership, relocation
targets, linked selection or runtime behavior.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from match_recovery.components import relative_path
from match_recovery.model import Origin, Span, digest, require_digest, require_text
from match_recovery.source.reader import _role_lexical_state_after, read_text
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_ASSOCIATED:"
PATTERN = re.compile(
    r"// RECOVERY_ASSOCIATED: v1 (switch_targets|switch_selector) "
    r"(0x[0-9A-Fa-f]{1,8}) (0x[0-9A-Fa-f]{1,8}) "
    r"(0x[0-9A-Fa-f]{1,8}) (0x[0-9A-Fa-f]{1,8})"
)
KINDS = frozenset(("switch_targets", "switch_selector"))
_MAX_ADDRESS = 0xFFFFFFFF
_MAX_END = _MAX_ADDRESS
_LINE_SPLICE = re.compile(r"(?:\\|\?\?/)[ \t]*$")


@dataclass(frozen=True)
class AssociatedDataBinding:
    """One explicit owner-to-data relation in the original address space."""

    owner: Origin
    span: Span
    kind: str
    evidence: Origin
    unit: str
    line: int

    def __post_init__(self) -> None:
        if type(self.owner) is not Origin or type(self.evidence) is not Origin:
            raise ValueError("associated binding requires typed owner and evidence origins")
        if type(self.span) is not Span:
            raise ValueError("associated binding requires a typed data span")
        # Frozen records are the normal API, but a caller can still mutate a
        # dataclass through low-level reflection. Reconstruct nested values so
        # none of their constructor invariants are trusted at this boundary.
        Origin(self.owner.image_sha256, self.owner.address_space,
               self.owner.address)
        Origin(self.evidence.image_sha256, self.evidence.address_space,
               self.evidence.address)
        Span(self.span.start, self.span.end)
        if type(self.kind) is not str or self.kind not in KINDS:
            raise ValueError("unsupported associated data kind")
        relative_path(self.unit)
        if type(self.line) is not int or self.line < 1:
            raise ValueError("associated annotation line must be a positive one-based line")
        if ((self.owner.image_sha256, self.owner.address_space)
                != (self.evidence.image_sha256, self.evidence.address_space)):
            raise ValueError("associated owner and evidence belong to different originals")
        if (not 0 <= self.owner.address <= _MAX_ADDRESS
                or not 0 <= self.evidence.address <= _MAX_ADDRESS):
            raise ValueError("associated owner and evidence must be bounded 32-bit addresses")
        if not (0 <= self.span.start <= _MAX_ADDRESS
                and self.span.start < self.span.end <= _MAX_END):
            raise ValueError("associated data span must be a bounded 32-bit half-open range")

    @property
    def interpretation(self) -> dict:
        return {
            "kind": self.kind,
            "owner": asdict(self.owner),
            "span": asdict(self.span),
            "original_evidence": asdict(self.evidence),
            "unit": self.unit,
        }

    @property
    def interpretation_id(self) -> str:
        return digest({"entity_id": self.evidence.entity_id,
                       "value": self.interpretation})


def _source_comment_lines(text: str):
    """Yield line context, including physical-line splice continuation."""
    in_block = False
    raw_delimiter = None
    continued = False
    for number, line in enumerate(text.splitlines(), 1):
        yield number, line, in_block, raw_delimiter is not None, continued
        in_block, raw_delimiter, _ = _role_lexical_state_after(
            line, in_block, raw_delimiter)
        continued = _LINE_SPLICE.search(line) is not None


def parse_associated(text: str, path: str, pin: str, *,
                     address_space: str) -> tuple[AssociatedDataBinding, ...]:
    """Parse source annotations without consulting binary or compiler data."""
    relative_path(path)
    require_digest(pin)
    require_text(address_space, "address space")
    if PREFIX not in text:
        return ()
    bindings = []
    # This parser intentionally does not implement C++ translation-phase
    # splicing. A physical annotation-looking line can be part of a continued
    # // comment or ordinary string, or splicing can alter its lexical context.
    # Later splicing cannot affect the earlier unspliced lexical prefix. Once
    # a splice is encountered, however, this walker cannot establish context:
    # refuse even apparent records it would otherwise consider block/raw text.
    # Files without association records retain the ordinary empty behavior.
    first_splice = next((number for number, line in enumerate(text.splitlines(), 1)
                         if _LINE_SPLICE.search(line)), None)
    for line, source_line, in_block, in_raw, continued in _source_comment_lines(text):
        stripped = source_line.strip()
        if (first_splice is not None and line >= first_splice
                and stripped.startswith(PREFIX)):
            raise ValueError(
                f"associated-data annotation in line-spliced source at {path}:{line}")
        if in_block or in_raw or not stripped.startswith(PREFIX):
            continue
        if continued:
            continue
        match = PATTERN.fullmatch(stripped)
        if match is None:
            raise ValueError(f"invalid associated-data annotation at {path}:{line}")
        kind, owner, start, end, evidence = match.groups()
        try:
            binding = AssociatedDataBinding(
                Origin(pin, address_space, int(owner, 16)),
                Span(int(start, 16), int(end, 16)), kind,
                Origin(pin, address_space, int(evidence, 16)), path, line,
            )
        except ValueError as error:
            raise ValueError(f"invalid associated-data annotation at {path}:{line}: {error}") from error
        bindings.append(binding)
    return validate_associated(bindings)


def validate_associated(bindings) -> tuple[AssociatedDataBinding, ...]:
    """Require one unambiguous population of explicit associated data spans."""
    rows = tuple(bindings)
    if any(type(row) is not AssociatedDataBinding for row in rows):
        raise ValueError("associated data requires typed source interpretations")
    # Reconstruct each row to rerun validation even if a caller bypassed the
    # frozen dataclass contract after construction.
    rows = tuple(AssociatedDataBinding(row.owner, row.span, row.kind,
                                       row.evidence, row.unit, row.line)
                 for row in rows)
    if len({(row.owner.image_sha256, row.owner.address_space) for row in rows}) > 1:
        raise ValueError("associated bindings belong to different originals")
    identities = [(row.owner.entity_id, row.span, row.kind, row.evidence.entity_id,
                   row.unit) for row in rows]
    if len(set(identities)) != len(identities):
        raise ValueError("duplicate associated-data binding")
    owner_units = {}
    for row in rows:
        previous = owner_units.setdefault(row.owner.entity_id, row.unit)
        if previous != row.unit:
            raise ValueError("one associated code owner is declared in multiple source units")
    spans = sorted(row.span for row in rows)
    if any(left.end > right.start for left, right in zip(spans, spans[1:])):
        raise ValueError("associated data spans overlap")
    return rows


def _overlaps(left: Span, right: Span) -> bool:
    return left.start < right.end and right.start < left.end


def read_associated(root: Path, pin: str, *, inputs=None) -> tuple[AssociatedDataBinding, ...]:
    """Read product annotations and prove their source-level owner closure.

    This validates mappings and overlap declarations only. Instruction-boundary
    evidence and physical code/data layout belong to binary verification.
    """
    root = root.resolve()
    from .source_inventory import source_file_texts
    configuration, source_rows = source_file_texts(root, inputs)
    source_rows = tuple(source_rows)
    bindings = []
    for relative, text in source_rows:
        if configuration.is_seed(relative):
            continue
        bindings.extend(parse_associated(text, relative, pin,
                                         address_space=configuration.address_space))
    result = validate_associated(bindings)
    if not result:
        return result

    functions = []
    objects = []
    generated = []
    literals = []
    from .generated_source import parse_generated
    from .literal_source import parse_literals
    from .object_source import parse_objects

    for relative, text in source_rows:
        # Retained seed mappings do not supply live owner annotations, but
        # remain original identities whose bytes cannot be relabelled.
        objects.extend(parse_objects(text, relative, pin,
                                     address_space=configuration.address_space))
        generated.extend(parse_generated(text, relative, pin,
                                         address_space=configuration.address_space))
        literals.extend(parse_literals(text, relative, pin,
                                       address_space=configuration.address_space))
        if inputs is None:
            functions.extend((relative, item) for item in read_text(text, relative))
    if inputs is not None:
        from types import SimpleNamespace
        functions.extend((row["path"], SimpleNamespace(
            address=row["origin"]["address"],
            image_spans=tuple(tuple(span) for span in row["spans"]),
            function_fragment=row.get("function_fragment"),
            extract_error=row.get("extract_error")))
            for row in inputs.records)

    functions_by_unit: dict[str, list] = {}
    for unit, record in functions:
        functions_by_unit.setdefault(unit, []).append(record)
    for binding in result:
        candidates = [record for record in functions_by_unit.get(binding.unit, ())
                      if record.address == binding.owner.address]
        if len(candidates) != 1:
            raise ValueError("associated owner must match one source-mapped function in its unit")
        parent = candidates[0]
        if (parent.function_fragment is not None or not parent.image_spans
                or parent.image_spans[0][0] != parent.address
                or parent.image_spans[0][1] <= parent.image_spans[0][0]
                or parent.extract_error):
            raise ValueError("associated owner requires one independent non-fragment primary function span")
        if any(low < 0 or high <= low for low, high in parent.image_spans):
            raise ValueError("associated owner spans must be positive nonnegative ranges")
        owner_spans = tuple(Span(low, high) for low, high in parent.image_spans)
        if any(_overlaps(left, right)
               for index, left in enumerate(owner_spans)
               for right in owner_spans[index + 1:]):
            raise ValueError("associated owner spans must be pairwise disjoint")
        parent_span = owner_spans[0]
        if _overlaps(binding.span, parent_span):
            raise ValueError("associated data must be outside its owner function span")
        if not parent_span.start <= binding.evidence.address < parent_span.end:
            raise ValueError("associated evidence must lie inside its mapped owner function")

    # No associated byte may also be claimed as any mapped function code.
    for unit, record in functions:
        for low, high in record.image_spans:
            if high <= low:
                continue
            function_span = Span(low, high)
            if any(_overlaps(binding.span, function_span) for binding in result):
                raise ValueError("associated data overlaps a source-mapped function span")

    # Existing object/table/literal annotations are separate source ownership
    # domains; do not silently relabel their bytes as companion switch data.
    for binding in result:
        for other in objects:
            conflict = (_overlaps(binding.span, other.span) if other.span is not None
                        else binding.span.start <= other.origin.address < binding.span.end)
            if conflict:
                raise ValueError("associated data overlaps a source-owned object")
        for other in generated:
            conflict = (_overlaps(binding.span, other.span) if other.span is not None
                        else binding.span.start <= other.origin.address < binding.span.end)
            if conflict:
                raise ValueError("associated data overlaps a compiler-generated table")
        for other in literals:
            if _overlaps(binding.span, other.span):
                raise ValueError("associated data overlaps a source-owned literal")
    return result
