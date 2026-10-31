"""Explicit additional C++ definitions sharing a mapped original code span.

The original record remains unique. These are evidence-backed interpretations,
not aliases inferred from equal bytes, a new claim ledger, or linker directives.
"""

from dataclasses import asdict, dataclass, field
from pathlib import Path
import re

from .bound_objects import FunctionBinding
from .components import relative_path
from .model import Origin, Span, digest
from .object_source import referenced_symbol_units
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
    # Derived from the mapped evidence function's compiler context. This is
    # intentionally excluded from interpretation identity: source annotations
    # and original evidence remain stable when build-context facts are rebuilt.
    compiler_unit: str | None = field(default=None, compare=False)
    context_error: str | None = field(default=None, compare=False)

    def __post_init__(self):
        FunctionBinding(self.origin, (self.span,), self.symbol, self.unit)
        if self.compiler_unit is not None:
            relative_path(self.compiler_unit)
        if self.context_error is not None and (not isinstance(self.context_error, str) or not self.context_error):
            raise ValueError("invalid shared compiler-context diagnostic")
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

    @property
    def object_unit(self):
        """Resolved compiler carrier, or the legacy annotation unit for raw parsers."""
        if self.context_error:
            return None
        if self.compiler_unit:
            return self.compiler_unit
        # Raw library callers historically supplied a translation-unit path
        # directly. An unqualified header path never proves its own carrier.
        return self.unit if Path(self.unit).suffix.lower() in {".c", ".cc", ".cpp", ".cxx"} else None


def parse_shared(text, path, pin, *, address_space):
    from .source.reader import (_block_state_after, _brace_delta, _extract_forward_text,
                                _is_placeholder_region, function_line)
    from .source.mangled import identifier, qualified_name

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
            else:
                # A shared annotation is attached to a method definition.
                # Merely finding a brace-delimited region allowed a marker at
                # class scope to borrow the containing class braces and then
                # masquerade as an implicit member body.
                qualified = qualified_name(symbol)
                method = qualified.rsplit("::", 1)[-1] if "::" in qualified else identifier(symbol)
                definition_line = None
                if method:
                    try:
                        definition_line = function_line(region, method)
                    except ValueError:
                        pass
                in_region_block = False
                region_lines = region.splitlines()
                limit = definition_line if definition_line is not None else len(region_lines)
                class_before_definition = False
                for source_line in region_lines[:limit]:
                    code = _brace_delta(source_line, in_region_block)[3]
                    in_region_block = _block_state_after(source_line, in_region_block)
                    if re.search(r"\b(?:class|struct)\s+[A-Za-z_]", code):
                        class_before_definition = True
                        break
                if class_before_definition:
                    error = ("shared source annotation is attached at class scope"
                             if definition_line is not None else
                             "shared source binding has no method definition body")
        except ValueError:
            following = "\n".join(lines[index + 1:stop])
            if re.search(r"\b(?:class|struct)\s+[A-Za-z_]", following):
                error = "shared source binding has no method definition body"
            else:
                error = "shared source binding has no body region"
        origin = Origin(pin, address_space, int(start, 16))
        bindings.append(SharedFunctionBinding(origin, Span(origin.address, int(end, 16)), symbol,
                        Origin(pin, address_space, int(evidence, 16)), path, index + 1, error))
    return tuple(bindings)


def read_shared(root: Path, pin: str, *, contents=None):
    """Read shared annotations and body state without deriving compiler contexts."""
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


def read_shared_bindings(root: Path, pin: str, *, contents=None):
    """Read body-backed shared methods and declaration-anchored generated members."""
    from .generated_member_source import read_generated_members

    return (*read_shared(root, pin, contents=contents),
            *read_generated_members(root, pin, contents=contents))


