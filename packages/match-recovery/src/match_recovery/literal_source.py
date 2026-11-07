"""Source-owned, TU-private literal identities, not inferred pooled aliases.

Bounds and original correspondence are explicit source interpretations. Parsing
does not establish compiler emission, literal contents or relocation agreement.

A v1 literal claims its original extent exclusively. A v2 ``merged`` literal
declares one unit's private reference to a constant the linker folded with
other units' copies: several merged literals, each in its own unit, may name
the same exact extent, and one may share it with an object binding another
unit owns at that same start. Partial overlaps are never admitted.
"""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from .components import relative_path
from .model import Origin, Span, digest, require_text
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_LITERAL:"
PATTERN = re.compile(r"// RECOVERY_LITERAL: v1 (0x[0-9A-Fa-f]+) (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+)")
MERGED = re.compile(r"// RECOVERY_LITERAL: v2 merged (0x[0-9A-Fa-f]+) (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+)")


@dataclass(frozen=True)
class LiteralBinding:
    origin: Origin
    span: Span
    symbol: str
    unit: str
    line: int
    evidence: Origin
    merged: bool = False

    def __post_init__(self):
        relative_path(self.unit)
        require_text(self.symbol, "literal symbol")
        if re.fullmatch(r"\S+", self.symbol) is None:
            raise ValueError("literal symbol must be one token")
        if not isinstance(self.origin, Origin) or not isinstance(self.evidence, Origin) or not isinstance(self.span, Span):
            raise ValueError("literal requires original identity, bounds and evidence")
        if self.span.start != self.origin.address:
            raise ValueError("literal extent must begin at its original identity")
        if (self.evidence.image_sha256, self.evidence.address_space) != (self.origin.image_sha256, self.origin.address_space):
            raise ValueError("literal evidence belongs to another original")
        if type(self.line) is not int or self.line < 1:
            raise ValueError("invalid literal source location")
        if type(self.merged) is not bool:
            raise ValueError("literal merge declaration must be a boolean")

    @property
    def interpretation(self):
        value = {"kind": "tu_private_literal", "origin": asdict(self.origin),
                 "span": asdict(self.span), "symbol": self.symbol, "unit": self.unit,
                 "original_evidence": asdict(self.evidence)}
        if self.merged:
            # Only merged literals carry the field, so v1 identities are unchanged.
            value["linker_merged"] = True
        return value

    @property
    def interpretation_id(self):
        return digest({"entity_id": self.evidence.entity_id, "value": self.interpretation})


def parse_literals(text: str, path: str, pin: str, *, address_space: str) -> tuple[LiteralBinding, ...]:
    """Parse current or historical source without consulting compiler names."""
    relative_path(path)
    bindings = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = PATTERN.fullmatch(source_line.strip())
        merged = match is None
        if merged:
            match = MERGED.fullmatch(source_line.strip())
        if match is None:
            raise ValueError(f"invalid literal annotation at {path}:{line}")
        start, end, symbol, evidence = match.groups()
        bindings.append(LiteralBinding(Origin(pin, address_space, int(start, 16)),
            Span(int(start, 16), int(end, 16)), symbol, path, line,
            Origin(pin, address_space, int(evidence, 16)), merged))
    return tuple(bindings)


def validate_literals(bindings) -> tuple[LiteralBinding, ...]:
    """Require an unambiguous source-owned literal population for one original."""
    bindings = tuple(bindings)
    if any(not isinstance(item, LiteralBinding) for item in bindings):
        raise ValueError("literal bindings require typed source interpretations")
    if len({(item.origin.image_sha256, item.origin.address_space) for item in bindings}) > 1:
        raise ValueError("literal bindings belong to different originals")
    if len({(item.unit, item.symbol) for item in bindings}) != len(bindings):
        raise ValueError("duplicate TU-private literal symbol binding")
    by_start = {}
    for item in bindings:
        by_start.setdefault(item.origin.entity_id, []).append(item)
    for group in by_start.values():
        # A shared start is admitted only as one linker-folded constant:
        # every claimant declares the merge, names the same exact extent and
        # lives in its own unit. Anything else remains ambiguous.
        if len(group) > 1 and (not all(item.merged for item in group)
                               or len({item.span for item in group}) != 1
                               or len({item.unit for item in group}) != len(group)):
            raise ValueError("multiple literals claim the same original start address")
    spans = sorted({item.span for item in bindings})
    if any(left.end > right.start for left, right in zip(spans, spans[1:])):
        raise ValueError("original literal extents overlap")
    return bindings


def merged_object_alias(literal: LiteralBinding, other) -> bool:
    """Whether a merged literal names the same folded constant as an object.

    Only another unit's object binding qualifies, and only when it starts at
    the literal and, if bounded, has exactly the literal's extent. Generated
    tables and partial overlaps never qualify.
    """
    from .object_source import ObjectBinding
    return (literal.merged and type(other) is ObjectBinding
            and other.unit != literal.unit
            and (other.origin.image_sha256, other.origin.address_space)
            == (literal.origin.image_sha256, literal.origin.address_space)
            and other.origin.address == literal.span.start
            and (other.span is None or other.span == literal.span))


def object_reserved_literal_symbols(literals, objects) -> frozenset[str]:
    """Literal names that must stay disjoint from object emitted symbols.

    A name is released only when every object binding emitting it is a
    merged alias of every literal declaring it: the same folded constant.
    Other namespaces (functions, generated tables, imports) stay reserved.
    """
    literals, objects = tuple(literals), tuple(objects)
    reserved = set()
    for symbol in {item.symbol for item in literals}:
        claimants = [item for item in literals if item.symbol == symbol]
        owners = [item for item in objects if item.emitted_symbol == symbol]
        if not owners or not all(merged_object_alias(literal, owner)
                                 for literal in claimants for owner in owners):
            reserved.add(symbol)
    return frozenset(reserved)


def read_literals(root: Path, pin: str, *, inputs=None) -> tuple[LiteralBinding, ...]:
    root = root.resolve()
    from .source_inventory import source_file_texts
    configuration, source_rows = source_file_texts(root, inputs)
    bindings = []
    for relative, text in source_rows:
        if configuration.is_seed(relative):
            continue
        bindings.extend(parse_literals(text, relative, pin,
                                       address_space=configuration.address_space))
    bindings = validate_literals(bindings)
    if bindings:
        # A private literal is not a second ownership label for a declared
        # object/table. Unbounded starts constrain only that known point.
        from .object_source import read_objects
        from .generated_source import read_generated
        others = (*read_objects(root, pin, inputs=inputs), *read_generated(root, pin, inputs=inputs))
        for literal in bindings:
            for other in others:
                overlap = (literal.span.start <= other.origin.address < literal.span.end
                           or other.span is not None and other.span.start < literal.span.end
                           and literal.span.start < other.span.end)
                if overlap and not merged_object_alias(literal, other):
                    raise ValueError("original literal overlaps another source-owned object/table")
    return bindings
