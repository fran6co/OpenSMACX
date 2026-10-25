"""Source-owned identities for compiler-generated tables, never copied tables."""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from match_recovery.components import relative_path
from match_recovery.model import Origin, Span, digest, require_text
from .source.reader import _role_lexical_state_after
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_GENERATED:"
PATTERN = re.compile(r"// RECOVERY_GENERATED: v1 (vtable|vbtable) (0x[0-9a-fA-F]+) (\?|0x[0-9a-fA-F]+) (\S+) (0x[0-9a-fA-F]+)")
_TRIGRAPH = re.compile(r"\?\?[=/'()!<>-]")
_MARKER_WORDS = ("RECOVERY_", "ORIGINAL:", "LEVER:", "TRIED:", "DEFERRED:", "UNRECOVERABLE:")


def _validate_generated_symbol(symbol: str, source_kind: str) -> None:
    prefix = {"vtable": "??_7", "vbtable": "??_8"}.get(source_kind)
    if prefix is None:
        raise ValueError("unsupported generated source kind")
    if not isinstance(symbol, str) or not symbol.startswith(prefix):
        raise ValueError(f"{source_kind} interpretation requires an exact {prefix} symbol")
    if (not symbol or any(char.isspace() or ord(char) < 0x20 or 0x7F <= ord(char) <= 0x9F
                          for char in symbol)
            or "/" in symbol or "\\" in symbol or _TRIGRAPH.search(symbol)
            or any(marker in symbol for marker in _MARKER_WORDS)):
        raise ValueError("generated symbol is not one annotation-safe token")


def _require_safe_generated_append(text: str) -> None:
    """Refuse insertion when appending a marker could continue prior syntax."""
    in_block, raw_delimiter = False, None
    lines = text.splitlines(keepends=True)
    for raw_line in lines:
        content = raw_line.rstrip("\r\n")
        in_block, raw_delimiter, _ = _role_lexical_state_after(content, in_block, raw_delimiter)
    if in_block or raw_delimiter is not None:
        raise ValueError("cannot add a generated marker after an unterminated block comment or raw string")
    if lines:
        last_line = lines[-1].rstrip("\r\n")
        if re.search(r"(?:\\|\?\?/)[ \t]*$", last_line):
            raise ValueError("cannot add a generated marker after a continued source line or macro")


@dataclass(frozen=True)
class GeneratedBinding:
    origin: Origin
    span: Span | None
    symbol: str
    evidence: Origin
    path: str
    line: int
    source_kind: str = "vtable"

    @property
    def interpretation(self):
        kind = {"vtable": "compiler_generated_vtable", "vbtable": "compiler_generated_vbtable"}[self.source_kind]
        return {"kind": kind, "origin": asdict(self.origin),
                "span": asdict(self.span) if self.span is not None else None,
                "symbol": self.symbol, "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        return digest({"entity_id": self.evidence.entity_id, "value": self.interpretation})

    def __post_init__(self):
        require_text(self.symbol, "generated symbol")
        relative_path(self.path)
        if self.source_kind not in ("vtable", "vbtable"):
            raise ValueError("unsupported generated source kind")
        _validate_generated_symbol(self.symbol, self.source_kind)
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
        source_kind, start, end, symbol, evidence = match.groups()
        start, evidence = int(start, 16), int(evidence, 16)
        span = None if end == "?" else Span(start, int(end, 16))
        bindings.append(GeneratedBinding(Origin(pin, address_space, start), span, symbol,
                                          Origin(pin, address_space, evidence), path, line, source_kind))
    return tuple(bindings)


def format_generated(binding: GeneratedBinding) -> str:
    """Format one generated binding without changing its source or publishing it."""
    if not isinstance(binding, GeneratedBinding):
        raise TypeError("generated formatter requires a GeneratedBinding")
    _validate_generated_symbol(binding.symbol, binding.source_kind)
    end = "?" if binding.span is None else f"0x{binding.span.end:08X}"
    return (f"// RECOVERY_GENERATED: v1 {binding.source_kind} "
            f"0x{binding.origin.address:08X} {end} {binding.symbol} "
            f"0x{binding.evidence.address:08X}")


def add_generated(text: str, binding: GeneratedBinding) -> str:
    """Add one marker in memory, refusing duplicates and requiring exact readback.

    This is a pure source operation. File writes still require the caller's
    guarded publication protocol.
    """
    if not isinstance(text, str) or not isinstance(binding, GeneratedBinding):
        raise TypeError("generated add requires source text and a GeneratedBinding")
    _require_safe_generated_append(text)
    address_space = binding.origin.address_space
    before = parse_generated(text, binding.path, binding.origin.image_sha256,
                             address_space=address_space)
    if any(row.symbol == binding.symbol or row.origin.entity_id == binding.origin.entity_id
           for row in before):
        raise ValueError("generated symbol or original table start already exists")
    suffix = "" if not text or text.endswith(("\n", "\r")) else "\n"
    candidate = text + suffix + format_generated(binding) + "\n"
    after = parse_generated(candidate, binding.path, binding.origin.image_sha256,
                             address_space=address_space)
    if (len(after) != len(before) + 1
            or tuple(row.interpretation_id for row in after[:-1])
            != tuple(row.interpretation_id for row in before)
            or after[-1].interpretation_id != binding.interpretation_id):
        raise ValueError("generated marker failed exact source readback")
    return candidate


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