def resolve_shared_contexts(root: Path, bindings, *, settings, contents=None, source_records=None):
    """Derive each shared definition's compiler TU from its mapped evidence.

    `unit` remains the source annotation/body path. In particular, a header
    definition is never relabeled as a translation unit. Evidence must resolve
    through exactly one set of validated mapped compiler contexts; missing or
    conflicting witnesses stay attached as explicit unknowns.
    """
    from dataclasses import replace
    from .source.reader import read_text
    from .source_selection import compilation_context

    root = Path(root).resolve()
    configuration = SourceConfiguration.read(root)
    if not hasattr(settings, "unit_extensions") or not hasattr(settings, "source"):
        raise TypeError("shared compiler-context resolution requires project measurement settings")
    from .generated_member_source import GeneratedMemberBinding
    needs_witnesses = any(Path(binding.unit).suffix in configuration.interface_extensions
                          or isinstance(binding, GeneratedMemberBinding) for binding in bindings)
    if source_records is None and needs_witnesses:
        source_records = []
        for path in configuration.files(root):
            relative = path.relative_to(root).as_posix()
            if configuration.is_seed(relative):
                continue
            raw = (contents or {}).get(relative)
            text = path.read_text(encoding="utf-8") if raw is None else raw.decode("utf-8")
            source_records.extend(read_text(text, path))
    source_records = source_records or ()
    resolved = []
    for binding in bindings:
        if isinstance(binding, GeneratedMemberBinding):
            from .source import State
            from .source.mangled import qualified_name

            witnesses = [record for record in source_records
                         if record.address == binding.evidence.address]
            constructors = [record for record in witnesses
                            if (record.symbol or record.name).startswith("??0")
                            and qualified_name(record.symbol or record.name).rsplit("::", 1)[0] == binding.owner]
            if len(witnesses) != 1 or len(constructors) != 1:
                resolved.append(replace(binding, compiler_unit=None,
                    context_error="generated member evidence is missing or ambiguous for its class constructor"))
                continue
            if constructors[0].state != State.IMPLEMENTED:
                resolved.append(replace(binding, compiler_unit=None,
                    context_error="generated member evidence constructor is not an implemented source body"))
                continue
        if Path(binding.unit).suffix not in configuration.interface_extensions:
            resolved.append(replace(binding, compiler_unit=binding.unit, context_error=None))
            continue
        witnesses = [record for record in source_records
                     if record.address == binding.evidence.address]
        contexts, failures = set(), []
        for record in witnesses:
            try:
                context = compilation_context(root, record, settings)
                if (context.suffix not in settings.unit_extensions
                        or context.suffix in configuration.interface_extensions):
                    raise ValueError(f"shared evidence context is not a configured translation unit: {context}")
                contexts.add(context.relative_to(root).as_posix())
            except (ValueError, OSError) as error:
                failures.append(str(error))
        if len(contexts) == 1 and not failures:
            resolved.append(replace(binding, compiler_unit=next(iter(contexts)), context_error=None))
        elif len(contexts) > 1 or contexts and failures:
            resolved.append(replace(binding, compiler_unit=None,
                context_error="shared evidence has ambiguous compiler context"))
        elif failures:
            resolved.append(replace(binding, compiler_unit=None, context_error=failures[0]))
        else:
            resolved.append(replace(binding, compiler_unit=None,
                context_error="shared evidence has no mapped product compiler context"))
    return tuple(resolved)


def _validate_shared_symbol(symbol: str) -> None:
    if (not isinstance(symbol, str) or not symbol or not symbol.isascii()
            or any(c.isspace() or ord(c) < 32 or ord(c) == 127 for c in symbol)
            or any(token in symbol for token in ("/*", "*/", "//", "\\", "??/"))):
        raise ValueError("shared symbol must be one annotation-safe ASCII token")


def format_shared(binding: SharedFunctionBinding) -> str:
    """Format one shared-body marker without publishing or changing its source."""
    if not isinstance(binding, SharedFunctionBinding):
        raise TypeError("shared formatter requires a SharedFunctionBinding")
    _validate_shared_symbol(binding.symbol)
    return (f"{PREFIX} v1 0x{binding.origin.address:08X} "
            f"0x{binding.span.end:08X} {binding.symbol} "
            f"0x{binding.evidence.address:08X}")


