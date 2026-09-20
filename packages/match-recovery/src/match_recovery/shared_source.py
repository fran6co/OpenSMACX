"""Explicit additional C++ definitions sharing a mapped original code span.

The original record remains unique. These are evidence-backed interpretations,
not aliases inferred from equal bytes, a new claim ledger, or linker directives.
"""

from dataclasses import asdict, dataclass
from pathlib import Path
import re

from .bound_objects import FunctionBinding
from .model import Origin, Span, digest
from .object_source import referenced_object_units
from .source_config import SourceConfiguration


PREFIX = "// RECOVERY_SHARED:"
PATTERN = re.compile(r"// RECOVERY_SHARED: v1 (0x[0-9a-fA-F]+) (0x[0-9a-fA-F]+) (\S+) (0x[0-9a-fA-F]+)")


@dataclass(frozen=True)
class SharedFunctionBinding:
    origin: Origin
    span: Span
    symbol: str
    evidence: Origin
    unit: str
    line: int
    body_error: str | None = None

    def __post_init__(self):
        FunctionBinding(self.origin, (self.span,), self.symbol, self.unit)
        if (self.evidence.image_sha256, self.evidence.address_space) != (self.origin.image_sha256, self.origin.address_space):
            raise ValueError("shared body evidence belongs to another original")
        if type(self.line) is not int or self.line < 1:
            raise ValueError("invalid shared body source location")

    @property
    def interpretation(self):
        return {"kind": "shared_function_body", "origin": asdict(self.origin),
                "span": asdict(self.span), "symbol": self.symbol, "unit": self.unit,
                "original_evidence": asdict(self.evidence)}

    @property
    def interpretation_id(self):
        return digest({"entity_id": self.evidence.entity_id, "value": self.interpretation})


def parse_shared(text, path, pin, *, address_space):
    from .source.reader import _block_state_after, _extract_forward_text, _is_placeholder_region

    lines = text.splitlines()
    locations, in_block = [], False
    for i, line in enumerate(lines):
        if line.lstrip().startswith(PREFIX):
            locations.append((i, in_block))
        in_block = _block_state_after(line, in_block)
    bindings = []
    for index, in_block in locations:
        match = PATTERN.fullmatch(lines[index].strip())
        if match is None:
            raise ValueError(f"invalid shared body binding at {path}:{index + 1}")
        start, end, symbol, evidence = match.groups()
        # A shared annotation sits beside its definition. Never borrow the
        # following annotated body's braces or unfinished-state sentinel.
        stop = next((i for i in range(index + 1, len(lines))
                     if lines[i].lstrip().startswith((PREFIX, "// ORIGINAL:"))), len(lines))
        error = None
        try:
            region = _extract_forward_text(lines[index:stop], 1, symbol, in_block=in_block)
            if _is_placeholder_region(region):
                error = "unfinished shared source placeholder"
        except ValueError:
            error = "shared source binding has no body region"
        origin = Origin(pin, address_space, int(start, 16))
        bindings.append(SharedFunctionBinding(origin, Span(origin.address, int(end, 16)), symbol,
                        Origin(pin, address_space, int(evidence, 16)), path, index + 1, error))
    return tuple(bindings)


def read_shared(root: Path, pin: str, *, contents=None):
    """Read product definitions, including private candidate-overlay body state."""
    root = root.resolve()
    configuration = SourceConfiguration.read(root)
    bindings = []
    for path in configuration.files(root):
        relative = path.relative_to(root).as_posix()
        if configuration.is_seed(relative):
            continue
        raw = (contents or {}).get(relative)
        text = path.read_text(encoding="utf-8") if raw is None else raw.decode("utf-8")
        bindings.extend(parse_shared(text, relative, pin, address_space=configuration.address_space))
    if len({item.symbol for item in bindings}) != len(bindings):
        raise ValueError("duplicate shared function symbol binding")
    return tuple(bindings)


def validate_shared(bindings, view, *, reserved=()):
    """Sharing cannot resolve conflicting originals or invent an original extent."""
    entities = {item["entity_id"]: item for group in view["groups"] for item in group["entities"]}
    symbols = {item["symbol"] for entity in entities.values() for item in entity["candidates"] if not item["seed"]}
    if len({item.symbol for item in bindings}) != len(bindings) or (symbols | set(reserved)) & {item.symbol for item in bindings}:
        raise ValueError("shared function symbol duplicates an existing source binding")
    for binding in bindings:
        entity = entities.get(binding.origin.entity_id)
        if entity is None or entity["conflicts"]:
            raise ValueError("shared body requires an unambiguous mapped original identity")
        spans = {tuple((s["start"], s["end"]) for s in candidate["spans"]) for candidate in entity["candidates"]}
        if spans != {((binding.span.start, binding.span.end),)}:
            raise ValueError("shared body extent disagrees with the original mapping")
        if any(item["component"] is not None for item in entity["candidates"]):
            raise ValueError("shared dependency body requires component verification")
        if binding.evidence.entity_id not in entities:
            raise ValueError("shared body evidence anchor has no original mapping")


def referenced_shared_units(bindings, objects):
    # Owner scheduling uses the same external-relocation rules as real globals.
    # It supplies no addresses and does not authorize editing a dependency.
    return referenced_object_units(bindings, objects)
