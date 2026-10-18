"""Explicit mapped-subject selection shared by diagnostics and publication."""

import base64
from dataclasses import dataclass
from pathlib import Path
import re

from .components import relative_path
from .model import require_text
from .source import read_text
from .source.model import Mode
from .source.mangled import identifier, qualified_name


def compilation_context(root, record, settings):
    """Resolve a mapping's configured emitting TU, or reject unsafe context facts."""
    if record.mode is Mode.UNBOUND:
        raise ValueError("UNBOUND mapping has no emitted source or compilation context")
    if not record.context:
        return record.path
    name = record.context
    relative_path(name)
    path = settings.source.path(root, name)
    if (not any(Path(name).is_relative_to(source_root) for source_root in settings.source.roots)
            or Path(name).suffix not in settings.unit_extensions
            or Path(name).suffix in settings.source.interface_extensions
            or settings.source.is_seed(name)):
        raise ValueError(f"source context is not a configured product translation unit: {name}")
    if not path.is_file() or path.is_symlink():
        raise ValueError(f"source context is missing or symlinked: {name}")
    if path.resolve().relative_to(root.resolve()).as_posix() != name:
        raise ValueError(f"source context is not a canonical project-relative path: {name}")
    if not settings.flags_for(name):
        raise ValueError(f"source context has no configured compiler recipe: {name}")
    from .records import read_json
    from .compilation_database import entries
    project = read_json(root / "recovery/project.json")
    mappings = project.get("mappings")
    if not isinstance(mappings, dict) or "compile_commands" not in mappings:
        raise ValueError("source context requires a configured compilation database")
    database = settings.source.path(root, mappings["compile_commands"])
    matches = [entry.file for entry in entries(database.read_bytes(), root)
               if entry.file == path]
    if len(matches) != 1:
        raise ValueError(f"source context does not have one unique compilation-database recipe: {name}")
    return path


@dataclass(frozen=True)
class SnapshotMapping:
    """Selection/edge metadata, deliberately not a parsed source body or verdict."""

    address: int
    path: Path
    line: int
    name: str
    symbol: str
    image_spans: tuple[tuple[int, int], ...]
    context: str = ""
    function_fragment: dict | None = None
    mode: str = "body"
    state: str = "implemented"

    @property
    def address_hex(self):
        return f"0x{self.address:08X}"


def snapshot_mappings(root, snapshot):
    """Project every snapshot mapping without reparsing its source text.

    The source snapshot owns inventory and parse-cache validation. Consumers
    still own their normal current-input checks before returning or compiling.
    """
    records, hosts = [], {}
    for fact in snapshot["records"]:
        name = fact["path"]
        relative_path(name)
        if name not in snapshot["files"]:
            raise ValueError("indexed source mapping has no captured host")
        if name not in hosts:
            # The native reader canonicalizes its record path. Preserve its
            # alias ambiguity semantics without reopening source contents.
            hosts[name] = (root / name).resolve()
        records.append(SnapshotMapping(fact["origin"]["address"], hosts[name], fact["line"],
                                       fact["catalogued_symbol"], fact["emitted_symbol_hint"],
                                       tuple(tuple(pair) for pair in fact["spans"]),
                                       fact.get("context_path", ""), fact.get("function_fragment"),
                                       fact.get("legacy_mode", "body"),
                                       fact.get("legacy_state", "implemented")))
    return records


def selected_source_records(root, snapshot, selected_mappings):
    """Parse selected hosts once from captured bytes; reject index/body drift.

    UTF-8 replacement matches source_inventory and the source reader. A caller
    requiring stricter input decoding must check that separately. No source file
    is reopened and no partially populated reader record is manufactured.
    """
    indexed = {(row.path, row.line): row for row in snapshot_mappings(root, snapshot)}
    parsed, selected = {}, []
    for mapping in selected_mappings:
        key = mapping.path, mapping.line
        if indexed.get(key) != mapping:
            raise ValueError("selected source mapping differs from its snapshot index")
        if mapping.path not in parsed:
            name = mapping.path.relative_to(root).as_posix()
            if name not in snapshot["files"]:
                raise ValueError("selected source mapping has no captured canonical host")
            text = base64.b64decode(snapshot["files"][name]["content_base64"]).decode("utf-8", errors="replace")
            parsed[mapping.path] = {row.line: row for row in read_text(text, mapping.path)}
        record = parsed[mapping.path].get(mapping.line)
        if record is None:
            raise ValueError("selected source mapping differs from freshly parsed captured source")
        emitted_hint = (record.symbol if record.mode is Mode.UNBOUND else record.symbol or record.name)
        if SnapshotMapping(record.address, record.path, record.line, record.name,
                                             emitted_hint, record.image_spans,
                                             record.context,
                                             ({"parent_address": record.function_fragment.parent_address,
                                               "parent_symbol": record.function_fragment.parent_symbol,
                                               "reason": record.function_fragment.reason,
                                               "evidence": list(record.function_fragment.evidence)}
                                              if record.function_fragment is not None else None),
                                             record.mode.value, record.state.value) != mapping:
            raise ValueError("selected source mapping differs from freshly parsed captured source")
        selected.append(record)
    return selected


def select_records(records, targets, host, symbol_format):
    if not isinstance(targets, (list, tuple)) or not targets:
        raise ValueError("source selection requires explicit nonempty targets")
    candidates = [row for row in records if host is None or row.path == host]
    selected = []
    for target in targets:
        require_text(target, "source target")
        matches = [row for row in candidates if target in (row.name, row.symbol)]
        if not matches and symbol_format == "msvc":
            matches = [row for row in candidates if any(
                target in (qualified_name(name), identifier(name)) for name in (row.name, row.symbol) if name)]
        # C++ names can be hex strings too. Explicit 0x means address; otherwise
        # source names take precedence. No implicit product-over-seed preference.
        if (re.fullmatch(r"0[xX][0-9a-fA-F]+", target)
                or not matches and re.fullmatch(r"[0-9a-fA-F]+", target)):
            matches = [row for row in candidates if row.address == int(target, 16)]
        if len(matches) != 1:
            locations = ", ".join(f"{row.path}:{row.line} ({row.address_hex} {row.symbol or row.name})" for row in matches)
            advice = ("select an exact --in path or original address/symbol"
                      if len({row.path for row in matches}) > 1
                      else "select an original address or exact symbol")
            raise ValueError(f"source target {target!r} requires one source mapping; found {len(matches)}" +
                             (f": {locations}; {advice}" if matches else ""))
        selected.append(matches[0])
    if len({(row.path, row.line) for row in selected}) != len(selected):
        raise ValueError("the same source mapping was selected more than once")
    return selected