def write_shared_symbol(raw: bytes, *, expected, binding: SharedFunctionBinding,
                        symbol: str) -> bytes:
    """Rename one emitted shared view, preserving original identity/evidence.

    Pure annotation operation, not compiler evidence or editing authorization.
    The caller supplies a complete current parsed snapshot and independently
    verifies the new compiler symbol. Body text and every other byte survive.
    """
    from dataclasses import replace
    from .source.writer import _define_directive_lines

    if not isinstance(raw, bytes):
        raise ValueError("shared symbol rewriting requires source bytes")
    _validate_shared_symbol(symbol)
    text = raw.decode("utf-8")
    if raw.startswith(b"\xef\xbb\xbf") or re.search(r"[\v\f\x1c-\x1e\x85\u2028\u2029]", text):
        raise ValueError("unsupported shared source encoding or separator")
    pin, space = binding.origin.image_sha256, binding.origin.address_space
    current = parse_shared(text, binding.unit, pin, address_space=space)
    if current != tuple(expected) or current.count(binding) != 1:
        raise ValueError("shared symbol update requires a current complete snapshot")
    if any(row != binding and row.symbol == symbol for row in current):
        raise ValueError("duplicate shared symbol")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    if binding.line - 1 in _define_directive_lines(normalized):
        raise ValueError("shared annotation is inside a macro directive")
    # An earlier splice can alter lexical context. A later untouched macro
    # cannot affect this annotation and need not prevent a guarded rename.
    prefix = "\n".join(normalized.splitlines()[:binding.line])
    if re.search(r"(?:\\|\?\?/)[ \t]*\n", prefix):
        raise ValueError("shared annotation follows line-spliced source")
    lines = raw.splitlines(keepends=True)
    old = lines[binding.line - 1]
    leading = len(old) - len(old.lstrip(b" \t"))
    match = PATTERN.fullmatch(old[leading:].decode("utf-8").strip())
    if match is None or not binding.symbol.isascii():
        raise ValueError("shared marker is not safely rewritable")
    start = leading + match.start(3)
    end = start + len(binding.symbol.encode("ascii"))
    if old[start:end] != binding.symbol.encode("ascii"):
        raise ValueError("shared symbol does not match source bytes")
    lines[binding.line - 1] = old[:start] + symbol.encode("ascii") + old[end:]
    result = b"".join(lines)
    wanted = tuple(replace(row, symbol=symbol) if row == binding else row for row in current)
    if parse_shared(result.decode("utf-8"), binding.unit, pin, address_space=space) != wanted:
        raise ValueError("shared symbol rewrite changed other binding facts")
    return result


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
        if any(item.get("unbound", False) for item in entity["candidates"]):
            raise ValueError("shared body cannot borrow an unbound original identity; bind its genuine owner first")
        spans = {tuple((s["start"], s["end"]) for s in candidate["spans"]) for candidate in entity["candidates"]}
        if spans != {((binding.span.start, binding.span.end),)}:
            raise ValueError("shared body extent disagrees with the original mapping")
        if any(item["component"] is not None for item in entity["candidates"]):
            raise ValueError("shared dependency body requires component verification")
        if binding.evidence.entity_id not in entities:
            raise ValueError("shared body evidence anchor has no original mapping")


def validate_shared_bindings(bindings, view, *, reserved=()):
    from .generated_member_source import GeneratedMemberBinding, validate_generated_members

    validate_shared(bindings, view, reserved=reserved)
    validate_generated_members([item for item in bindings if isinstance(item, GeneratedMemberBinding)], view)


def resolve_shared_bindings(root: Path, pin: str, *, settings, view, contents=None,
                            source_records=None, reserved=()):
    bindings = read_shared_bindings(root, pin, contents=contents)
    resolved = resolve_shared_contexts(root, bindings, settings=settings, contents=contents,
                                       source_records=source_records)
    validate_shared_bindings(resolved, view, reserved=reserved)
    return resolved


def referenced_shared_units(bindings, objects):
    # Shared function identities have no object parent/member semantics. Build
    # their owner map explicitly, then reuse only the generic COFF scheduler.
    owners = {}
    for binding in bindings:
        if binding.symbol in owners:
            raise ValueError("duplicate shared function symbol binding")
        # Raw parse_shared() users retain the historical same-file context.
        # Product readers always attach a derived context or explicit refusal.
        if binding.object_unit is not None:
            owners[binding.symbol] = binding.object_unit
    return referenced_symbol_units(owners, objects)
