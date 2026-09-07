"""Source-owned original object identities, not inferred address aliases.

An annotation sits beside the owning definition. Its address identifies the
object start, not a pointer variable containing that address. Object extent,
layout, initialization and lifetime remain separate verification obligations.
"""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from match_recovery.model import Origin, Span, digest, require_text
from match_recovery.components import relative_path
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_OBJECT:"
PATTERN = re.compile(r"// RECOVERY_OBJECT: v1 (0x[0-9A-Fa-f]+) (\S+)\s*")
BOUNDED = re.compile(r"// RECOVERY_OBJECT: v2 (0x[0-9A-Fa-f]+) (0x[0-9A-Fa-f]+) (\S+) (0x[0-9A-Fa-f]+)\s*")


@dataclass(frozen=True)
class ObjectBinding:
    origin: Origin
    symbol: str
    unit: str
    line: int
    span: Span | None = None
    evidence: Origin | None = None

    def __post_init__(self):
        relative_path(self.unit)
        require_text(self.symbol, "object symbol")
        if (self.span is None) != (self.evidence is None):
            raise ValueError("bounded object requires original evidence")
        if self.span is not None:
            if self.span.start != self.origin.address:
                raise ValueError("object extent must begin at its original identity")
            if (self.evidence.image_sha256, self.evidence.address_space) != (self.origin.image_sha256, self.origin.address_space):
                raise ValueError("object evidence belongs to another original")

    @property
    def interpretation(self):
        if self.span is None:
            raise ValueError("unbounded object has no initializer interpretation")
        return {"kind": "object_initial_data", "origin": asdict(self.origin),
                "span": asdict(self.span), "symbol": self.symbol,
                "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        value = self.interpretation
        return digest({"entity_id": self.evidence.entity_id, "value": value})


def parse_objects(text: str, path: str, pin: str, *, address_space: str) -> tuple[ObjectBinding, ...]:
    """Parse current or historical object identities without reading live files."""
    relative_path(path)
    bindings = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = BOUNDED.fullmatch(source_line.strip())
        if match:
            start, end, symbol, evidence = match.groups()
            bindings.append(ObjectBinding(Origin(pin, address_space, int(start, 16)), symbol, path, line,
                                          Span(int(start, 16), int(end, 16)), Origin(pin, address_space, int(evidence, 16))))
            continue
        match = PATTERN.fullmatch(source_line.strip())
        if match is None:
            raise ValueError(f"invalid object annotation at {path}:{line}")
        bindings.append(ObjectBinding(Origin(pin, address_space, int(match[1], 16)), match[2], path, line))
    return tuple(bindings)


def read_objects(root: Path, pin: str) -> tuple[ObjectBinding, ...]:
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    bindings = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        if configuration.is_seed(relative):
            continue
        bindings.extend(parse_objects(path.read_text(encoding="utf-8"), relative, pin, address_space=configuration.address_space))
    if len({item.symbol for item in bindings}) != len(bindings):
        raise ValueError("duplicate original object symbol binding")
    if len({item.origin.entity_id for item in bindings}) != len(bindings):
        raise ValueError("multiple objects claim the same original start address")
    spans = sorted(item.span for item in bindings if item.span is not None)
    if any(left.end > right.start for left, right in zip(spans, spans[1:])):
        raise ValueError("original object extents overlap")
    return tuple(bindings)


def object_targets(bindings: tuple[ObjectBinding, ...], objects: dict) -> tuple[dict, list]:
    """Require the owner's compiled definition before resolving object references."""
    targets, reports = {}, []
    for binding in bindings:
        obj = objects.get(binding.unit)
        if obj is None:
            continue  # The task does not prove objects owned by uncompiled TUs.
        matches = [symbol for symbol in obj.symbols.values() if symbol.name == binding.symbol
                   and symbol.storage == 2 and symbol.section > 0 and not symbol.type & 0x20]
        valid = len(matches) == 1 and not obj.sections[matches[0].section - 1].flags & 0x20
        reports.append({"symbol": binding.symbol, "unit": binding.unit,
                        "address": f"0x{binding.origin.address:08X}",
                        "definition": "pass" if valid else "unknown"})
        if valid:
            targets[binding.symbol] = binding.origin.address
    return targets, reports


def referenced_object_units(bindings: tuple[ObjectBinding, ...], objects: dict) -> frozenset[str]:
    """Find declared owners of actual external relocation references.

    This schedules read-only compilation, not target resolution or task editing
    authorization. Bare symbol declarations, local symbols and weak-external
    fallbacks cannot select a source owner. Callers retain definition checks and
    compile each dependency under the same source overlay and flag trial.
    """
    owners = {binding.symbol: binding.unit for binding in bindings}
    if len(owners) != len(bindings):
        raise ValueError("duplicate original object symbol binding")
    required = set()
    for obj in objects.values():
        for section in obj.sections:
            for _, index, kind in section.relocations:
                if kind not in (6, 20):
                    continue
                symbol = obj.symbols[index]
                if symbol.storage == 2 and symbol.name in owners:
                    required.add(owners[symbol.name])
    return frozenset(required)
