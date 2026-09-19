"""Source-owned, TU-private literal identities, not inferred pooled aliases.

Bounds and original correspondence are explicit source interpretations. Parsing
does not establish compiler emission, literal contents or relocation agreement.
"""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from .components import relative_path
from .model import Origin, Span, digest, require_text
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_LITERAL:"
PATTERN = re.compile(r"// RECOVERY_LITERAL: v1 (0x[0-9A-Fa-f]+) (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+)")


@dataclass(frozen=True)
class LiteralBinding:
    origin: Origin
    span: Span
    symbol: str
    unit: str
    line: int
    evidence: Origin

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

    @property
    def interpretation(self):
        return {"kind": "tu_private_literal", "origin": asdict(self.origin),
                "span": asdict(self.span), "symbol": self.symbol, "unit": self.unit,
                "original_evidence": asdict(self.evidence)}

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
        if match is None:
            raise ValueError(f"invalid literal annotation at {path}:{line}")
        start, end, symbol, evidence = match.groups()
        bindings.append(LiteralBinding(Origin(pin, address_space, int(start, 16)),
            Span(int(start, 16), int(end, 16)), symbol, path, line,
            Origin(pin, address_space, int(evidence, 16))))
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
    if len({item.origin.entity_id for item in bindings}) != len(bindings):
        raise ValueError("multiple literals claim the same original start address")
    spans = sorted(item.span for item in bindings)
    if any(left.end > right.start for left, right in zip(spans, spans[1:])):
        raise ValueError("original literal extents overlap")
    return bindings


def read_literals(root: Path, pin: str) -> tuple[LiteralBinding, ...]:
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    bindings = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        if configuration.is_seed(relative):
            continue
        bindings.extend(parse_literals(path.read_text(encoding="utf-8"), relative, pin,
                                       address_space=configuration.address_space))
    bindings = validate_literals(bindings)
    if bindings:
        # A private literal is not a second ownership label for a declared
        # object/table. Unbounded starts constrain only that known point.
        from .object_source import read_objects
        from .generated_source import read_generated
        others = (*read_objects(root, pin), *read_generated(root, pin))
        for literal in bindings:
            for other in others:
                overlap = (literal.span.start <= other.origin.address < literal.span.end
                           or other.span is not None and other.span.start < literal.span.end
                           and literal.span.start < other.span.end)
                if overlap:
                    raise ValueError("original literal overlaps another source-owned object/table")
    return bindings
