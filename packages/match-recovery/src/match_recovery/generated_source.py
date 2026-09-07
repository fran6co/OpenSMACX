"""Source-owned identities for compiler-generated tables, never copied tables."""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from match_recovery.components import relative_path
from match_recovery.model import Origin, Span, digest, require_text
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_GENERATED:"
PATTERN = re.compile(r"// RECOVERY_GENERATED: v1 vtable (0x[0-9a-fA-F]+) (\?|0x[0-9a-fA-F]+) (\S+) (0x[0-9a-fA-F]+)")


@dataclass(frozen=True)
class GeneratedBinding:
    origin: Origin
    span: Span | None
    symbol: str
    evidence: Origin
    path: str
    line: int

    @property
    def interpretation(self):
        return {"kind": "compiler_generated_vtable", "origin": asdict(self.origin),
                "span": asdict(self.span) if self.span is not None else None,
                "symbol": self.symbol, "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        return digest({"entity_id": self.evidence.entity_id, "value": self.interpretation})

    def __post_init__(self):
        require_text(self.symbol, "generated symbol")
        relative_path(self.path)
        if self.span is not None and self.span.start != self.origin.address:
            raise ValueError("table extent must start at its original identity")
        if (self.evidence.image_sha256, self.evidence.address_space) != (self.origin.image_sha256, self.origin.address_space):
            raise ValueError("table evidence belongs to another original")


def parse_generated(text: str, path: str, pin: str, *, address_space: str) -> tuple[GeneratedBinding, ...]:
    """Parse supplied source; the configured reader selects product versus seed."""
    relative_path(path)
    bindings = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = PATTERN.fullmatch(source_line.strip())
        if not match:
            raise ValueError(f"invalid generated binding at {path}:{line}")
        start, evidence = int(match[1], 16), int(match[4], 16)
        span = None if match[2] == "?" else Span(start, int(match[2], 16))
        bindings.append(GeneratedBinding(Origin(pin, address_space, start), span, match[3],
                                          Origin(pin, address_space, evidence), path, line))
    return tuple(bindings)


def read_generated(root: Path, pin: str) -> tuple[GeneratedBinding, ...]:
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    bindings = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        if configuration.is_seed(relative):
            continue
        bindings.extend(parse_generated(path.read_text(encoding="utf-8"), relative, pin,
                                        address_space=configuration.address_space))
    if (len({row.symbol for row in bindings}) != len(bindings)
            or len({row.origin.entity_id for row in bindings}) != len(bindings)):
        raise ValueError("duplicate generated symbol or original table start")
    return tuple(bindings)
