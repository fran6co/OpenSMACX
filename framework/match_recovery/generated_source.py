"""Source-owned identities for compiler-generated tables, never copied tables."""

from dataclasses import dataclass
from pathlib import Path
import re

from .components import relative_path
from .model import Origin, Span, require_text


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

    def __post_init__(self):
        require_text(self.symbol, "generated symbol")
        relative_path(self.path)
        if self.span is not None and self.span.start != self.origin.address:
            raise ValueError("table extent must start at its original identity")
        if (self.evidence.image_sha256, self.evidence.address_space) != (self.origin.image_sha256, self.origin.address_space):
            raise ValueError("table evidence belongs to another original")


def parse_generated(text: str, path: str, pin: str) -> tuple[GeneratedBinding, ...]:
    """Parse current or historical source; seed annotations remain hints."""
    relative_path(path)
    parts = Path(path).parts
    if Path(path).suffix not in (".cpp", ".h") or parts[:2] in (("src", "recovered"), ("src", "unrecovered")):
        return ()
    bindings = []
    for line, source_line in enumerate(text.splitlines(), 1):
        if not source_line.lstrip().startswith(PREFIX):
            continue
        match = PATTERN.fullmatch(source_line.strip())
        if not match:
            raise ValueError(f"invalid generated binding at {path}:{line}")
        start, evidence = int(match[1], 16), int(match[4], 16)
        span = None if match[2] == "?" else Span(start, int(match[2], 16))
        if max(start, evidence, span.end - 1 if span else 0) > 0xFFFFFFFF:
            raise ValueError("generated PE binding exceeds 32 bits")
        bindings.append(GeneratedBinding(Origin(pin, "pe:va", start), span, match[3],
                                          Origin(pin, "pe:va", evidence), path, line))
    return tuple(bindings)


def read_generated(root: Path, pin: str) -> tuple[GeneratedBinding, ...]:
    root = root.resolve()
    bindings = []
    for path in sorted((root / "src").rglob("*")):
        if path.suffix not in (".cpp", ".h") or not path.is_file():
            continue
        if path.relative_to(root / "src").parts[0] in ("recovered", "unrecovered"):
            continue
        if not path.resolve().is_relative_to(root):
            raise ValueError("generated binding resolves outside source root")
        bindings.extend(parse_generated(path.read_text(encoding="utf-8"), path.relative_to(root).as_posix(), pin))
    if (len({row.symbol for row in bindings}) != len(bindings)
            or len({row.origin.entity_id for row in bindings}) != len(bindings)):
        raise ValueError("duplicate generated symbol or original table start")
    return tuple(bindings)
